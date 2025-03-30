# NNLander - Minimal Neural Networks for Control Systems

![NNLander Screenshot](screenshot.png)

Neural Networks applied to a simple 2D lunar lander simulation game.

**[日本語版はこちらをご覧ください](README_ja.md)** | **[View in Japanese](README_ja.md)**

## Overview

This workshop provides a hands-on introduction to application of neural networks to control systems,
starting with the concept of control systems, simulation environment, input/output of neural networks,

## Workshop Stages

See [slides/slides.pdf](slides/slides.pdf) for the workshop slides.

### Prerequisites

- CMake (3.10 or higher)
- C++ compiler (supporting C++20)
- Git (for fetching Raylib)

For system setup see:
- [Workshop Requirements EN](workshop_requirements_nn_en.txt) (English)
- [Workshop Requirements JA](workshop_requirements_nn_ja.txt) （日本語）

### Building the executables

On Linux/macOS:
```bash
./build.sh
```

On Windows:
```bash
./build.bat
```

### Running the executables

On Linux/macOS:
```bash
./build/bin/lander01
./build/bin/lander02
./build/bin/lander03
./build/bin/lander04
./build/bin/lander05
```

On Windows:
```bash
.\build\bin\Release\lander01.exe
.\build\bin\Release\lander02.exe
.\build\bin\Release\lander03.exe
.\build\bin\Release\lander04.exe
.\build\bin\Release\lander05.exe
```

#### Using Visual Studio

1. Open the .sln file found in the `build` folder
2. Select the project (e.g.`lander01`) you want to run from the drop-down menu
3. Select the `Release` configuration
4. Press F5 to build and run

## Project Structure

```
NNLander/
├── README.md                     # Main documentation
├── README_ja.md                  # Japanese documentation
├── workshop_requirements_nn_en.txt  # Requirements (English)
├── workshop_requirements_nn_ja.txt  # Requirements (Japanese)
├── CMakeLists.txt                # Main CMake configuration
├── build.sh                      # Build script for Linux/macOS
├── build.bat                     # Build script for Windows
├── Common/                       # Shared code for all landers
│   ├── Simulation.h              # Physics and game simulation
│   ├── SimulationDisplay.h       # Rendering functionality
│   ├── SimpleNeuralNet.h         # Neural network implementation
│   ├── DrawUI.h                  # UI rendering components
│   └── Utils.h                   # Utility functions
├── Lander01/                     # Manual control implementation
│   ├── lander01.cpp              # Main program
│   └── CMakeLists.txt            # Build configuration
├── Lander02/                     # Basic neural network implementation
│   ├── lander02.cpp              # Main program
│   └── CMakeLists.txt            # Build configuration
├── Lander03/                     # Improved neural network implementation
│   ├── lander03.cpp              # Main program
│   ├── TrainingTaskRandom.h      # Random training task
│   └── CMakeLists.txt            # Build configuration
├── Lander04/                     # Advanced neural network implementation
│   ├── lander04.cpp              # Main program
│   ├── TrainingTaskGA.h          # Genetic Algorithm training task
│   └── CMakeLists.txt            # Build configuration
├── Lander05/                     # Reinforcement Learning neural network application
│   ├── lander05.cpp              # Main program
│   ├── TrainingTaskRES.h         # REINFORCE-ES training task
│   └── CMakeLists.txt            # Build configuration
├── slides/                       # Workshop presentation materials
└── build/                        # Build output directory
```

## Game Controls (for Lander01)

- **Up Arrow**: Apply thrust
- **Left Arrow**: Apply left thrust
- **Right Arrow**: Apply right thrust
- **Space**: Restart game after landing or crashing

## Game Objectives

- Land safely on the green landing pad
- Control your descent speed (should be less than 1.5 units)
- Manage your fuel consumption

## Contact

*Davide Pasca*:
- [davide@newtypekk.com](mailto:davide@newtypekk.com)
- [github.com/dpasca](https://github.com/dpasca)
- [newtypekk.com](https://newtypekk.com)
- [x.com/109mae](https://x.com/109mae)
