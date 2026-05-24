#ifndef MINIGIT_UTILS_PATH_UTILS_H
#define MINIGIT_UTILS_PATH_UTILS_H

#include <filesystem>
#include <string>

namespace minigit {

// Convert any path to POSIX-style '/' separator string
std::string normalize_path(const std::filesystem::path& p);

// Encode a path into a safe filename: '/' -> '_', ':' -> '_'
std::string encode_path(const std::filesystem::path& p);

// Return the platform-native display string (\ on Windows, / on Linux)
std::string display_path(const std::filesystem::path& p);

// Resolve the .minigit directory location from a given working path
std::filesystem::path find_minigit_root(const std::filesystem::path& working_dir);

} // namespace minigit

#endif // MINIGIT_UTILS_PATH_UTILS_H
