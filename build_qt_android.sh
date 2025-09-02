#!/bin/bash

# Lyricstator Qt6 Android Build Script

set -e  # Exit on any error

echo "=== Lyricstator Qt6 Android Build ==="
echo "Building for Android platforms"
echo ""

# Configuration
BUILD_TYPE="${1:-Release}"
ANDROID_ABI="${2:-arm64-v8a}"
BUILD_DIR="build_qt_android_$ANDROID_ABI"
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Build type: $BUILD_TYPE"
echo "Android ABI: $ANDROID_ABI"
echo "Source directory: $SOURCE_DIR"
echo "Build directory: $BUILD_DIR"
echo ""

# Check required environment variables
if [ -z "$ANDROID_SDK_ROOT" ] && [ -z "$ANDROID_HOME" ]; then
    echo "Error: Android SDK not found!"
    echo "Please set ANDROID_SDK_ROOT or ANDROID_HOME environment variable."
    echo "Example: export ANDROID_SDK_ROOT=/path/to/android/sdk"
    exit 1
fi

# Use ANDROID_SDK_ROOT if available, otherwise fall back to ANDROID_HOME
ANDROID_SDK="${ANDROID_SDK_ROOT:-$ANDROID_HOME}"

if [ -z "$ANDROID_NDK_ROOT" ] && [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: Android NDK not found!"
    echo "Please set ANDROID_NDK_ROOT or ANDROID_NDK_HOME environment variable."
    echo "Example: export ANDROID_NDK_ROOT=/path/to/android/ndk"
    exit 1
fi

# Use ANDROID_NDK_ROOT if available, otherwise fall back to ANDROID_NDK_HOME
ANDROID_NDK="${ANDROID_NDK_ROOT:-$ANDROID_NDK_HOME}"

echo "Android SDK: $ANDROID_SDK"
echo "Android NDK: $ANDROID_NDK"

# Check if Qt6 for Android is available
if [ -z "$QT_ANDROID" ]; then
    # Try to find Qt6 Android installation
    QT_ANDROID_CANDIDATES=(
        "$HOME/Qt/6.*/android_*"
        "/opt/Qt/6.*/android_*"
        "/usr/local/Qt/6.*/android_*"
    )
    
    for candidate in "${QT_ANDROID_CANDIDATES[@]}"; do
        if [ -d "$candidate" ]; then
            QT_ANDROID="$candidate"
            break
        fi
    done
    
    if [ -z "$QT_ANDROID" ]; then
        echo "Error: Qt6 for Android not found!"
        echo "Please install Qt6 for Android or set QT_ANDROID environment variable."
        echo "Example: export QT_ANDROID=/path/to/Qt/6.x.x/android_arm64_v8a"
        exit 1
    fi
fi

echo "Qt6 Android: $QT_ANDROID"

# Validate paths
if [ ! -d "$ANDROID_SDK" ]; then
    echo "Error: Android SDK directory not found: $ANDROID_SDK"
    exit 1
fi

if [ ! -d "$ANDROID_NDK" ]; then
    echo "Error: Android NDK directory not found: $ANDROID_NDK"
    exit 1
fi

if [ ! -d "$QT_ANDROID" ]; then
    echo "Error: Qt6 Android directory not found: $QT_ANDROID"
    exit 1
fi

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake not found!"
    echo "Please install CMake (version 3.16 or later)."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo "Found CMake: $CMAKE_VERSION"
echo ""

# Determine Android API level
ANDROID_API_LEVEL="${ANDROID_API_LEVEL:-21}"
echo "Android API Level: $ANDROID_API_LEVEL"

# Determine NDK toolchain
NDK_TOOLCHAIN="$ANDROID_NDK/build/cmake/android.toolchain.cmake"
if [ ! -f "$NDK_TOOLCHAIN" ]; then
    echo "Error: Android NDK CMake toolchain not found: $NDK_TOOLCHAIN"
    exit 1
fi

echo "NDK Toolchain: $NDK_TOOLCHAIN"
echo ""

# Create and enter build directory
echo "Setting up build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake for Android
echo "Configuring with CMake for Android..."
cmake \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_TOOLCHAIN" \
    -DANDROID_ABI="$ANDROID_ABI" \
    -DANDROID_PLATFORM="android-$ANDROID_API_LEVEL" \
    -DANDROID_NDK="$ANDROID_NDK" \
    -DANDROID_SDK="$ANDROID_SDK" \
    -DCMAKE_PREFIX_PATH="$QT_ANDROID" \
    -DCMAKE_FIND_ROOT_PATH="$QT_ANDROID" \
    -DQT_HOST_PATH="$QT_ANDROID/../gcc_64" \
    -DANDROID=ON \
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

# Check if library was created
LIBRARY_NAME="libLyricstator_Qt6_${ANDROID_ABI}.so"
if [ -f "$LIBRARY_NAME" ]; then
    echo "Library created: $PWD/$LIBRARY_NAME"
    
    # Show library info
    if command -v file &> /dev/null; then
        echo "Library info: $(file "$LIBRARY_NAME")"
    fi
    
    if command -v readelf &> /dev/null; then
        echo ""
        echo "Library architecture:"
        readelf -h "$LIBRARY_NAME" | grep Machine
    fi
else
    echo "Warning: Library not found at expected location: $PWD/$LIBRARY_NAME"
fi

# Create APK if Qt Android tools are available
QT_ANDROID_DEPLOY="$QT_ANDROID/bin/androiddeployqt"
if [ -f "$QT_ANDROID_DEPLOY" ]; then
    echo ""
    echo "Creating Android APK..."
    
    # Copy Android project files if they don't exist
    if [ ! -d "android" ]; then
        if [ -d "$SOURCE_DIR/android" ]; then
            cp -r "$SOURCE_DIR/android" .
        else
            echo "Warning: Android project files not found in $SOURCE_DIR/android"
            echo "Creating minimal Android project..."
            mkdir -p android
        fi
    fi
    
    # Run androiddeployqt
    "$QT_ANDROID_DEPLOY" \
        --input android-Lyricstator_Qt6-deployment-settings.json \
        --output android \
        --android-platform android-$ANDROID_API_LEVEL \
        --jdk "$JAVA_HOME" \
        --gradle \
        --release \
        || {
            echo "Warning: APK creation failed, but build was successful"
        }
        
    # Check for APK
    APK_PATH="android/build/outputs/apk/release/android-release.apk"
    if [ -f "$APK_PATH" ]; then
        echo "APK created: $PWD/$APK_PATH"
        
        # Show APK info
        if command -v aapt &> /dev/null; then
            echo "APK info:"
            aapt dump badging "$APK_PATH" | head -5
        fi
    fi
else
    echo ""
    echo "Warning: androiddeployqt not found, skipping APK creation"
    echo "Library can be used in Android Studio projects"
fi

echo ""
echo "=== Android Build Complete ==="
echo "Library location: $PWD/$LIBRARY_NAME"
if [ -f "android/build/outputs/apk/release/android-release.apk" ]; then
    echo "APK location: $PWD/android/build/outputs/apk/release/android-release.apk"
fi
echo ""
echo "Next steps:"
echo "1. Test the library in an Android project"
echo "2. Install APK: adb install $PWD/android/build/outputs/apk/release/android-release.apk"
echo "3. Or integrate library into existing Android project"
echo ""

echo "Android build script completed successfully!"
cd "$SOURCE_DIR"