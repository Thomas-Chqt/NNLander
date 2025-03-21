cmake_minimum_required(VERSION 3.10)
project(LanderSimulation)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find raylib package if installed system-wide
find_package(raylib QUIET)

# If system raylib not found, fetch and build it
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Add the common headers directory
include_directories(Common)

# Add subdirectories for each lander demo
add_subdirectory(Lander01)
add_subdirectory(Lander02)
add_subdirectory(Lander03)
add_subdirectory(Lander04)
