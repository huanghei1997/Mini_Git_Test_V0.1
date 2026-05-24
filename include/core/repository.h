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
