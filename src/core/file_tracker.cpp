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

// ── Helpers ───────────────────────────────────────────────────

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

    // Commit dirs are named 001, 002, ... — find the highest
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

// ── list / save_list ──────────────────────────────────────────

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

// ── track ─────────────────────────────────────────────────────

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

// ── untrack ───────────────────────────────────────────────────

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

// ── get_status (single file) ──────────────────────────────────

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
        // No commits yet — every tracked file is "new"
        return FileStatus::New;
    }

    // Look for this file's snapshot in the commit
    fs::path snapshot_file = commit_dir / "files" / encode_path(abs_file);
    if (!fs::exists(snapshot_file)) {
        // File wasn't in the last commit — it's new
        return FileStatus::New;
    }

    // Compare file sizes — if different, it's modified
    auto current_size = fs::file_size(abs_file);
    auto snapshot_size = fs::file_size(snapshot_file);
    if (current_size != snapshot_size) {
        return FileStatus::Modified;
    }

    // Compare last-write times — if current is newer, it's modified
    auto current_time = fs::last_write_time(abs_file);
    auto snapshot_time = fs::last_write_time(snapshot_file);
    if (current_time != snapshot_time) {
        return FileStatus::Modified;
    }

    return FileStatus::Clean;
}

// ── get_all_status ────────────────────────────────────────────

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
