#include "core/branch.h"
#include "core/repository.h"
#include "core/commit.h"
#include "core/snapshot.h"
#include "utils/path_utils.h"
#include <iostream>
#include <algorithm>

namespace minigit {

namespace fs = std::filesystem;

// ── list ──────────────────────────────────────────────────────

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

// ── create ────────────────────────────────────────────────────

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

// ── switch_to ─────────────────────────────────────────────────

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

// ── remove ────────────────────────────────────────────────────

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
