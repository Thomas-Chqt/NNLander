# Lander Simulation

A simple 2D lunar lander simulation game built with C++ and Raylib.

## Game Controls

- **Up Arrow**: Apply thrust
- **Left Arrow**: Rotate counter-clockwise
- **Right Arrow**: Rotate clockwise
- **Space**: Restart game after landing or crashing

## Game Objectives

- Land safely on the green landing pad
- Control your descent speed (should be less than 1.5 units)
- Keep your lander level (angle less than 10 degrees)
- Manage your fuel consumption

## Building the Project

This project uses CMake as its build system, which makes it platform-independent.

### Prerequisites

- CMake (3.10 or higher)
- C++ compiler (supporting C++11)
- Git (for fetching Raylib)

### Building on Linux/macOS

```bash
# Create a build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Run the game
./bin/lander_sim
```

### Building on Windows

#### Using Command Line

```bash
# Create a build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . --config Release

# Run the game
.\bin\Release\lander_sim.exe
```

#### Using Visual Studio

1. Open the project folder in Visual Studio with "Open Folder"
2. Visual Studio should detect the CMakeLists.txt and configure the project
3. Select the "lander01.exe" target from the drop-down menu
4. Press F5 to build and run

## Project Structure

- `CMakeLists.txt` - CMake build configuration
- `Lander*` - Each folder contains an evolution of the demo

## Notes

The CMake configuration will automatically download and build Raylib if it's not found on your system, making it easy to get started without manual Raylib installation.
