#include "utils/path_utils.h"
#include <algorithm>

namespace minigit {

namespace fs = std::filesystem;

std::string normalize_path(const fs::path& p) {
    // generic_string() always returns '/' as separator
    return p.generic_string();
}

std::string encode_path(const fs::path& p) {
    std::string s = normalize_path(p);
    std::replace(s.begin(), s.end(), '/', '_');
    std::replace(s.begin(), s.end(), ':', '_');
    return s;
}

std::string display_path(const fs::path& p) {
    // Returns platform-native format (\ on Windows, / on Linux)
    return p.string();
}

fs::path find_minigit_root(const fs::path& working_dir) {
    // Look for .minigit/ in working_dir and its parents
    fs::path current = fs::absolute(working_dir);
    while (true) {
        fs::path candidate = current / ".minigit";
        if (fs::exists(candidate) && fs::is_directory(candidate)) {
            return current;
        }
        fs::path parent = current.parent_path();
        if (parent == current) {
            // Reached filesystem root, not found
            return fs::path();
        }
        current = parent;
    }
}

} // namespace minigit
