cmake_minimum_required(VERSION 3.16)
project(RosterFlow VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(rosterflow src/main.cpp src/roster.cpp)
target_include_directories(rosterflow PRIVATE include)
target_compile_options(rosterflow PRIVATE -Wall -Wextra -Wpedantic)

file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

include(CTest)
if(BUILD_TESTING)
    add_executable(rosterflow_tests tests/roster_tests.cpp src/roster.cpp)
    target_include_directories(rosterflow_tests PRIVATE include)
    add_test(NAME rosterflow_tests COMMAND rosterflow_tests)
endif()
