#include "core/save.h"
#include "core/repository.h"
#include "core/file_tracker.h"
#include "core/snapshot.h"
#include "core/commit.h"
#include "utils/path_utils.h"
#include "utils/time_utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace minigit {

namespace fs = std::filesystem;
using json = nlohmann::json;

// ── save_dir ──────────────────────────────────────────────────

fs::path SaveManager::save_dir(const Repository& repo) {
    return repo.branch_dir(repo.current_branch) / "save";
}

// ── exists ────────────────────────────────────────────────────

bool SaveManager::exists(const Repository& repo) {
    return fs::exists(save_dir(repo) / "meta.json");
}

// ── create (2.1) ──────────────────────────────────────────────

bool SaveManager::create(Repository& repo) {
    auto tracked = FileTracker::list(repo);
    if (tracked.empty()) {
        std::cerr << "Nothing to save: no tracked files.\n";
        return false;
    }

    fs::path dir = save_dir(repo);

    // Overwrite any existing save
    if (fs::exists(dir)) {
        fs::remove_all(dir);
    }
    fs::create_directories(dir);

    // Snapshot tracked files
    auto file_map = Snapshot::create(tracked, dir);
    if (file_map.empty()) {
        std::cerr << "Save failed: no files could be snapshotted.\n";
        fs::remove_all(dir);
        return false;
    }

    // Write meta.json
    json meta;
    meta["timestamp"] = current_timestamp();
    meta["branch"] = repo.current_branch;
    meta["files"] = json::object();
    for (auto& [encoded, original] : file_map) {
        meta["files"][encoded] = original;
    }

    {
        std::ofstream f(dir / "meta.json");
        if (!f) return false;
        f << meta.dump(2);
    }

    std::cout << "Saved checkpoint on [" << repo.current_branch
              << "] (" << file_map.size() << " files)\n";
    return true;
}

// ── restore (2.2) ─────────────────────────────────────────────

bool SaveManager::restore(const Repository& repo) {
    fs::path dir = save_dir(repo);
    if (!exists(repo)) {
        std::cerr << "No save found on branch [" << repo.current_branch << "].\n";
        return false;
    }

    // Load file map from meta.json
    try {
        std::ifstream f(dir / "meta.json");
        if (!f) return false;
        json meta = json::parse(f);

        std::map<std::string, std::string> file_map;
        for (auto& [key, val] : meta["files"].items()) {
            file_map[key] = val.get<std::string>();
        }

        if (!Snapshot::restore(dir, file_map)) return false;

        std::cout << "Restored save on [" << repo.current_branch << "]\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Restore save failed: " << e.what() << "\n";
        return false;
    }
}

// ── promote (2.3) ─────────────────────────────────────────────

bool SaveManager::promote(Repository& repo, const std::string& message) {
    fs::path dir = save_dir(repo);
    if (!exists(repo)) {
        std::cerr << "No save to promote on branch ["
                  << repo.current_branch << "].\n";
        return false;
    }

    // Restore save first so workspace matches the save state
    if (!restore(repo)) return false;

    // Now create a normal commit (which snapshots the current workspace)
    if (!CommitNode::create(repo, message)) return false;

    // Remove the save after successful promotion
    fs::remove_all(dir);
    std::cout << "Save promoted to commit on [" << repo.current_branch << "]\n";
    return true;
}

} // namespace minigit
