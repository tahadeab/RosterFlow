# RosterFlow

**RosterFlow** is a practical command-line roster management system built with modern C++. It demonstrates how core data structures can support a real team-management workflow instead of remaining isolated classroom examples.

## Practical Purpose

RosterFlow helps a coach or team administrator maintain a player roster, search by shirt number, remove records, update selection priorities, identify the next priority player, undo the latest addition, and persist the roster in a portable CSV file.

## Features

| Feature | Practical use |
|---|---|
| Player management | Add, list, search, remove, and update player records |
| Doubly linked list | Preserve roster order and support linked-node operations |
| Priority queue | Select the next player according to priority |
| Action stack | Undo the most recent player addition |
| CSV persistence | Save and reload roster data between sessions |
| Input validation | Prevent duplicate numbers and invalid core values |
| CMake build | Provide a portable build workflow for supported C++ environments |

## Project Structure

```text
RosterFlow/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── include/
│   └── roster.hpp
├── src/
│   ├── main.cpp
│   └── roster.cpp
├── tests/
│   └── roster_tests.cpp
└── data/
    └── players.csv
```

The original generic filenames such as `DrTaqreadFinal.cpp`, `Source1.cpp`, and `Declareation.h` were replaced with descriptive names that communicate each file's responsibility. This makes the project easier to read, assess, maintain, and extend.

## Requirements

The project requires a C++17-compatible compiler and CMake 3.16 or newer.

## Build and Run on Linux or macOS

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/rosterflow
```

## Build on Windows

Open the repository in Visual Studio with CMake support, or run the same CMake commands from PowerShell after installing CMake and a C++ compiler.

## Design Notes

The project uses C++17 and separates public declarations in `include/roster.hpp`, implementation logic in `src/roster.cpp`, and the command-line interface in `src/main.cpp`. Generated binaries and build artifacts are excluded from Git, while the sample CSV file remains versioned so the application starts with usable demonstration data.

## Author and License

**Author:** taha deab

This project is distributed under the [MIT License](LICENSE).
