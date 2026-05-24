#include "tui/tree_view.h"
#include "core/repository.h"
#include "core/commit.h"
#include "core/branch.h"
#include <algorithm>

namespace minigit {

namespace fs = std::filesystem;

// ── build (4.1) ───────────────────────────────────────────────

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

// ── selected_node ─────────────────────────────────────────────

const TreeNode* TreeData::selected_node() const {
    if (columns.empty()) return nullptr;
    if (selected_col < 0 || selected_col >= (int)columns.size()) return nullptr;
    auto& col = columns[selected_col];
    if (selected_row < 0 || selected_row >= (int)col.nodes.size()) return nullptr;
    return &col.nodes[selected_row];
}

} // namespace minigit
