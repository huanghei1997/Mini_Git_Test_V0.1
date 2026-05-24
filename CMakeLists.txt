cmake_minimum_required(VERSION 3.14)
project(mini-git LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Third-party dependencies
# ============================================================

include(FetchContent)

# --- FTXUI ---
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/ftxui/CMakeLists.txt")
    add_subdirectory(third_party/ftxui)
else()
    FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
        GIT_TAG        v5.0.0
    )
    FetchContent_MakeAvailable(ftxui)
endif()

# --- nlohmann/json ---
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nlohmann/json.hpp")
    # Use embedded header
else()
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# ============================================================
# Source files
# ============================================================

set(CORE_SOURCES
    src/core/repository.cpp
    src/core/commit.cpp
    src/core/branch.cpp
    src/core/save.cpp
    src/core/file_tracker.cpp
    src/core/snapshot.cpp
)

set(TUI_SOURCES
    src/tui/app.cpp
    src/tui/tree_view.cpp
    src/tui/file_view.cpp
    src/tui/commit_dialog.cpp
    src/tui/status_bar.cpp
)

set(UTILS_SOURCES
    src/utils/path_utils.cpp
    src/utils/time_utils.cpp
)

# ============================================================
# Executable
# ============================================================

add_executable(mini-git
    src/main.cpp
    ${CORE_SOURCES}
    ${TUI_SOURCES}
    ${UTILS_SOURCES}
)

target_include_directories(mini-git PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# nlohmann/json include path (embedded or fetched)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nlohmann/json.hpp")
    target_include_directories(mini-git PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party
    )
else()
    target_link_libraries(mini-git PRIVATE nlohmann_json::nlohmann_json)
endif()

# FTXUI libraries
target_link_libraries(mini-git PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)
