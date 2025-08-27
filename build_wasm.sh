#!/bin/bash

# OpenClaw WebAssembly Build Script
# This script handles the complete build process including SDL2 shader patching

set -e  # Exit on any error

echo "=== OpenClaw WebAssembly Build Script ==="
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the OpenClaw root directory."
    exit 1
fi

# Check if Emscripten is available
if ! command -v emcmake &> /dev/null; then
    echo "Error: Emscripten not found. Please source the Emscripten environment first:"
    echo "  source ./emsdk/emsdk_env.sh"
    exit 1
fi

echo "1. Setting up build directory..."
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

echo "2. Configuring CMake for Emscripten..."
emcmake cmake -DEmscripten=1 ..

echo "3. Building the project (first attempt to download SDL2)..."
make

echo "4. Patching SDL2 shaders for WebGL compatibility..."
cd ..
./patch_sdl2_shaders.sh

echo "5. Clearing SDL2 build cache and rebuilding..."
rm -rf ./emsdk/upstream/emscripten/cache/build/sdl2
cd build
make

echo ""
echo "=== Build completed successfully! ==="
echo ""
echo "To run the game:"
echo "1. Start a web server: python3 -m http.server 8080"
echo "2. Open: http://localhost:8080/Build_Release/openclaw.html"
echo ""
echo "Note: Make sure you have CLAW.REZ and ASSETS.ZIP in the Build_Release directory."
