#ifndef MINIGIT_TUI_APP_H
#define MINIGIT_TUI_APP_H

#include <filesystem>

namespace minigit {

int run_app(const std::filesystem::path& repo_path);

} // namespace minigit

#endif // MINIGIT_TUI_APP_H
