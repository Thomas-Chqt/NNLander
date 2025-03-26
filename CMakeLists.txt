cmake_minimum_required(VERSION 3.10)
project(LanderSimulation)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set optimization flags for all build types
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
endif()

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
        GIT_TAG 5.5
        GIT_SHALLOW ON
    )
    # Set raylib build options
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build raylib as a static library")
    set(RAYLIB_BUILD_MODE Release CACHE STRING "Build mode for raylib")
    FetchContent_MakeAvailable(raylib)
endif()

# Add the common headers directory
include_directories(Common)

# Add subdirectories for each lander demo
add_subdirectory(Lander01)
add_subdirectory(Lander02)
add_subdirectory(Lander03)
add_subdirectory(Lander04)
