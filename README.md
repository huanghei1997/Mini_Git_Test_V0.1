# Mini-Git

A lightweight version control system built with C++17, featuring an interactive terminal UI (TUI).

## Features

- **File Tracking** — Add/remove files to version control
- **Commit Management** — Full-snapshot commits preserving complete file contents
- **Branch Operations** — Create branches, switch between branches
- **Quick Save** — Save/Restore temporary working state
- **Commit Tree View** — Visualize multi-branch commit history
- **Terminal UI** — Fully keyboard-driven interactive interface

## Build

### Prerequisites

- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.14+
- Internet connection (first build fetches FTXUI and nlohmann/json automatically)

### Build Steps

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Windows (MSYS2/MinGW)

```bash
mkdir build && cd build
cmake -G Ninja ..
cmake --build .
```

## Usage

### Launch TUI

```bash
# Run in any project directory
./mini-git
```

The program automatically creates/loads a `.minigit/` repository in the current directory.

### Run Tests

```bash
./mini-git --test
```

## Keyboard Shortcuts

| Key | Action | Context |
|-----|--------|---------|
| `Q` | Quit | Main view |
| `F` | File list | Global |
| `A` | Open file browser (multi-select) | File list |
| `D` | Untrack file / Delete branch | File list / Tree view |
| `C` | Create commit | Global |
| `T` | Commit tree (branch + version management) | Global |
| `B` | Create branch from selected node | Tree view |
| `S` | Quick save | Global |
| `R` | Restore save | Global |
| `P` | Promote save to commit | Global |
| `?` | Help panel | Global |
| `Esc` | Go back | Sub-views |
| `↑↓` | Navigate up/down | Lists / Tree / Browser |
| `←→` | Switch branch column | Tree view |
| `Space` | Toggle file selection | File browser |
| `Enter` | Enter directory / Confirm / Checkout | Browser / Dialog / Tree |
| `Backspace` | Go to parent directory | File browser |

## Project Structure

```
H2/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── core/
│   │   ├── repository.h      # Repository structure & initialization
│   │   ├── file_tracker.h    # File tracking & status detection
│   │   ├── commit.h          # Commit create/load/checkout
│   │   ├── branch.h          # Branch management
│   │   ├── save.h            # Quick save/restore
│   │   └── snapshot.h        # Snapshot engine
│   ├── tui/
│   │   ├── app.h             # TUI entry point
│   │   └── tree_view.h       # Tree data structures
│   └── utils/
│       ├── path_utils.h      # Path normalization/encoding
│       └── time_utils.h      # Timestamp utilities
├── src/
│   ├── main.cpp              # Entry point + integration tests
│   ├── core/                 # Backend implementation
│   └── tui/                  # TUI implementation
└── .gitignore
```

## Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Snapshot strategy | Full copy | Simple implementation, acceptable disk usage for small projects |
| Commit ID | Zero-padded 3-digit number | Simple, intuitive, directory-sort friendly |
| Data format | JSON (nlohmann/json) | Human-readable, easy to debug |
| UI framework | FTXUI | Cross-platform terminal UI, no external dependencies |
| Path handling | `std::filesystem` | C++17 standard, cross-platform |

## License

Academic project for educational purposes only.
