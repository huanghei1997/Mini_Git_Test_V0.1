#include "core/repository.h"
#include "utils/path_utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace minigit {

namespace fs = std::filesystem;
using json = nlohmann::json;

// ── Path helpers ──────────────────────────────────────────────

fs::path Repository::minigit_dir() const {
    return root_path / ".minigit";
}

fs::path Repository::branches_dir() const {
    return minigit_dir() / "branches";
}

fs::path Repository::branch_dir(const std::string& branch_name) const {
    return branches_dir() / branch_name;
}

fs::path Repository::tracked_files_path() const {
    return minigit_dir() / "tracked_files";
}

fs::path Repository::config_path() const {
    return minigit_dir() / "config";
}

// ── init ──────────────────────────────────────────────────────

bool Repository::init(const fs::path& path) {
    fs::path abs_path = fs::absolute(path);
    fs::path mg_dir = abs_path / ".minigit";

    // Idempotent: if .minigit/ already exists, don't overwrite
    if (fs::exists(mg_dir)) {
        std::cout << "Repository already exists at " << display_path(abs_path) << "\n";
        return true;
    }

    try {
        // Create directory structure:
        //   .minigit/
        //   .minigit/branches/main/commits/
        fs::create_directories(mg_dir / "branches" / "main" / "commits");

        // Write initial config
        json config;
        config["current_branch"] = "main";
        {
            std::ofstream f(mg_dir / "config");
            if (!f) return false;
            f << config.dump(2);
        }

        // Write empty tracked_files list
        json tracked = json::array();
        {
            std::ofstream f(mg_dir / "tracked_files");
            if (!f) return false;
            f << tracked.dump(2);
        }

        std::cout << "Initialized Mini-Git repository at "
                  << display_path(abs_path) << "\n";
        return true;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Init failed: " << e.what() << "\n";
        return false;
    }
}

// ── load ──────────────────────────────────────────────────────

bool Repository::load(const fs::path& path, Repository& repo) {
    fs::path root = find_minigit_root(path);
    if (root.empty()) {
        std::cerr << "No .minigit/ found in " << display_path(path)
                  << " or any parent directory.\n";
        return false;
    }

    repo.root_path = root;
    return repo.load_config();
}

// ── save_config ───────────────────────────────────────────────

bool Repository::save_config() const {
    try {
        json config;
        config["current_branch"] = current_branch;

        std::ofstream f(config_path());
        if (!f) return false;
        f << config.dump(2);
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to save config: " << e.what() << "\n";
        return false;
    }
}

// ── load_config ───────────────────────────────────────────────

bool Repository::load_config() {
    try {
        std::ifstream f(config_path());
        if (!f) {
            std::cerr << "Cannot open config at "
                      << display_path(config_path()) << "\n";
            return false;
        }

        json config = json::parse(f);
        current_branch = config.value("current_branch", "main");
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << "\n";
        return false;
    }
}

} // namespace minigit
