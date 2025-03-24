#!/bin/bash
echo "Creating build directory if needed..."
mkdir -p build
pushd build
echo "Running CMake (this may take a minute, especially first time when downloading dependencies)..."
cmake ..
echo "Building project with make..."
make
popd
echo "** Build completed. Executables are in build/bin/"
