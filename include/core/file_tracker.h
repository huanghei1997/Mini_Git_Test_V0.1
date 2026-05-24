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
