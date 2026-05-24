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

// ── App State ─────────────────────────────────────────────────

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

// ── Status Bar (3.2) ──────────────────────────────────────────

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

// ── File List View (3.3) ──────────────────────────────────────

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

// ── File Browser helpers ──────────────────────────────────────

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

// ── Commit Dialog (3.4) ───────────────────────────────────────

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

// ── Branch Create Dialog (3.5a) ───────────────────────────────

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


// ── Help View ─────────────────────────────────────────────────

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

// ── Tree View (4.2) ────────────────────────────────────────

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

// ── Main View ─────────────────────────────────────────────────

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

// ── run_app (3.1) ─────────────────────────────────────────────

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
        // ── Global: Quit ──
        if (state.view == ViewMode::Main) {
            if (event == Event::Character('q') || event == Event::Character('Q')) {
                screen.Exit();
                return true;
            }
        }

        // ── Escape: back to main ──
        if (event == Event::Escape) {
            state.view = ViewMode::Main;
            state.commit_msg.clear();
            state.branch_name.clear();
            return true;
        }

        // ── Commit Dialog input ──
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

        // ── Branch Create input ──
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


        // ── File Browser ──
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

        // ── Tree View navigation (4.3 + 4.4) ──
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

        // ── File View navigation ──
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

        // ── Main view shortcuts ──
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
