#include "core/commit.h"
#include "core/repository.h"
#include "core/file_tracker.h"
#include "core/snapshot.h"
#include "utils/path_utils.h"
#include "utils/time_utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace minigit {

namespace fs = std::filesystem;
using json = nlohmann::json;

// ── next_id ───────────────────────────────────────────────────

std::string CommitNode::next_id(const Repository& repo) {
    fs::path commits_dir = repo.branch_dir(repo.current_branch) / "commits";
    if (!fs::exists(commits_dir)) return "001";

    int max_num = 0;
    for (auto& entry : fs::directory_iterator(commits_dir)) {
        if (entry.is_directory()) {
            try {
                int num = std::stoi(entry.path().filename().string());
                if (num > max_num) max_num = num;
            } catch (...) {}
        }
    }

    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << (max_num + 1);
    return oss.str();
}

// ── head_id ───────────────────────────────────────────────────

std::string CommitNode::head_id(const Repository& repo) {
    fs::path commits_dir = repo.branch_dir(repo.current_branch) / "commits";
    if (!fs::exists(commits_dir)) return "";

    std::string max_id;
    for (auto& entry : fs::directory_iterator(commits_dir)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name > max_id) max_id = name;
        }
    }
    return max_id;
}

// ── create ────────────────────────────────────────────────────

bool CommitNode::create(Repository& repo, const std::string& message) {
    // Get tracked files
    auto tracked = FileTracker::list(repo);
    if (tracked.empty()) {
        std::cerr << "Nothing to commit: no tracked files.\n";
        return false;
    }

    // Determine new commit ID and parent
    std::string new_id = next_id(repo);
    std::string parent = head_id(repo);

    // Create commit directory
    fs::path commit_dir = repo.branch_dir(repo.current_branch)
                          / "commits" / new_id;
    fs::create_directories(commit_dir);

    // Create snapshot (copy all tracked files)
    auto file_map = Snapshot::create(tracked, commit_dir);
    if (file_map.empty()) {
        std::cerr << "Commit failed: no files could be snapshotted.\n";
        fs::remove_all(commit_dir);
        return false;
    }

    // Write meta.json
    json meta;
    meta["id"] = new_id;
    meta["message"] = message;
    meta["timestamp"] = current_timestamp();
    meta["parent_id"] = parent;
    meta["files"] = json::object();
    for (auto& [encoded, original] : file_map) {
        meta["files"][encoded] = original;
    }

    {
        std::ofstream f(commit_dir / "meta.json");
        if (!f) {
            std::cerr << "Failed to write meta.json\n";
            return false;
        }
        f << meta.dump(2);
    }

    std::cout << "Committed " << new_id << ": \"" << message << "\" ("
              << file_map.size() << " files)\n";
    return true;
}

// ── load ──────────────────────────────────────────────────────

bool CommitNode::load(const fs::path& commit_dir, CommitNode& node) {
    fs::path meta_path = commit_dir / "meta.json";
    try {
        std::ifstream f(meta_path);
        if (!f) {
            std::cerr << "Cannot open " << display_path(meta_path) << "\n";
            return false;
        }
        json meta = json::parse(f);
        node.id        = meta.value("id", "");
        node.message   = meta.value("message", "");
        node.timestamp = meta.value("timestamp", "");
        node.parent_id = meta.value("parent_id", "");
        node.files.clear();
        if (meta.contains("files") && meta["files"].is_object()) {
            for (auto& [key, val] : meta["files"].items()) {
                node.files[key] = val.get<std::string>();
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load commit: " << e.what() << "\n";
        return false;
    }
}

// ── checkout ──────────────────────────────────────────────────

bool CommitNode::checkout(const Repository& repo, const std::string& commit_id) {
    fs::path commit_dir = repo.branch_dir(repo.current_branch)
                          / "commits" / commit_id;
    if (!fs::exists(commit_dir)) {
        std::cerr << "Commit " << commit_id << " not found on branch "
                  << repo.current_branch << ".\n";
        return false;
    }

    // Load meta to get file map
    CommitNode node;
    if (!load(commit_dir, node)) return false;

    // Restore files
    if (!Snapshot::restore(commit_dir, node.files)) return false;

    std::cout << "Checked out commit " << commit_id << " on ["
              << repo.current_branch << "]\n";
    return true;
}

} // namespace minigit
