#include "core/snapshot.h"
#include "utils/path_utils.h"
#include <iostream>

namespace minigit {

namespace fs = std::filesystem;

std::map<std::string, std::string>
Snapshot::create(const std::vector<fs::path>& files,
                 const fs::path& dest_dir) {
    std::map<std::string, std::string> file_map;
    fs::path files_dir = dest_dir / "files";
    fs::create_directories(files_dir);

    for (auto& src : files) {
        fs::path abs_src = fs::absolute(src);
        if (!fs::exists(abs_src)) {
            std::cerr << "Snapshot skip (missing): "
                      << display_path(abs_src) << "\n";
            continue;
        }

        std::string encoded = encode_path(abs_src);
        fs::path dest = files_dir / encoded;

        try {
            fs::copy_file(abs_src, dest,
                          fs::copy_options::overwrite_existing);
            file_map[encoded] = normalize_path(abs_src);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Snapshot copy failed: " << e.what() << "\n";
        }
    }
    return file_map;
}

bool Snapshot::restore(const fs::path& snapshot_dir,
                       const std::map<std::string, std::string>& file_map) {
    fs::path files_dir = snapshot_dir / "files";
    if (!fs::exists(files_dir)) {
        std::cerr << "Snapshot directory not found: "
                  << display_path(files_dir) << "\n";
        return false;
    }

    bool all_ok = true;
    for (auto& [encoded, original] : file_map) {
        fs::path src = files_dir / encoded;
        fs::path dest = fs::path(original);

        if (!fs::exists(src)) {
            std::cerr << "Snapshot file missing: " << encoded << "\n";
            all_ok = false;
            continue;
        }

        try {
            // Ensure destination directory exists
            fs::create_directories(dest.parent_path());
            fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Restore failed: " << e.what() << "\n";
            all_ok = false;
        }
    }
    return all_ok;
}

} // namespace minigit
