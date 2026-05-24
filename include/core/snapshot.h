#ifndef MINIGIT_CORE_SNAPSHOT_H
#define MINIGIT_CORE_SNAPSHOT_H

#include <filesystem>
#include <vector>
#include <map>
#include <string>

namespace minigit {

struct Snapshot {
    // Copy all listed files into dest_dir/files/ using encoded filenames.
    // Returns a map: encoded_filename -> normalized_original_path
    static std::map<std::string, std::string>
    create(const std::vector<std::filesystem::path>& files,
           const std::filesystem::path& dest_dir);

    // Restore files from snapshot_dir/files/ back to their original paths.
    // file_map: encoded_filename -> original_path (from meta.json)
    static bool restore(const std::filesystem::path& snapshot_dir,
                        const std::map<std::string, std::string>& file_map);
};

} // namespace minigit

#endif // MINIGIT_CORE_SNAPSHOT_H
