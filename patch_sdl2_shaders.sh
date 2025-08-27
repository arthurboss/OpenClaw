#!/bin/bash

# Script to patch SDL2 shaders for WebGL compatibility
# This script should be run after CMake configuration but before building

SDL2_SHADER_FILE="./emsdk/upstream/emscripten/cache/ports/sdl2/SDL-release-2.32.8/src/render/opengles2/SDL_shaders_gles2.c"

if [ ! -f "$SDL2_SHADER_FILE" ]; then
    echo "SDL2 shader file not found. SDL2 will be downloaded during the build process."
    echo "The shader patch will be applied automatically when SDL2 is available."
    exit 0
fi

echo "Patching SDL2 shaders for WebGL compatibility..."

# Backup original file
cp "$SDL2_SHADER_FILE" "${SDL2_SHADER_FILE}.backup"

# Patch 1: Disable the problematic extension
sed -i '' 's/#extension GL_OES_EGL_image_external : require/\/\/ #extension GL_OES_EGL_image_external : require  \/\/ Disabled for WebGL compatibility/g' "$SDL2_SHADER_FILE"

# Patch 2: Replace samplerExternalOES with sampler2D
sed -i '' 's/samplerExternalOES/sampler2D/g' "$SDL2_SHADER_FILE"

echo "SDL2 shaders patched successfully!"
echo "Backup saved as ${SDL2_SHADER_FILE}.backup"
echo ""
echo "Now run: rm -rf ./emsdk/upstream/emscripten/cache/build/sdl2 && make"
