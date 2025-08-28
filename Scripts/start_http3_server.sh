#!/bin/bash

# OpenClaw HTTP/3 Development Server Script
# This script sets up and starts a Caddy server with HTTP/3 support

set -e  # Exit on any error

echo "=== OpenClaw HTTP/3 Development Server ==="
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the OpenClaw root directory."
    exit 1
fi

# Check if Build_Release directory exists
if [ ! -d "Build_Release" ]; then
    echo "Error: Build_Release directory not found. Please build the project first:"
    echo "  source ./emsdk/emsdk_env.sh && ./build_wasm.sh"
    exit 1
fi

# Check if Caddyfile exists
if [ ! -f "Caddyfile" ]; then
    echo "Error: Caddyfile not found. Please ensure the Caddyfile is in the project root."
    exit 1
fi

# Function to install Caddy
install_caddy() {
    echo "Installing Caddy..."
    
    if command -v brew &> /dev/null; then
        # macOS
        echo "Installing Caddy via Homebrew..."
        brew install caddy
    elif command -v apt-get &> /dev/null; then
        # Ubuntu/Debian
        echo "Installing Caddy via apt..."
        sudo apt-get update
        sudo apt-get install -y debian-keyring debian-archive-keyring apt-transport-https curl
        curl -1sLf 'https://dl.cloudsmith.io/public/caddy/stable/gpg.key' | sudo gpg --dearmor -o /usr/share/keyrings/caddy-stable-archive-keyring.gpg
        curl -1sLf 'https://dl.cloudsmith.io/public/caddy/stable/debian.deb.txt' | sudo tee /etc/apt/sources.list.d/caddy-stable.list
        sudo apt-get update
        sudo apt-get install caddy
    else
        echo "Error: Could not install Caddy automatically."
        echo "Please install Caddy manually: https://caddyserver.com/docs/install"
        exit 1
    fi
}

# Check if our custom Caddy binary exists
if [ -f "./caddy" ]; then
    echo "Using custom Caddy binary with Brotli support"
    CADDY_CMD="./caddy"
elif command -v caddy &> /dev/null; then
    echo "Using system Caddy (Brotli support may not be available)"
    CADDY_CMD="caddy"
else
    echo "Caddy not found. Installing..."
    install_caddy
    CADDY_CMD="caddy"
fi

echo "Caddy version: $($CADDY_CMD version)"
echo ""

# Check if the game files exist
echo "Checking game files..."
if [ ! -f "Build_Release/openclaw.html" ]; then
    echo "Warning: openclaw.html not found in Build_Release/"
fi

if [ ! -f "Build_Release/openclaw.wasm" ]; then
    echo "Warning: openclaw.wasm not found in Build_Release/"
fi

if [ ! -f "Build_Release/openclaw.js" ]; then
    echo "Warning: openclaw.js not found in Build_Release/"
fi

echo ""

# Start the server
echo "Starting HTTP/3 development server..."
echo "Server will be available at: https://localhost:8080"
echo "Game URL: https://localhost:8080/openclaw.html"
echo ""
echo "Features enabled:"
echo "  ✅ HTTP/3 (QUIC protocol)"
echo "  ✅ HTTP/2 fallback"
echo "  ✅ HTTP/1.1 fallback"
echo "  ✅ Brotli + Zstd + Gzip compression"
echo "  ✅ Automatic HTTPS"
echo "  ✅ Asset caching"
echo "  ✅ CORS support"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

# Start Caddy with the configuration
$CADDY_CMD run --config Caddyfile
