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
