// ============================================================
// Mini-Git -- Complete Source Code Archive
// https://github.com/huanghei1997/Mini_Git_Test_V0.1
// This file is for code reading only, not compilable.
// Total files: 21
// ============================================================

// ============================================================
// FILE 1 : include\core\repository.h
// ============================================================

#ifndef MINIGIT_CORE_REPOSITORY_H
#define MINIGIT_CORE_REPOSITORY_H

#include <filesystem>
#include <string>
#include <vector>

namespace minigit {

struct Repository {
    std::filesystem::path root_path;   // The directory containing .minigit/
    std::string current_branch;        // Currently active branch name

    // Get the .minigit/ directory path
    std::filesystem::path minigit_dir() const;

    // Get the branches/ directory path
    std::filesystem::path branches_dir() const;

    // Get a specific branch directory
    std::filesystem::path branch_dir(const std::string& branch_name) const;

    // Get the tracked_files JSON path
    std::filesystem::path tracked_files_path() const;

    // Get the config JSON path
    std::filesystem::path config_path() const;

    // Initialize a new repository at the given path
    // Creates .minigit/ with config, tracked_files, branches/main/commits/
    static bool init(const std::filesystem::path& path);

    // Load an existing repository from a path (searches for .minigit/)
    static bool load(const std::filesystem::path& path, Repository& repo);

    // Persist current config (current_branch) to .minigit/config
    bool save_config() const;

    // Read config from .minigit/config
    bool load_config();
};

} // namespace minigit

#endif // MINIGIT_CORE_REPOSITORY_H


// ============================================================
// FILE 2 : include\core\file_tracker.h
// ============================================================

#ifndef MINIGIT_CORE_FILE_TRACKER_H
#define MINIGIT_CORE_FILE_TRACKER_H

#include <filesystem>
#include <string>
#include <vector>
#include <map>

namespace minigit {

struct Repository;

enum class FileStatus {
    Clean,
    Modified,
    New,
    Deleted
};

std::string status_to_string(FileStatus s);

struct FileTracker {
    // Add a file to the tracked list (persisted to .minigit/tracked_files)
    static bool track(Repository& repo, const std::filesystem::path& file);

    // Remove a file from the tracked list
    static bool untrack(Repository& repo, const std::filesystem::path& file);

    // Load the tracked file list from disk
    static std::vector<std::filesystem::path> list(const Repository& repo);

    // Save the tracked file list to disk
    static bool save_list(const Repository& repo,
                          const std::vector<std::filesystem::path>& files);

    // Get status of all tracked files by comparing with the latest commit snapshot
    static std::map<std::filesystem::path, FileStatus>
    get_all_status(const Repository& repo);

    // Get status of a single tracked file
    static FileStatus get_status(const Repository& repo,
                                 const std::filesystem::path& file);
};

} // namespace minigit

#endif // MINIGIT_CORE_FILE_TRACKER_H


// ============================================================
// FILE 3 : include\core\commit.h
// ============================================================

#ifndef MINIGIT_CORE_COMMIT_H
#define MINIGIT_CORE_COMMIT_H

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace minigit {

struct Repository;

struct CommitNode {
    std::string id;
    std::string message;
    std::string timestamp;
    std::string parent_id;
    std::map<std::string, std::string> files;  // encoded_name -> original_path

    // Create a new commit on the current branch
    static bool create(Repository& repo, const std::string& message);

    // Load a CommitNode from its meta.json
    static bool load(const std::filesystem::path& commit_dir, CommitNode& node);

    // Get the next commit ID for a branch (e.g. "001", "002", ...)
    static std::string next_id(const Repository& repo);

    // Get the current HEAD commit ID for the current branch (empty if none)
    static std::string head_id(const Repository& repo);

    // Checkout: restore workspace files to the state of a given commit
    static bool checkout(const Repository& repo, const std::string& commit_id);
};

} // namespace minigit

#endif // MINIGIT_CORE_COMMIT_H


// ============================================================
// FILE 4 : include\core\branch.h
// ============================================================

#ifndef MINIGIT_CORE_BRANCH_H
#define MINIGIT_CORE_BRANCH_H

#include <string>
#include <vector>
#include <filesystem>

namespace minigit {

struct Repository;

struct BranchManager {
    // Create a new branch forking from a specific commit on the current branch.
    // Copies all commits up to and including from_commit_id into the new branch.
    static bool create(Repository& repo,
                       const std::string& branch_name,
                       const std::string& from_commit_id);

    // Switch to an existing branch, restoring workspace to its HEAD commit.
    static bool switch_to(Repository& repo, const std::string& branch_name);

    // Delete a branch (cannot delete current branch)
    static bool remove(Repository& repo, const std::string& branch_name);

    // List all branch names
    static std::vector<std::string> list(const Repository& repo);
};

} // namespace minigit

#endif // MINIGIT_CORE_BRANCH_H


// ============================================================
// FILE 5 : include\core\save.h
// ============================================================

#ifndef MINIGIT_CORE_SAVE_H
#define MINIGIT_CORE_SAVE_H

#include <filesystem>
#include <string>

namespace minigit {

struct Repository;

struct SaveManager {
    // 2.1 Create a save (quick checkpoint) on the current branch.
    //     Overwrites any existing save for this branch.
    static bool create(Repository& repo);

    // 2.2 Restore workspace from the current branch's save.
    static bool restore(const Repository& repo);

    // 2.3 Promote the save to a formal commit with the given message.
    //     Deletes the save after promotion.
    static bool promote(Repository& repo, const std::string& message);

    // Check whether a save exists for the current branch.
    static bool exists(const Repository& repo);

    // Get the save directory path for the current branch.
    static std::filesystem::path save_dir(const Repository& repo);
};

} // namespace minigit

#endif // MINIGIT_CORE_SAVE_H


// ============================================================
// FILE 6 : include\core\snapshot.h
// ============================================================

#ifndef MINIGIT_CORE_SNAPSHOT_H
#define MINIGIT_CORE_SNAPSHOT_H

#include <filesystem>
#include <vector>
#include <map>
#include <string>

namespace minigit {

struct Snapshot {
    // Copy all listed files into dest_dir/files/ using encoded filenames.
    // Returns a map: encoded_filename -> normalized_original_path
    static std::map<std::string, std::string>
    create(const std::vector<std::filesystem::path>& files,
           const std::filesystem::path& dest_dir);

    // Restore files from snapshot_dir/files/ back to their original paths.
    // file_map: encoded_filename -> original_path (from meta.json)
    static bool restore(const std::filesystem::path& snapshot_dir,
                        const std::map<std::string, std::string>& file_map);
};

} // namespace minigit

#endif // MINIGIT_CORE_SNAPSHOT_H


// ============================================================
// FILE 7 : include\tui\app.h
// ============================================================

#ifndef MINIGIT_TUI_APP_H
#define MINIGIT_TUI_APP_H

#include <filesystem>

namespace minigit {

int run_app(const std::filesystem::path& repo_path);

} // namespace minigit

#endif // MINIGIT_TUI_APP_H


// ============================================================
// FILE 8 : include\tui\tree_view.h
// ============================================================

#ifndef MINIGIT_TUI_TREE_VIEW_H
#define MINIGIT_TUI_TREE_VIEW_H

#include <string>
#include <vector>
#include <filesystem>

namespace minigit {

struct Repository;

// A node in the visual commit tree
struct TreeNode {
    std::string commit_id;
    std::string message;
    std::string timestamp;
    std::string branch_name;
    bool is_head = false;      // HEAD of this branch?
    bool is_current = false;   // On the currently active branch?
};

// A column in the tree = one branch
struct TreeColumn {
    std::string branch_name;
    std::vector<TreeNode> nodes;  // Ordered from oldest (top) to newest (bottom)
};

struct TreeData {
    std::vector<TreeColumn> columns;
    int selected_col = 0;
    int selected_row = 0;

    // Build tree data from repository
    static TreeData build(const Repository& repo);

    // Get currently selected node (nullptr if empty)
    const TreeNode* selected_node() const;
};

} // namespace minigit

#endif // MINIGIT_TUI_TREE_VIEW_H


// ============================================================
// FILE 9 : include\utils\path_utils.h
// ============================================================

#ifndef MINIGIT_UTILS_PATH_UTILS_H
#define MINIGIT_UTILS_PATH_UTILS_H

#include <filesystem>
#include <string>

namespace minigit {

// Convert any path to POSIX-style '/' separator string
std::string normalize_path(const std::filesystem::path& p);

// Encode a path into a safe filename: '/' -> '_', ':' -> '_'
std::string encode_path(const std::filesystem::path& p);

// Return the platform-native display string (\ on Windows, / on Linux)
std::string display_path(const std::filesystem::path& p);

// Resolve the .minigit directory location from a given working path
std::filesystem::path find_minigit_root(const std::filesystem::path& working_dir);

} // namespace minigit

#endif // MINIGIT_UTILS_PATH_UTILS_H


// ============================================================
// FILE 10 : include\utils\time_utils.h
// ============================================================

#ifndef MINIGIT_UTILS_TIME_UTILS_H
#define MINIGIT_UTILS_TIME_UTILS_H

#include <string>

namespace minigit {

std::string current_timestamp();

} // namespace minigit

#endif // MINIGIT_UTILS_TIME_UTILS_H


// ============================================================
// FILE 11 : src\main.cpp
// ============================================================

#include "tui/app.h"
#include <filesystem>

int main() {
    std::filesystem::path repo_path = std::filesystem::current_path();
    return minigit::run_app(repo_path);
}


// ============================================================
// FILE 12 : src\core\repository.cpp
// ============================================================

#include "core/repository.h"
#include "utils/path_utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace minigit {

namespace fs = std::filesystem;
using json = nlohmann::json;

// 鈹€鈹€ Path helpers 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ init 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ load 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ save_config 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ load_config 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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


// ============================================================
// FILE 13 : src\core\file_tracker.cpp
// ============================================================

#include "core/file_tracker.h"
#include "core/repository.h"
#include "utils/path_utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace minigit {

namespace fs = std::filesystem;
using json = nlohmann::json;

// 鈹€鈹€ Helpers 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

std::string status_to_string(FileStatus s) {
    switch (s) {
        case FileStatus::Clean:    return "clean";
        case FileStatus::Modified: return "modified";
        case FileStatus::New:      return "new";
        case FileStatus::Deleted:  return "deleted";
    }
    return "unknown";
}

// Find the latest commit directory for the current branch.
// Returns empty path if no commits exist yet.
static fs::path latest_commit_dir(const Repository& repo) {
    fs::path commits_dir = repo.branch_dir(repo.current_branch) / "commits";
    if (!fs::exists(commits_dir)) return {};

    // Commit dirs are named 001, 002, ... 鈥?find the highest
    std::string max_id;
    for (auto& entry : fs::directory_iterator(commits_dir)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name > max_id) max_id = name;
        }
    }
    if (max_id.empty()) return {};
    return commits_dir / max_id;
}

// 鈹€鈹€ list / save_list 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

std::vector<fs::path> FileTracker::list(const Repository& repo) {
    std::vector<fs::path> result;
    try {
        std::ifstream f(repo.tracked_files_path());
        if (!f) return result;
        json arr = json::parse(f);
        for (auto& item : arr) {
            result.emplace_back(item.get<std::string>());
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to read tracked_files: " << e.what() << "\n";
    }
    return result;
}

bool FileTracker::save_list(const Repository& repo,
                            const std::vector<fs::path>& files) {
    try {
        json arr = json::array();
        for (auto& f : files) {
            arr.push_back(normalize_path(f));
        }
        std::ofstream out(repo.tracked_files_path());
        if (!out) return false;
        out << arr.dump(2);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save tracked_files: " << e.what() << "\n";
        return false;
    }
}

// 鈹€鈹€ track 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

bool FileTracker::track(Repository& repo, const fs::path& file) {
    fs::path abs_file = fs::absolute(file);

    // Check that the file actually exists
    if (!fs::exists(abs_file)) {
        std::cerr << "File does not exist: " << display_path(abs_file) << "\n";
        return false;
    }

    // Load current list
    auto files = list(repo);

    // Check for duplicates (compare normalized paths)
    std::string norm = normalize_path(abs_file);
    for (auto& f : files) {
        if (normalize_path(f) == norm) {
            std::cout << "Already tracking: " << display_path(abs_file) << "\n";
            return true;
        }
    }

    // Add and save
    files.push_back(abs_file);
    if (!save_list(repo, files)) return false;

    std::cout << "Tracking: " << display_path(abs_file) << "\n";
    return true;
}

// 鈹€鈹€ untrack 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

bool FileTracker::untrack(Repository& repo, const fs::path& file) {
    fs::path abs_file = fs::absolute(file);
    std::string norm = normalize_path(abs_file);

    auto files = list(repo);
    auto it = std::remove_if(files.begin(), files.end(),
        [&](const fs::path& f) { return normalize_path(f) == norm; });

    if (it == files.end()) {
        std::cerr << "Not tracking: " << display_path(abs_file) << "\n";
        return false;
    }

    files.erase(it, files.end());
    if (!save_list(repo, files)) return false;

    std::cout << "Untracked: " << display_path(abs_file) << "\n";
    return true;
}

// 鈹€鈹€ get_status (single file) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

FileStatus FileTracker::get_status(const Repository& repo,
                                   const fs::path& file) {
    fs::path abs_file = fs::absolute(file);

    // If the file doesn't exist on disk, it was deleted
    if (!fs::exists(abs_file)) {
        return FileStatus::Deleted;
    }

    // Find the latest commit's snapshot
    fs::path commit_dir = latest_commit_dir(repo);
    if (commit_dir.empty()) {
        // No commits yet 鈥?every tracked file is "new"
        return FileStatus::New;
    }

    // Look for this file's snapshot in the commit
    fs::path snapshot_file = commit_dir / "files" / encode_path(abs_file);
    if (!fs::exists(snapshot_file)) {
        // File wasn't in the last commit 鈥?it's new
        return FileStatus::New;
    }

    // Compare file sizes 鈥?if different, it's modified
    auto current_size = fs::file_size(abs_file);
    auto snapshot_size = fs::file_size(snapshot_file);
    if (current_size != snapshot_size) {
        return FileStatus::Modified;
    }

    // Compare last-write times 鈥?if current is newer, it's modified
    auto current_time = fs::last_write_time(abs_file);
    auto snapshot_time = fs::last_write_time(snapshot_file);
    if (current_time != snapshot_time) {
        return FileStatus::Modified;
    }

    return FileStatus::Clean;
}

// 鈹€鈹€ get_all_status 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

std::map<fs::path, FileStatus>
FileTracker::get_all_status(const Repository& repo) {
    std::map<fs::path, FileStatus> result;
    auto files = list(repo);
    for (auto& f : files) {
        result[f] = get_status(repo, f);
    }
    return result;
}

} // namespace minigit


// ============================================================
// FILE 14 : src\core\commit.cpp
// ============================================================

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

// 鈹€鈹€ next_id 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ head_id 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ create 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ load 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ checkout 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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


// ============================================================
// FILE 15 : src\core\branch.cpp
// ============================================================

#include "core/branch.h"
#include "core/repository.h"
#include "core/commit.h"
#include "core/snapshot.h"
#include "utils/path_utils.h"
#include <iostream>
#include <algorithm>

namespace minigit {

namespace fs = std::filesystem;

// 鈹€鈹€ list 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

std::vector<std::string> BranchManager::list(const Repository& repo) {
    std::vector<std::string> names;
    fs::path br_dir = repo.branches_dir();
    if (!fs::exists(br_dir)) return names;

    for (auto& entry : fs::directory_iterator(br_dir)) {
        if (entry.is_directory()) {
            names.push_back(entry.path().filename().string());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

// 鈹€鈹€ create 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

bool BranchManager::create(Repository& repo,
                           const std::string& branch_name,
                           const std::string& from_commit_id) {
    // Validate branch name
    if (branch_name.empty()) {
        std::cerr << "Branch name cannot be empty.\n";
        return false;
    }
    // Reject invalid characters (spaces, slashes, special chars)
    for (char c : branch_name) {
        if (c == ' ' || c == '/' || c == '\\' || c == '.' ||
            c == ':' || c == '~' || c == '^' || c == '?' || c == '*') {
            std::cerr << "Invalid branch name: contains '" << c << "'\n";
            return false;
        }
    }

    // Check for duplicate
    fs::path new_branch_dir = repo.branches_dir() / branch_name;
    if (fs::exists(new_branch_dir)) {
        std::cerr << "Branch already exists: " << branch_name << "\n";
        return false;
    }

    // Verify source commit exists
    fs::path src_commit_dir = repo.branch_dir(repo.current_branch)
                              / "commits" / from_commit_id;
    if (!fs::exists(src_commit_dir)) {
        std::cerr << "Commit " << from_commit_id << " not found on ["
                  << repo.current_branch << "].\n";
        return false;
    }

    try {
        // Create new branch directory
        fs::path new_commits_dir = new_branch_dir / "commits";
        fs::create_directories(new_commits_dir);

        // Copy all commits up to and including from_commit_id
        fs::path src_commits_dir = repo.branch_dir(repo.current_branch) / "commits";
        for (auto& entry : fs::directory_iterator(src_commits_dir)) {
            if (entry.is_directory()) {
                std::string id = entry.path().filename().string();
                if (id <= from_commit_id) {
                    fs::copy(entry.path(), new_commits_dir / id,
                             fs::copy_options::recursive);
                }
            }
        }

        std::cout << "Created branch [" << branch_name
                  << "] from commit " << from_commit_id << "\n";
        return true;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Branch creation failed: " << e.what() << "\n";
        return false;
    }
}

// 鈹€鈹€ switch_to 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

bool BranchManager::switch_to(Repository& repo, const std::string& branch_name) {
    fs::path target_dir = repo.branches_dir() / branch_name;
    if (!fs::exists(target_dir)) {
        std::cerr << "Branch not found: " << branch_name << "\n";
        return false;
    }

    // Update current branch
    repo.current_branch = branch_name;
    if (!repo.save_config()) return false;

    // Get HEAD commit of target branch
    std::string head = CommitNode::head_id(repo);
    if (head.empty()) {
        std::cout << "Switched to [" << branch_name << "] (no commits)\n";
        return true;
    }

    // Restore workspace to HEAD commit
    if (!CommitNode::checkout(repo, head)) return false;

    std::cout << "Switched to [" << branch_name << "] at commit " << head << "\n";
    return true;
}

// 鈹€鈹€ remove 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

bool BranchManager::remove(Repository& repo, const std::string& branch_name) {
    if (branch_name.empty()) {
        std::cerr << "Branch name cannot be empty.\n";
        return false;
    }
    if (branch_name == repo.current_branch) {
        std::cerr << "Cannot delete current branch [" << branch_name << "]\n";
        return false;
    }

    fs::path target_dir = repo.branches_dir() / branch_name;
    if (!fs::exists(target_dir)) {
        std::cerr << "Branch not found: " << branch_name << "\n";
        return false;
    }

    try {
        fs::remove_all(target_dir);
        std::cout << "Deleted branch [" << branch_name << "]\n";
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Branch deletion failed: " << e.what() << "\n";
        return false;
    }
}

} // namespace minigit


// ============================================================
// FILE 16 : src\core\save.cpp
// ============================================================

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

// 鈹€鈹€ save_dir 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

fs::path SaveManager::save_dir(const Repository& repo) {
    return repo.branch_dir(repo.current_branch) / "save";
}

// 鈹€鈹€ exists 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

bool SaveManager::exists(const Repository& repo) {
    return fs::exists(save_dir(repo) / "meta.json");
}

// 鈹€鈹€ create (2.1) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ restore (2.2) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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

// 鈹€鈹€ promote (2.3) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

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


// ============================================================
// FILE 17 : src\core\snapshot.cpp
// ============================================================

#include "core/snapshot.h"
#include "utils/path_utils.h"
#include <iostream>

namespace minigit {

namespace fs = std::filesystem;

std::map<std::string, std::string>
Snapshot::create(const std::vector<fs::path>& files,
                 const fs::path& dest_dir) {
    std::map<std::string, std::string> file_map;
    fs::path files_dir = dest_dir / "files";
    fs::create_directories(files_dir);

    for (auto& src : files) {
        fs::path abs_src = fs::absolute(src);
        if (!fs::exists(abs_src)) {
            std::cerr << "Snapshot skip (missing): "
                      << display_path(abs_src) << "\n";
            continue;
        }

        std::string encoded = encode_path(abs_src);
        fs::path dest = files_dir / encoded;

        try {
            fs::copy_file(abs_src, dest,
                          fs::copy_options::overwrite_existing);
            file_map[encoded] = normalize_path(abs_src);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Snapshot copy failed: " << e.what() << "\n";
        }
    }
    return file_map;
}

bool Snapshot::restore(const fs::path& snapshot_dir,
                       const std::map<std::string, std::string>& file_map) {
    fs::path files_dir = snapshot_dir / "files";
    if (!fs::exists(files_dir)) {
        std::cerr << "Snapshot directory not found: "
                  << display_path(files_dir) << "\n";
        return false;
    }

    bool all_ok = true;
    for (auto& [encoded, original] : file_map) {
        fs::path src = files_dir / encoded;
        fs::path dest = fs::path(original);

        if (!fs::exists(src)) {
            std::cerr << "Snapshot file missing: " << encoded << "\n";
            all_ok = false;
            continue;
        }

        try {
            // Ensure destination directory exists
            fs::create_directories(dest.parent_path());
            fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Restore failed: " << e.what() << "\n";
            all_ok = false;
        }
    }
    return all_ok;
}

} // namespace minigit


// ============================================================
// FILE 18 : src\tui\app.cpp
// ============================================================

#include "tui/app.h"
#include "core/repository.h"
#include "core/file_tracker.h"
#include "core/commit.h"
#include "core/branch.h"
#include "core/save.h"
#include "tui/tree_view.h"
#include "utils/path_utils.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <set>
#include <sstream>

namespace minigit {

namespace fs = std::filesystem;
using namespace ftxui;

// 鈹€鈹€ App State 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

enum class ViewMode { Main, Files, FileBrowser, CommitDialog, BranchCreate, Tree, Help };

struct AppState {
    Repository repo;
    ViewMode view = ViewMode::Main;
    std::string status_msg;
    std::string commit_msg;
    std::string branch_name;
    std::string branch_from_commit;  // commit ID to branch from (empty = HEAD)
    // File browser state
    fs::path browse_dir;                         // current browsing directory
    std::vector<fs::path> browse_entries;         // files/dirs in browse_dir
    int browse_cursor = 0;                        // cursor position
    std::set<fs::path> browse_selected;           // selected files (absolute)
    int file_selected = 0;
    TreeData tree;
};

// 鈹€鈹€ Status Bar (3.2) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static Element render_status_bar(const AppState& state) {
    auto tracked = FileTracker::list(state.repo);
    std::string head = CommitNode::head_id(state.repo);
    if (head.empty()) head = "(none)";

    return hbox({
        text(" [") | bold,
        text(state.repo.current_branch) | bold | color(Color::Cyan),
        text("] "),
        text("HEAD: " + head) | dim,
        text("  |  "),
        text("Files: " + std::to_string(tracked.size())) | dim,
        filler(),
        text(state.status_msg) | color(Color::Yellow),
        text(" "),
    });
}

// 鈹€鈹€ File List View (3.3) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static Element render_file_view(AppState& state) {
    auto statuses = FileTracker::get_all_status(state.repo);
    Elements lines;
    int idx = 0;
    for (auto& [path, status] : statuses) {
        std::string label = normalize_path(path);
        std::string st = status_to_string(status);
        Color c = Color::White;
        if (status == FileStatus::New) c = Color::Green;
        else if (status == FileStatus::Modified) c = Color::Yellow;
        else if (status == FileStatus::Deleted) c = Color::Red;

        auto line = hbox({
            text(idx == state.file_selected ? " > " : "   "),
            text(label) | size(WIDTH, LESS_THAN, 60),
            filler(),
            text("[" + st + "]") | color(c),
        });
        lines.push_back(line);
        idx++;
    }
    if (lines.empty()) {
        lines.push_back(text("  (no tracked files)") | dim);
    }

    return vbox({
        text(" Tracked Files") | bold | underlined,
        separator(),
        vbox(lines) | vscroll_indicator | frame | flex,
        separator(),
        text(" [Esc] Back  [Up/Down] Navigate  [A] Add file  [D] Untrack") | dim,
    });
}

// 鈹€鈹€ File Browser helpers 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static void refresh_browse(AppState& state) {
    state.browse_entries.clear();
    if (!fs::exists(state.browse_dir)) return;
    for (auto& entry : fs::directory_iterator(state.browse_dir)) {
        std::string name = entry.path().filename().string();
        if (name == ".minigit" || name == ".git") continue;  // hide internal dirs
        state.browse_entries.push_back(entry.path());
    }
    std::sort(state.browse_entries.begin(), state.browse_entries.end(),
        [](const fs::path& a, const fs::path& b) {
            bool da = fs::is_directory(a), db = fs::is_directory(b);
            if (da != db) return da > db;  // dirs first
            return a.filename() < b.filename();
        });
    if (state.browse_cursor >= (int)state.browse_entries.size())
        state.browse_cursor = std::max(0, (int)state.browse_entries.size() - 1);
}

static Element render_file_browser(const AppState& state) {
    // Relative path for display
    std::string rel = fs::relative(state.browse_dir, state.repo.root_path).string();
    if (rel == ".") rel = "/";

    Elements lines;
    auto already_tracked = FileTracker::list(state.repo);
    std::set<std::string> tracked_set;
    for (auto& p : already_tracked) tracked_set.insert(fs::absolute(p).string());

    for (int i = 0; i < (int)state.browse_entries.size(); i++) {
        auto& p = state.browse_entries[i];
        bool is_dir = fs::is_directory(p);
        bool selected = state.browse_selected.count(p) > 0;
        bool is_tracked = tracked_set.count(fs::absolute(p).string()) > 0;
        bool is_cursor = (i == state.browse_cursor);

        std::string prefix = is_cursor ? " > " : "   ";
        std::string check = is_dir ? "[/]" : (selected ? "[x]" : "[ ]");
        std::string name = p.filename().string() + (is_dir ? "/" : "");

        auto line = hbox({
            text(prefix),
            text(check + " ") | (selected ? color(Color::Green) : nothing),
            text(name) | (is_dir ? bold : nothing),
            is_tracked ? text(" (tracked)") | dim : text(""),
        });
        if (is_cursor) line = line | inverted;
        lines.push_back(line);
    }

    if (lines.empty()) {
        lines.push_back(text("   (empty directory)") | dim);
    }

    int sel_count = (int)state.browse_selected.size();

    return vbox({
        text(" File Browser") | bold | underlined,
        hbox({text(" "), text(rel) | dim}),
        separator(),
        vbox(lines) | vscroll_indicator | frame | flex,
        separator(),
        hbox({
            text(" Selected: " + std::to_string(sel_count) + " file(s) ") | (sel_count > 0 ? color(Color::Green) : nothing),
        }),
        text(" [Space] Select  [Enter] Open dir/Confirm  [Backspace] Parent  [Esc] Cancel") | dim,
    });
}

// 鈹€鈹€ Commit Dialog (3.4) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static Element render_commit_dialog(const AppState& state) {
    return vbox({
        text(" Create Commit") | bold | underlined,
        separator(),
        hbox({
            text(" Message: "),
            text(state.commit_msg.empty() ? "_" : state.commit_msg) | inverted,
        }),
        separator(),
        text(" [Enter] Confirm  [Esc] Cancel") | dim,
    }) | border | center;
}

// 鈹€鈹€ Branch Create Dialog (3.5a) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static Element render_branch_create(const AppState& state) {
    std::string from = state.branch_from_commit.empty()
                       ? CommitNode::head_id(state.repo)
                       : state.branch_from_commit;
    return vbox({
        text(" Create Branch") | bold | underlined,
        separator(),
        hbox({text(" Name: "), text(state.branch_name.empty() ? "_" : state.branch_name) | inverted}),
        text(" (will fork from commit " + from + ")") | dim,
        separator(),
        text(" [Enter] Confirm  [Esc] Cancel") | dim,
    }) | border | center;
}


// 鈹€鈹€ Help View 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static Element render_help() {
    return vbox({
        text(" Mini-Git Help") | bold | underlined,
        separator(),
        text("  Q       Quit"),
        text("  F       Toggle file list"),
        text("  C       Create commit"),
        text("  T       Commit tree (branch/version switch)"),
        text("  S       Quick save"),
        text("  R       Restore save"),
        text("  P       Promote save to commit"),
        text("  ?       Toggle help"),
        separator(),
        text(" [Esc] Close") | dim,
    }) | border | center;
}

// 鈹€鈹€ Tree View (4.2) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static Element render_tree(AppState& state) {
    auto& tree = state.tree;
    if (tree.columns.empty()) {
        return vbox({
            text(" Commit Tree") | bold | underlined,
            separator(),
            text("  (no branches or commits)") | dim,
            separator(),
            text(" [Esc] Back") | dim,
        });
    }

    // Find the max number of commits across all branches for row alignment
    int max_rows = 0;
    for (auto& col : tree.columns) {
        max_rows = std::max(max_rows, (int)col.nodes.size());
    }

    // Build header: branch names
    Elements header_cells;
    for (int c = 0; c < (int)tree.columns.size(); c++) {
        bool is_sel_col = (c == tree.selected_col);
        bool is_current = (tree.columns[c].branch_name == state.repo.current_branch);
        auto cell = text(" " + tree.columns[c].branch_name + " ")
                    | (is_current ? bold : nothing)
                    | (is_sel_col ? color(Color::Cyan) : nothing);
        header_cells.push_back(cell | size(WIDTH, EQUAL, 20));
    }

    // Build rows
    Elements rows;
    for (int r = 0; r < max_rows; r++) {
        Elements row_cells;
        for (int c = 0; c < (int)tree.columns.size(); c++) {
            auto& col = tree.columns[c];
            if (r < (int)col.nodes.size()) {
                auto& node = col.nodes[r];
                bool selected = (c == tree.selected_col && r == tree.selected_row);

                // Node symbol
                std::string sym;
                if (node.is_head) sym = selected ? ">*" : " *";
                else              sym = selected ? "> " : "  ";

                // Commit display
                std::string label = sym + node.commit_id + " " + node.message;
                if (label.size() > 18) label = label.substr(0, 18) + "..";

                Color clr = Color::White;
                if (node.is_head && node.is_current) clr = Color::Green;
                else if (node.is_head) clr = Color::Yellow;
                else if (selected) clr = Color::Cyan;

                auto cell = text(label) | color(clr);
                if (selected) cell = cell | bold;
                row_cells.push_back(cell | size(WIDTH, EQUAL, 20));
            } else {
                // Empty cell with connector if branch has fewer commits
                std::string filler_str = (r < (int)col.nodes.size() + 1 && !col.nodes.empty())
                                         ? "  |" : "   ";
                row_cells.push_back(text(filler_str) | dim | size(WIDTH, EQUAL, 20));
            }
        }
        rows.push_back(hbox(row_cells));
    }

    // Detail panel for selected node
    Elements detail;
    auto* sel = tree.selected_node();
    if (sel) {
        detail.push_back(separator());
        detail.push_back(hbox({
            text(" Commit: ") | bold, text(sel->commit_id),
            text("  Branch: ") | bold, text(sel->branch_name),
        }));
        detail.push_back(hbox({
            text(" Message: ") | bold, text(sel->message),
        }));
        detail.push_back(hbox({
            text(" Time: ") | bold, text(sel->timestamp),
        }));
    }

    return vbox({
        text(" Commit Tree") | bold | underlined,
        separator(),
        hbox(header_cells),
        separator(),
        vbox(rows) | vscroll_indicator | frame | flex,
        vbox(detail),
        separator(),
        text(" [Esc] Back  [Arrows] Navigate  [Enter] Checkout version  [B] New branch  [D] Delete branch") | dim,
    });
}

// 鈹€鈹€ Main View 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

static Element render_main(AppState& state) {
    bool has_save = SaveManager::exists(state.repo);
    auto tracked = FileTracker::list(state.repo);
    auto statuses = FileTracker::get_all_status(state.repo);
    std::string head = CommitNode::head_id(state.repo);
    auto branches = BranchManager::list(state.repo);

    // Count modified / new / deleted
    int n_modified = 0, n_new = 0, n_deleted = 0;
    for (auto& [path, st] : statuses) {
        if (st == FileStatus::Modified) n_modified++;
        else if (st == FileStatus::New) n_new++;
        else if (st == FileStatus::Deleted) n_deleted++;
    }
    int n_changed = n_modified + n_new + n_deleted;

    // Load HEAD commit info
    std::string head_msg = "(no commits)";
    std::string head_time = "";
    if (!head.empty()) {
        CommitNode node;
        fs::path cdir = state.repo.branch_dir(state.repo.current_branch)
                        / "commits" / head;
        if (CommitNode::load(cdir, node)) {
            head_msg = head + " - " + node.message;
            head_time = node.timestamp;
        }
    }

    // Branch list string
    std::string branch_str;
    for (auto& b : branches) {
        if (!branch_str.empty()) branch_str += ", ";
        branch_str += (b == state.repo.current_branch) ? "[" + b + "]" : b;
    }

    // Smart hint
    std::string hint;
    if (tracked.empty()) {
        hint = "No tracked files. Press [F] then [A] to add files.";
    } else if (head.empty()) {
        hint = std::to_string(tracked.size()) + " file(s) tracked. Press [C] to make first commit.";
    } else if (n_changed > 0) {
        hint = std::to_string(n_changed) + " file(s) changed. Press [C] to commit or [S] to save.";
    } else {
        hint = "Working tree clean.";
    }

    // Build layout
    Elements rows;
    rows.push_back(text(""));
    rows.push_back(text("  Mini-Git") | bold);
    rows.push_back(text(""));
    rows.push_back(hbox({text("    Branch:   ") | dim, text(state.repo.current_branch) | bold}));
    rows.push_back(hbox({text("    HEAD:     ") | dim, text(head_msg)}));
    if (!head_time.empty())
        rows.push_back(hbox({text("              ") | dim, text(head_time) | dim}));
    rows.push_back(hbox({text("    Files:    ") | dim,
        text(std::to_string(tracked.size()) + " tracked"),
        n_modified > 0 ? text(", " + std::to_string(n_modified) + " modified") | color(Color::Yellow) : text(""),
        n_new > 0      ? text(", " + std::to_string(n_new) + " new") | color(Color::Green) : text(""),
        n_deleted > 0  ? text(", " + std::to_string(n_deleted) + " deleted") | color(Color::Red) : text(""),
    }));
    rows.push_back(hbox({text("    Branches: ") | dim, text(branch_str)}));
    rows.push_back(hbox({text("    Save:     ") | dim,
        has_save ? text("checkpoint available") | color(Color::Green)
                 : text("none") | dim}));
    rows.push_back(text(""));
    rows.push_back(separator());
    rows.push_back(text("    " + hint) | color(Color::Cyan));
    rows.push_back(text(""));

    return vbox(rows);
}

// 鈹€鈹€ run_app (3.1) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

int run_app(const fs::path& repo_path) {
    AppState state;

    // Load or init repository
    if (!Repository::load(repo_path, state.repo)) {
        Repository::init(repo_path);
        Repository::load(repo_path, state.repo);
    }

    auto screen = ScreenInteractive::Fullscreen();

    auto main_renderer = Renderer([&] {
        Element content;
        switch (state.view) {
            case ViewMode::Files:         content = render_file_view(state); break;
            case ViewMode::FileBrowser:   content = render_file_browser(state); break;
            case ViewMode::CommitDialog:  content = render_commit_dialog(state); break;
            case ViewMode::BranchCreate:  content = render_branch_create(state); break;
            case ViewMode::Tree:          content = render_tree(state); break;
            case ViewMode::Help:          content = render_help(); break;
            default:                      content = render_main(state); break;
        }

        return vbox({
            render_status_bar(state),
            separator(),
            content | flex,
            separator(),
            text(" [C]ommit [S]ave [F]iles [T]ree [?]Help [Q]uit ") | center | dim,
        }) | border;
    });

    auto main_component = CatchEvent(main_renderer, [&](Event event) {
        // 鈹€鈹€ Global: Quit 鈹€鈹€
        if (state.view == ViewMode::Main) {
            if (event == Event::Character('q') || event == Event::Character('Q')) {
                screen.Exit();
                return true;
            }
        }

        // 鈹€鈹€ Escape: back to main 鈹€鈹€
        if (event == Event::Escape) {
            state.view = ViewMode::Main;
            state.commit_msg.clear();
            state.branch_name.clear();
            return true;
        }

        // 鈹€鈹€ Commit Dialog input 鈹€鈹€
        if (state.view == ViewMode::CommitDialog) {
            if (event == Event::Return) {
                if (!state.commit_msg.empty()) {
                    auto tracked = FileTracker::list(state.repo);
                    if (tracked.empty()) {
                        state.status_msg = "Nothing to commit: no tracked files";
                    } else if (CommitNode::create(state.repo, state.commit_msg)) {
                        state.status_msg = "Committed [" + CommitNode::head_id(state.repo) + "]: " + state.commit_msg;
                    } else {
                        state.status_msg = "Commit failed (snapshot error?)";
                    }
                    state.commit_msg.clear();
                    state.view = ViewMode::Main;
                }
                return true;
            }
            if (event == Event::Backspace) {
                if (!state.commit_msg.empty()) state.commit_msg.pop_back();
                return true;
            }
            if (event.is_character()) {
                state.commit_msg += event.character();
                return true;
            }
            return false;
        }

        // 鈹€鈹€ Branch Create input 鈹€鈹€
        if (state.view == ViewMode::BranchCreate) {
            if (event == Event::Return) {
                if (!state.branch_name.empty()) {
                    std::string from = state.branch_from_commit.empty()
                                       ? CommitNode::head_id(state.repo)
                                       : state.branch_from_commit;
                    if (from.empty()) {
                        state.status_msg = "Cannot branch: no commits yet";
                    } else if (BranchManager::create(state.repo, state.branch_name, from)) {
                        state.status_msg = "Branch [" + state.branch_name + "] from " + from;
                    } else {
                        // Detailed error (duplicate or invalid name)
                        auto existing = BranchManager::list(state.repo);
                        bool dup = std::find(existing.begin(), existing.end(), state.branch_name) != existing.end();
                        state.status_msg = dup ? "Branch already exists: " + state.branch_name
                                              : "Invalid branch name: " + state.branch_name;
                    }
                    state.branch_name.clear();
                    state.branch_from_commit.clear();
                    state.view = ViewMode::Main;
                }
                return true;
            }
            if (event == Event::Backspace) {
                if (!state.branch_name.empty()) state.branch_name.pop_back();
                return true;
            }
            if (event.is_character()) {
                state.branch_name += event.character();
                return true;
            }
            return false;
        }


        // 鈹€鈹€ File Browser 鈹€鈹€
        if (state.view == ViewMode::FileBrowser) {
            if (event == Event::ArrowUp) {
                if (state.browse_cursor > 0) state.browse_cursor--;
                return true;
            }
            if (event == Event::ArrowDown) {
                if (state.browse_cursor < (int)state.browse_entries.size() - 1)
                    state.browse_cursor++;
                return true;
            }
            // Space: toggle select (files only)
            if (event == Event::Character(' ')) {
                if (!state.browse_entries.empty() &&
                    state.browse_cursor < (int)state.browse_entries.size()) {
                    auto& p = state.browse_entries[state.browse_cursor];
                    if (!fs::is_directory(p)) {
                        if (state.browse_selected.count(p))
                            state.browse_selected.erase(p);
                        else
                            state.browse_selected.insert(p);
                    }
                }
                return true;
            }
            // Enter: open dir OR confirm selection
            if (event == Event::Return) {
                if (!state.browse_entries.empty() &&
                    state.browse_cursor < (int)state.browse_entries.size() &&
                    fs::is_directory(state.browse_entries[state.browse_cursor])) {
                    // Enter directory
                    state.browse_dir = state.browse_entries[state.browse_cursor];
                    state.browse_cursor = 0;
                    refresh_browse(state);
                } else if (!state.browse_selected.empty()) {
                    // Confirm: track all selected files
                    int count = 0;
                    for (auto& p : state.browse_selected) {
                        if (FileTracker::track(state.repo, p)) count++;
                    }
                    state.status_msg = "Tracked " + std::to_string(count) + " file(s)";
                    state.browse_selected.clear();
                    state.view = ViewMode::Files;
                }
                return true;
            }
            // Backspace: go to parent directory
            if (event == Event::Backspace) {
                fs::path parent = state.browse_dir.parent_path();
                if (parent != state.browse_dir && parent >= state.repo.root_path) {
                    state.browse_dir = parent;
                    state.browse_cursor = 0;
                    refresh_browse(state);
                }
                return true;
            }
            return false;
        }

        // 鈹€鈹€ Tree View navigation (4.3 + 4.4) 鈹€鈹€
        if (state.view == ViewMode::Tree) {
            auto& tree = state.tree;
            if (event == Event::ArrowUp) {
                if (tree.selected_row > 0) tree.selected_row--;
                return true;
            }
            if (event == Event::ArrowDown) {
                if (!tree.columns.empty() &&
                    tree.selected_row < (int)tree.columns[tree.selected_col].nodes.size() - 1) {
                    tree.selected_row++;
                }
                return true;
            }
            if (event == Event::ArrowLeft) {
                if (tree.selected_col > 0) {
                    tree.selected_col--;
                    // Clamp row
                    int max_r = (int)tree.columns[tree.selected_col].nodes.size() - 1;
                    if (tree.selected_row > max_r) tree.selected_row = std::max(0, max_r);
                }
                return true;
            }
            if (event == Event::ArrowRight) {
                if (tree.selected_col < (int)tree.columns.size() - 1) {
                    tree.selected_col++;
                    int max_r = (int)tree.columns[tree.selected_col].nodes.size() - 1;
                    if (tree.selected_row > max_r) tree.selected_row = std::max(0, max_r);
                }
                return true;
            }
            // Enter: checkout selected commit
            if (event == Event::Return) {
                auto* sel = tree.selected_node();
                if (sel) {
                    // Switch to the branch if different
                    if (sel->branch_name != state.repo.current_branch) {
                        state.repo.current_branch = sel->branch_name;
                        state.repo.save_config();
                    }
                    if (CommitNode::checkout(state.repo, sel->commit_id)) {
                        state.status_msg = "Checked out " + sel->commit_id + " on [" + sel->branch_name + "]";
                    } else {
                        state.status_msg = "Checkout failed!";
                    }
                    // Rebuild tree
                    state.tree = TreeData::build(state.repo);
                }
                return true;
            }
            // B: create branch from selected node
            if (event == Event::Character('b') || event == Event::Character('B')) {
                auto* sel = tree.selected_node();
                if (sel) {
                    state.branch_from_commit = sel->commit_id;
                    state.view = ViewMode::BranchCreate;
                    state.branch_name.clear();
                }
                return true;
            }
            // D: delete selected branch
            if (event == Event::Character('d') || event == Event::Character('D')) {
                if (!tree.columns.empty() && tree.selected_col < (int)tree.columns.size()) {
                    std::string target = tree.columns[tree.selected_col].branch_name;
                    if (BranchManager::remove(state.repo, target)) {
                        state.status_msg = "Deleted branch [" + target + "]";
                        state.tree = TreeData::build(state.repo);
                        if (tree.selected_col > 0) tree.selected_col--;
                    } else {
                        state.status_msg = (target == state.repo.current_branch)
                            ? "Cannot delete current branch"
                            : "Delete failed: " + target;
                    }
                }
                return true;
            }
            if (event == Event::Character('t') || event == Event::Character('T')) {
                state.view = ViewMode::Main;
                return true;
            }
            return false;
        }

        // 鈹€鈹€ File View navigation 鈹€鈹€
        if (state.view == ViewMode::Files) {
            auto tracked = FileTracker::list(state.repo);
            if (event == Event::ArrowUp) {
                if (state.file_selected > 0) state.file_selected--;
                return true;
            }
            if (event == Event::ArrowDown) {
                if (state.file_selected < (int)tracked.size() - 1) state.file_selected++;
                return true;
            }
            if (event == Event::Character('a') || event == Event::Character('A')) {
                state.view = ViewMode::FileBrowser;
                state.browse_dir = state.repo.root_path;
                state.browse_cursor = 0;
                state.browse_selected.clear();
                refresh_browse(state);
                return true;
            }
            if (event == Event::Character('d') || event == Event::Character('D')) {
                if (!tracked.empty() && state.file_selected < (int)tracked.size()) {
                    FileTracker::untrack(state.repo, tracked[state.file_selected]);
                    state.status_msg = "Untracked file";
                    if (state.file_selected > 0) state.file_selected--;
                }
                return true;
            }
            if (event == Event::Character('f') || event == Event::Character('F')) {
                state.view = ViewMode::Main;
                return true;
            }
            return false;
        }

        // 鈹€鈹€ Main view shortcuts 鈹€鈹€
        if (event == Event::Character('f') || event == Event::Character('F')) {
            state.view = ViewMode::Files;
            state.file_selected = 0;
            return true;
        }
        if (event == Event::Character('c') || event == Event::Character('C')) {
            state.view = ViewMode::CommitDialog;
            state.commit_msg.clear();
            return true;
        }
        if (event == Event::Character('s') || event == Event::Character('S')) {
            if (SaveManager::create(state.repo)) {
                state.status_msg = "Save created!";
            } else {
                state.status_msg = "Save failed (no tracked files?)";
            }
            return true;
        }
        if (event == Event::Character('r') || event == Event::Character('R')) {
            if (SaveManager::restore(state.repo)) {
                state.status_msg = "Save restored!";
            } else {
                state.status_msg = "No save to restore";
            }
            return true;
        }
        if (event == Event::Character('p') || event == Event::Character('P')) {
            if (SaveManager::promote(state.repo, "Promoted save")) {
                state.status_msg = "Save promoted to commit!";
            } else {
                state.status_msg = "Promote failed";
            }
            return true;
        }
        if (event == Event::Character('t') || event == Event::Character('T')) {
            state.tree = TreeData::build(state.repo);
            state.view = ViewMode::Tree;
            return true;
        }
        if (event == Event::Character('?')) {
            state.view = ViewMode::Help;
            return true;
        }

        return false;
    });

    screen.Loop(main_component);
    return 0;
}

} // namespace minigit


// ============================================================
// FILE 19 : src\tui\tree_view.cpp
// ============================================================

#include "tui/tree_view.h"
#include "core/repository.h"
#include "core/commit.h"
#include "core/branch.h"
#include <algorithm>

namespace minigit {

namespace fs = std::filesystem;

// 鈹€鈹€ build (4.1) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

TreeData TreeData::build(const Repository& repo) {
    TreeData data;

    auto branches = BranchManager::list(repo);

    for (auto& branch_name : branches) {
        TreeColumn col;
        col.branch_name = branch_name;

        // Scan commits directory for this branch
        fs::path commits_dir = repo.branches_dir() / branch_name / "commits";
        if (!fs::exists(commits_dir)) {
            data.columns.push_back(col);
            continue;
        }

        // Collect commit dirs and sort
        std::vector<std::string> ids;
        for (auto& entry : fs::directory_iterator(commits_dir)) {
            if (entry.is_directory()) {
                ids.push_back(entry.path().filename().string());
            }
        }
        std::sort(ids.begin(), ids.end());

        // Build nodes
        std::string head_id;
        if (!ids.empty()) head_id = ids.back();

        for (auto& id : ids) {
            TreeNode node;
            node.commit_id = id;
            node.branch_name = branch_name;
            node.is_head = (id == head_id);
            node.is_current = (branch_name == repo.current_branch);

            // Load meta for message and timestamp
            CommitNode cn;
            fs::path cdir = commits_dir / id;
            if (CommitNode::load(cdir, cn)) {
                node.message = cn.message;
                node.timestamp = cn.timestamp;
            }

            col.nodes.push_back(node);
        }

        data.columns.push_back(col);
    }

    // Default selection: current branch, latest commit
    for (int i = 0; i < (int)data.columns.size(); i++) {
        if (data.columns[i].branch_name == repo.current_branch) {
            data.selected_col = i;
            if (!data.columns[i].nodes.empty()) {
                data.selected_row = (int)data.columns[i].nodes.size() - 1;
            }
            break;
        }
    }

    return data;
}

// 鈹€鈹€ selected_node 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

const TreeNode* TreeData::selected_node() const {
    if (columns.empty()) return nullptr;
    if (selected_col < 0 || selected_col >= (int)columns.size()) return nullptr;
    auto& col = columns[selected_col];
    if (selected_row < 0 || selected_row >= (int)col.nodes.size()) return nullptr;
    return &col.nodes[selected_row];
}

} // namespace minigit


// ============================================================
// FILE 20 : src\utils\path_utils.cpp
// ============================================================

#include "utils/path_utils.h"
#include <algorithm>

namespace minigit {

namespace fs = std::filesystem;

std::string normalize_path(const fs::path& p) {
    // generic_string() always returns '/' as separator
    return p.generic_string();
}

std::string encode_path(const fs::path& p) {
    std::string s = normalize_path(p);
    std::replace(s.begin(), s.end(), '/', '_');
    std::replace(s.begin(), s.end(), ':', '_');
    return s;
}

std::string display_path(const fs::path& p) {
    // Returns platform-native format (\ on Windows, / on Linux)
    return p.string();
}

fs::path find_minigit_root(const fs::path& working_dir) {
    // Look for .minigit/ in working_dir and its parents
    fs::path current = fs::absolute(working_dir);
    while (true) {
        fs::path candidate = current / ".minigit";
        if (fs::exists(candidate) && fs::is_directory(candidate)) {
            return current;
        }
        fs::path parent = current.parent_path();
        if (parent == current) {
            // Reached filesystem root, not found
            return fs::path();
        }
        current = parent;
    }
}

} // namespace minigit


// ============================================================
// FILE 21 : src\utils\time_utils.cpp
// ============================================================

#include "utils/time_utils.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace minigit {

std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

} // namespace minigit
