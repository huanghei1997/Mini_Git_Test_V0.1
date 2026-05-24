#ifndef MINIGIT_CORE_SAVE_H
#define MINIGIT_CORE_SAVE_H

#include <filesystem>
#include <string>

namespace minigit {

struct Repository;

struct SaveManager {
    // 2.1 Create a save (quick checkpoint) on the current branch.
    //     Overwrites any existing save for this branch.
    static bool create(Repository& repo);

    // 2.2 Restore workspace from the current branch's save.
    static bool restore(const Repository& repo);

    // 2.3 Promote the save to a formal commit with the given message.
    //     Deletes the save after promotion.
    static bool promote(Repository& repo, const std::string& message);

    // Check whether a save exists for the current branch.
    static bool exists(const Repository& repo);

    // Get the save directory path for the current branch.
    static std::filesystem::path save_dir(const Repository& repo);
};

} // namespace minigit

#endif // MINIGIT_CORE_SAVE_H
