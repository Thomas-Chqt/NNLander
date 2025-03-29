#!/bin/bash
# Default to Release if no build type specified
BUILD_TYPE=${1:-Release}

echo "Creating build directory if needed..."
mkdir -p build
pushd build
echo "Running CMake with build type: $BUILD_TYPE"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
echo "Building project with make..."
make
popd
echo "** Build completed. Executables are in build/bin/"
