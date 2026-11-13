cmake_minimum_required(VERSION 3.16)
project(RosterFlow VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Core library: domain logic, persistence, configuration, logging.
# The executable and the test suite both link against it (single source of
# truth for business rules — no duplicated compilation drift).
# ---------------------------------------------------------------------------
add_library(rosterflow_core STATIC
    src/roster.cpp
    src/config.cpp
    src/logger.cpp
)
target_include_directories(rosterflow_core PUBLIC include)
target_compile_options(rosterflow_core PRIVATE -Wall -Wextra -Wpedantic)

find_package(Threads QUIET)
if(Threads_FOUND)
    target_link_libraries(rosterflow_core PUBLIC Threads::Threads)
endif()

# ---------------------------------------------------------------------------
# Command-line application
# ---------------------------------------------------------------------------
add_executable(rosterflow src/main.cpp)
target_link_libraries(rosterflow PRIVATE rosterflow_core)
target_compile_options(rosterflow PRIVATE -Wall -Wextra -Wpedantic)

# Start-up data lives next to the binary so the app works from the build dir.
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    add_executable(rosterflow_tests tests/roster_tests.cpp)
    target_link_libraries(rosterflow_tests PRIVATE rosterflow_core)
    target_compile_options(rosterflow_tests PRIVATE -Wall -Wextra)
    add_test(NAME rosterflow_tests COMMAND rosterflow_tests)
endif()
