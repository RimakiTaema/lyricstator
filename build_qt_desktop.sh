#!/bin/bash

# Lyricstator Qt6 Desktop Build Script

set -e  # Exit on any error

echo "=== Lyricstator Qt6 Desktop Build ==="
echo "Building for desktop platforms (Linux, Windows, macOS)"
echo ""

# Configuration
BUILD_TYPE="${1:-Release}"
BUILD_DIR="build_qt_desktop"
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Build type: $BUILD_TYPE"
echo "Source directory: $SOURCE_DIR"
echo "Build directory: $BUILD_DIR"
echo ""

# Check if Qt6 is available
if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo "Error: Qt6 not found!"
    echo "Please install Qt6 and make sure qmake is in your PATH."
    echo ""
    echo "Installation options:"
    echo "1. Download Qt6 from: https://www.qt.io/download"
    echo "2. Install via package manager:"
    echo "   - Ubuntu/Debian: sudo apt install qt6-base-dev qt6-multimedia-dev"
    echo "   - Fedora: sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel"
    echo "   - Arch: sudo pacman -S qt6-base qt6-multimedia"
    echo "   - macOS: brew install qt6"
    echo "   - Windows: Use Qt Online Installer"
    exit 1
fi

# Find qmake
QMAKE_CMD=""
if command -v qmake6 &> /dev/null; then
    QMAKE_CMD="qmake6"
elif command -v qmake &> /dev/null; then
    # Check if it's Qt6
    QT_VERSION=$(qmake -query QT_VERSION)
    if [[ $QT_VERSION == 6.* ]]; then
        QMAKE_CMD="qmake"
    else
        echo "Error: Found qmake but it's Qt$QT_VERSION, not Qt6"
        exit 1
    fi
fi

echo "Found Qt6 qmake: $QMAKE_CMD"
QT_VERSION=$($QMAKE_CMD -query QT_VERSION)
QT_INSTALL_PREFIX=$($QMAKE_CMD -query QT_INSTALL_PREFIX)
echo "Qt6 version: $QT_VERSION"
echo "Qt6 install prefix: $QT_INSTALL_PREFIX"
echo ""

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake not found!"
    echo "Please install CMake (version 3.16 or later)."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo "Found CMake: $CMAKE_VERSION"
echo ""

# Create and enter build directory
echo "Setting up build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cmake \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_PREFIX_PATH="$QT_INSTALL_PREFIX" \
    -DCMAKE_INSTALL_PREFIX="../install_qt" \
    -f "$SOURCE_DIR/CMakeLists_Qt6.txt" \
    "$SOURCE_DIR" \
    || {
        echo "Error: CMake configuration failed!"
        exit 1
    }

echo ""
echo "Configuration successful!"
echo ""

# Build
echo "Building..."
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo "Using $NUM_CORES parallel jobs"

cmake --build . --config "$BUILD_TYPE" -j "$NUM_CORES" || {
    echo "Error: Build failed!"
    exit 1
}

echo ""
echo "Build successful!"
echo ""

# Check if binary was created
BINARY_NAME="Lyricstator_Qt6"
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    BINARY_NAME="$BINARY_NAME.exe"
fi

if [ -f "$BINARY_NAME" ]; then
    echo "Binary created: $PWD/$BINARY_NAME"
    
    # Show binary info
    if command -v file &> /dev/null; then
        echo "Binary info: $(file "$BINARY_NAME")"
    fi
    
    if command -v ldd &> /dev/null; then
        echo ""
        echo "Library dependencies:"
        ldd "$BINARY_NAME" | head -10
        echo "... (showing first 10 dependencies)"
    elif command -v otool &> /dev/null; then
        echo ""
        echo "Library dependencies:"
        otool -L "$BINARY_NAME"
    fi
else
    echo "Warning: Binary not found at expected location: $PWD/$BINARY_NAME"
fi

echo ""
echo "=== Build Complete ==="
echo "Binary location: $PWD/$BINARY_NAME"
echo ""
echo "Next steps:"
echo "1. Test the application: ./$BINARY_NAME"
echo "2. Create installer: make installer"
echo "3. Build installer: cd installer && ./build_installer.sh"
echo ""

# Optional: Create installer if requested
if [ "$2" = "installer" ]; then
    echo "Creating installer..."
    make installer || {
        echo "Installer creation failed, but build was successful"
        exit 0
    }
fi

echo "Build script completed successfully!"
cd "$SOURCE_DIR"