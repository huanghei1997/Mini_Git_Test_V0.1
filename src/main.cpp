#include "tui/app.h"
#include <filesystem>

int main() {
    std::filesystem::path repo_path = std::filesystem::current_path();
    return minigit::run_app(repo_path);
}
