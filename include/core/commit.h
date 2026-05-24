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
