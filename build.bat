@echo off
echo Creating build directory if needed...
if not exist build mkdir build
pushd build
echo Running CMake (this may take a minute, especially first time when downloading dependencies)...
cmake ..
echo Building project...
cmake --build . --config Release
popd
echo ** Build completed. Executables are in build\bin\Release\