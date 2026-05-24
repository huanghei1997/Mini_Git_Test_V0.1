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
