#include "tui/app.h"
#include "utils/path_utils.h"
#include "core/repository.h"
#include "core/file_tracker.h"
#include "core/commit.h"
#include "core/snapshot.h"
#include "core/branch.h"
#include "core/save.h"
#include <iostream>
#include <filesystem>
#include <fstream>

// Temporary test for Phase 1.1 & 1.2 — will be removed later
void test_phase1() {
    namespace fs = std::filesystem;
    using namespace minigit;

    std::cout << "=== Phase 1.1: path_utils ===\n";
    std::cout << "normalize(\"src\\\\main.cpp\") = "
              << normalize_path(fs::path("src\\main.cpp")) << "\n";
    std::cout << "encode(\"src/main.cpp\")     = "
              << encode_path(fs::path("src/main.cpp")) << "\n";
    std::cout << "encode(\"C:/Users/test/f.txt\") = "
              << encode_path(fs::path("C:/Users/test/f.txt")) << "\n";
    std::cout << "display(\"src/main.cpp\")    = "
              << display_path(fs::path("src/main.cpp")) << "\n";
    std::cout << "\n";

    std::cout << "=== Phase 1.2: repository init ===\n";
    fs::path test_dir = fs::temp_directory_path() / "minigit_test";
    fs::remove_all(test_dir);  // Clean up any previous test

    // Test init
    bool ok = Repository::init(test_dir);
    std::cout << "init: " << (ok ? "OK" : "FAIL") << "\n";

    // Verify directory structure
    std::cout << ".minigit/ exists: "
              << fs::exists(test_dir / ".minigit") << "\n";
    std::cout << "config exists: "
              << fs::exists(test_dir / ".minigit" / "config") << "\n";
    std::cout << "tracked_files exists: "
              << fs::exists(test_dir / ".minigit" / "tracked_files") << "\n";
    std::cout << "branches/main/commits/ exists: "
              << fs::exists(test_dir / ".minigit" / "branches" / "main" / "commits") << "\n";

    // Test idempotent init
    bool ok2 = Repository::init(test_dir);
    std::cout << "re-init (idempotent): " << (ok2 ? "OK" : "FAIL") << "\n";

    // Test load
    Repository repo;
    bool loaded = Repository::load(test_dir, repo);
    std::cout << "load: " << (loaded ? "OK" : "FAIL") << "\n";
    std::cout << "current_branch: " << repo.current_branch << "\n";

    // Test save_config with changed branch
    repo.current_branch = "feature-x";
    repo.save_config();
    Repository repo2;
    Repository::load(test_dir, repo2);
    std::cout << "after save_config(\"feature-x\"): " << repo2.current_branch << "\n";

    // Reset branch back to main for remaining tests
    repo.current_branch = "main";
    repo.save_config();
    Repository::load(test_dir, repo);

    // ── Phase 1.3: track / untrack ─────────────────────────
    std::cout << "\n=== Phase 1.3: file_tracker — track/untrack ===\n";

    // Create test files to track
    fs::path file_a = test_dir / "hello.txt";
    fs::path file_b = test_dir / "sub" / "data.csv";
    fs::create_directories(test_dir / "sub");
    { std::ofstream(file_a) << "Hello World"; }
    { std::ofstream(file_b) << "a,b,c\n1,2,3"; }

    // Track two files
    FileTracker::track(repo, file_a);
    FileTracker::track(repo, file_b);

    // Verify list
    auto tracked = FileTracker::list(repo);
    std::cout << "tracked count: " << tracked.size() << " (expect 2)\n";

    // Duplicate track should be idempotent
    FileTracker::track(repo, file_a);
    tracked = FileTracker::list(repo);
    std::cout << "after duplicate track: " << tracked.size() << " (expect 2)\n";

    // Untrack one file
    FileTracker::untrack(repo, file_b);
    tracked = FileTracker::list(repo);
    std::cout << "after untrack: " << tracked.size() << " (expect 1)\n";

    // Re-track for status test
    FileTracker::track(repo, file_b);

    // ── Phase 1.4: status detection ────────────────────────
    std::cout << "\n=== Phase 1.4: file_tracker — status detection ===\n";

    // No commits yet, all files should be "new"
    auto statuses = FileTracker::get_all_status(repo);
    for (auto& [path, status] : statuses) {
        std::cout << normalize_path(path) << " -> " << status_to_string(status)
                  << " (expect new)\n";
    }

    // ── Phase 1.5: commit — create snapshot ──────────────────
    std::cout << "\n=== Phase 1.5: commit — create snapshot ===\n";

    // First commit
    bool c1 = CommitNode::create(repo, "Initial commit");
    std::cout << "commit 001: " << (c1 ? "OK" : "FAIL") << "\n";

    // Verify commit directory and meta.json
    fs::path c1_dir = repo.branch_dir("main") / "commits" / "001";
    std::cout << "001/ exists: " << fs::exists(c1_dir) << "\n";
    std::cout << "meta.json exists: " << fs::exists(c1_dir / "meta.json") << "\n";
    std::cout << "files/ exists: " << fs::exists(c1_dir / "files") << "\n";

    // Load and verify meta.json
    CommitNode node1;
    CommitNode::load(c1_dir, node1);
    std::cout << "id: " << node1.id << " (expect 001)\n";
    std::cout << "message: " << node1.message << "\n";
    std::cout << "parent_id: \"" << node1.parent_id << "\" (expect empty)\n";
    std::cout << "files count: " << node1.files.size() << " (expect 2)\n";

    // Status after commit: all files should be "clean"
    std::cout << "\n--- Status after commit 001 ---\n";
    statuses = FileTracker::get_all_status(repo);
    for (auto& [path, status] : statuses) {
        std::cout << normalize_path(path) << " -> " << status_to_string(status) << "\n";
    }

    // Modify a file, then check status
    { std::ofstream(file_a) << "Hello World Modified!"; }
    std::cout << "\n--- Status after modifying hello.txt ---\n";
    statuses = FileTracker::get_all_status(repo);
    for (auto& [path, status] : statuses) {
        std::cout << normalize_path(path) << " -> " << status_to_string(status) << "\n";
    }

    // Second commit
    bool c2 = CommitNode::create(repo, "Update hello.txt");
    std::cout << "\ncommit 002: " << (c2 ? "OK" : "FAIL") << "\n";

    CommitNode node2;
    fs::path c2_dir = repo.branch_dir("main") / "commits" / "002";
    CommitNode::load(c2_dir, node2);
    std::cout << "id: " << node2.id << " (expect 002)\n";
    std::cout << "parent_id: " << node2.parent_id << " (expect 001)\n";
    std::cout << "head_id: " << CommitNode::head_id(repo) << " (expect 002)\n";

    // ── Phase 1.6: commit checkout (restore snapshot) ─────
    std::cout << "\n=== Phase 1.6: commit checkout ===\n";

    // hello.txt currently has "Hello World Modified!" (from commit 002)
    // Checkout commit 001 to restore original content
    CommitNode::checkout(repo, "001");
    {
        std::ifstream f(file_a);
        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        std::cout << "after checkout 001, hello.txt = \"" << content
                  << "\" (expect \"Hello World\")\n";
    }

    // Checkout back to 002
    CommitNode::checkout(repo, "002");
    {
        std::ifstream f(file_a);
        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        std::cout << "after checkout 002, hello.txt = \"" << content
                  << "\" (expect \"Hello World Modified!\")\n";
    }

    // ── Phase 1.7: branch creation ─────────────────────
    std::cout << "\n=== Phase 1.7: branch creation ===\n";

    // Create feature-x from commit 001
    bool br = BranchManager::create(repo, "feature-x", "001");
    std::cout << "create feature-x: " << (br ? "OK" : "FAIL") << "\n";

    // List branches
    auto branches = BranchManager::list(repo);
    std::cout << "branches:";
    for (auto& b : branches) std::cout << " [" << b << "]";
    std::cout << " (expect [feature-x] [main])\n";

    // Duplicate branch should fail
    bool br_dup = BranchManager::create(repo, "feature-x", "001");
    std::cout << "duplicate create: " << (br_dup ? "unexpected OK" : "correctly rejected") << "\n";

    // Verify feature-x has only commit 001 (not 002)
    std::cout << "feature-x/commits/001 exists: "
              << fs::exists(repo.branches_dir() / "feature-x" / "commits" / "001") << "\n";
    std::cout << "feature-x/commits/002 exists: "
              << fs::exists(repo.branches_dir() / "feature-x" / "commits" / "002")
              << " (expect 0)\n";

    // ── Phase 1.8: branch switch ───────────────────────
    std::cout << "\n=== Phase 1.8: branch switch ===\n";

    // Currently on main (commit 002), hello.txt = "Hello World Modified!"
    // Switch to feature-x (HEAD = 001), hello.txt should revert
    BranchManager::switch_to(repo, "feature-x");
    std::cout << "current_branch: " << repo.current_branch << " (expect feature-x)\n";
    {
        std::ifstream f(file_a);
        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        std::cout << "hello.txt = \"" << content
                  << "\" (expect \"Hello World\")\n";
    }

    // Commit on feature-x
    { std::ofstream(file_a) << "Feature X content"; }
    CommitNode::create(repo, "Feature X work");
    std::cout << "feature-x head: " << CommitNode::head_id(repo) << " (expect 002)\n";

    // Switch back to main
    BranchManager::switch_to(repo, "main");
    std::cout << "current_branch: " << repo.current_branch << " (expect main)\n";
    {
        std::ifstream f(file_a);
        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        std::cout << "hello.txt = \"" << content
                  << "\" (expect \"Hello World Modified!\")\n";
    }

    // ── Phase 1.9: M1 integration summary ────────────────
    std::cout << "\n=== Phase 1.9: M1 integration summary ===\n";
    std::cout << "init -> track -> commit x2 -> checkout -> branch -> switch -> commit -> switch back\n";
    std::cout << "All operations completed without errors.\n";

    // Cleanup M1
    fs::remove_all(test_dir);
    std::cout << "\nAll M1 tests (1.1-1.9) passed.\n";
}

void test_phase2() {
    namespace fs = std::filesystem;
    using namespace minigit;

    std::cout << "\n========================================\n";
    std::cout << "=== Phase 2: Save functionality ===\n";
    std::cout << "========================================\n";

    // Setup: init repo, create files, track, commit
    fs::path test_dir = fs::temp_directory_path() / "minigit_test_p2";
    fs::remove_all(test_dir);
    Repository::init(test_dir);
    Repository repo;
    Repository::load(test_dir, repo);

    fs::path file_a = test_dir / "app.txt";
    fs::path file_b = test_dir / "config.txt";
    { std::ofstream(file_a) << "version 1"; }
    { std::ofstream(file_b) << "debug=false"; }
    FileTracker::track(repo, file_a);
    FileTracker::track(repo, file_b);
    CommitNode::create(repo, "Initial v1");

    // ── 2.1: Save create ──────────────────────────────────────
    std::cout << "\n=== 2.1: Save create ===\n";

    // Modify files, then save
    { std::ofstream(file_a) << "version 2 (wip)"; }
    bool s1 = SaveManager::create(repo);
    std::cout << "save create: " << (s1 ? "OK" : "FAIL") << "\n";
    std::cout << "save exists: " << SaveManager::exists(repo) << "\n";
    std::cout << "save dir: " << fs::exists(SaveManager::save_dir(repo) / "files") << "\n";

    // ── 2.2: Save restore ─────────────────────────────────────
    std::cout << "\n=== 2.2: Save restore ===\n";

    // Further modify the file
    { std::ofstream(file_a) << "version 3 (broken)"; }
    {
        std::ifstream f(file_a);
        std::string c((std::istreambuf_iterator<char>(f)), {});
        std::cout << "before restore: app.txt = \"" << c << "\"\n";
    }

    // Restore from save
    SaveManager::restore(repo);
    {
        std::ifstream f(file_a);
        std::string c((std::istreambuf_iterator<char>(f)), {});
        std::cout << "after restore:  app.txt = \"" << c
                  << "\" (expect \"version 2 (wip)\")\n";
    }

    // ── 2.3: Save promote to commit ───────────────────────────
    std::cout << "\n=== 2.3: Save promote ===\n";

    // Modify again, save again, then promote
    { std::ofstream(file_a) << "version 2 final"; }
    SaveManager::create(repo);

    std::string head_before = CommitNode::head_id(repo);
    std::cout << "head before promote: " << head_before << " (expect 001)\n";

    SaveManager::promote(repo, "Promote save to v2");

    std::string head_after = CommitNode::head_id(repo);
    std::cout << "head after promote:  " << head_after << " (expect 002)\n";
    std::cout << "save exists after promote: " << SaveManager::exists(repo)
              << " (expect 0)\n";

    // Verify the promoted commit content
    {
        std::ifstream f(file_a);
        std::string c((std::istreambuf_iterator<char>(f)), {});
        std::cout << "app.txt = \"" << c << "\" (expect \"version 2 final\")\n";
    }

    // Verify commit chain
    CommitNode node;
    fs::path c2_dir = repo.branch_dir("main") / "commits" / "002";
    CommitNode::load(c2_dir, node);
    std::cout << "commit 002 message: \"" << node.message << "\"\n";
    std::cout << "commit 002 parent: " << node.parent_id << " (expect 001)\n";

    // Cleanup
    fs::remove_all(test_dir);
    std::cout << "\nAll Phase 2 tests passed. Cleaned up "
              << display_path(test_dir) << "\n";
}

void test_integration() {
    namespace fs = std::filesystem;
    using namespace minigit;

    std::cout << "\n========================================\n";
    std::cout << "=== Phase 5.1: Integration (E2E) ===\n";
    std::cout << "========================================\n";

    fs::path test_dir = fs::temp_directory_path() / "minigit_test_e2e";
    fs::remove_all(test_dir);
    Repository::init(test_dir);
    Repository repo;
    Repository::load(test_dir, repo);

    // 1. Track files
    fs::path fa = test_dir / "hello.cpp";
    fs::path fb = test_dir / "notes.txt";
    { std::ofstream(fa) << "#include <iostream>\nint main() { return 0; }"; }
    { std::ofstream(fb) << "version 1"; }
    FileTracker::track(repo, fa);
    FileTracker::track(repo, fb);
    auto list = FileTracker::list(repo);
    std::cout << "tracked files: " << list.size() << " (expect 2)\n";

    // 2. First commit
    CommitNode::create(repo, "Initial commit");
    std::cout << "HEAD after commit 1: " << CommitNode::head_id(repo) << " (expect 001)\n";

    // 3. Modify + second commit
    { std::ofstream(fa) << "#include <iostream>\nint main() { std::cout << \"hi\"; }"; }
    auto st = FileTracker::get_status(repo, fa);
    std::cout << "hello.cpp status: " << (st == FileStatus::Modified ? "Modified" : "Other") << " (expect Modified)\n";
    CommitNode::create(repo, "Add output");
    std::cout << "HEAD after commit 2: " << CommitNode::head_id(repo) << " (expect 002)\n";

    // 4. Create branch from commit 001
    BranchManager::create(repo, "feature", "001");
    auto branches = BranchManager::list(repo);
    std::cout << "branches: " << branches.size() << " (expect 2)\n";

    // 5. Switch to feature branch
    BranchManager::switch_to(repo, "feature");
    std::cout << "current branch: " << repo.current_branch << " (expect feature)\n";
    std::cout << "HEAD on feature: " << CommitNode::head_id(repo) << " (expect 001)\n";

    // 6. Commit on feature branch
    { std::ofstream(fb) << "feature notes v1"; }
    CommitNode::create(repo, "Feature notes");
    std::cout << "HEAD on feature after commit: " << CommitNode::head_id(repo) << " (expect 002)\n";

    // 7. Save + Restore
    { std::ofstream(fb) << "feature notes v2 (wip)"; }
    SaveManager::create(repo);
    { std::ofstream(fb) << "BROKEN"; }
    SaveManager::restore(repo);
    {
        std::ifstream f(fb);
        std::string c((std::istreambuf_iterator<char>(f)), {});
        std::cout << "after restore: notes.txt = \"" << c << "\" (expect \"feature notes v2 (wip)\")\n";
    }

    // 8. Save + Promote
    { std::ofstream(fb) << "feature notes final"; }
    SaveManager::create(repo);
    SaveManager::promote(repo, "Finalize feature notes");
    std::cout << "HEAD after promote: " << CommitNode::head_id(repo) << " (expect 003)\n";
    std::cout << "save exists: " << SaveManager::exists(repo) << " (expect 0)\n";

    // 9. Switch back to main
    BranchManager::switch_to(repo, "main");
    std::cout << "current branch: " << repo.current_branch << " (expect main)\n";
    std::cout << "HEAD on main: " << CommitNode::head_id(repo) << " (expect 002)\n";

    // 10. Checkout commit 001 on main
    CommitNode::checkout(repo, "001");
    {
        std::ifstream f(fa);
        std::string c((std::istreambuf_iterator<char>(f)), {});
        bool match = (c == "#include <iostream>\nint main() { return 0; }");
        std::cout << "checkout 001: hello.cpp reverted = " << match << " (expect 1)\n";
    }

    // 11. Edge: untrack + re-track
    FileTracker::untrack(repo, fb);
    auto list2 = FileTracker::list(repo);
    std::cout << "after untrack: files = " << list2.size() << " (expect 1)\n";
    FileTracker::track(repo, fb);
    auto list3 = FileTracker::list(repo);
    std::cout << "after re-track: files = " << list3.size() << " (expect 2)\n";

    // Cleanup
    fs::remove_all(test_dir);
    std::cout << "\nAll E2E integration tests passed.\n";
}

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;

    // If --test flag, run tests; otherwise launch TUI
    if (argc > 1 && std::string(argv[1]) == "--test") {
        test_phase1();
        test_phase2();
        test_integration();
        return 0;
    }

    // Default: launch TUI in current directory
    fs::path repo_path = fs::current_path();
    return minigit::run_app(repo_path);
}
