# Lyricstator Qt6 Build Guide

This guide covers building Lyricstator with Qt6 for both desktop and Android platforms, including deployment using cQtdeployer.

## Overview

The project has been migrated to Qt6 while maintaining the existing SDL/TGUI build system. This provides:

- **Modern Qt6 GUI** - Native look and feel on all platforms
- **Cross-platform deployment** - Desktop (Linux, Windows, macOS) and Android
- **cQtdeployer integration** - Automated deployment for PC builds
- **Dual build system** - Qt6 and SDL/TGUI builds coexist

## Prerequisites

### Desktop Prerequisites

#### Qt6 Installation

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-multimedia-dev cmake build-essential
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel cmake gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-multimedia cmake gcc
```

**macOS:**
```bash
brew install qt6 cmake
```

**Windows:**
- Download Qt6 from [Qt Downloads](https://www.qt.io/download)
- Install Qt6 with Qt Creator
- Install Visual Studio 2019+ or MinGW
- Install CMake

#### cQtdeployer (Optional, for deployment)

**Download from releases:**
```bash
# Linux
wget https://github.com/QuasarApp/CQtDeployer/releases/latest/download/cqtdeployer-*.run
chmod +x cqtdeployer-*.run
./cqtdeployer-*.run

# Windows
# Download .exe installer from GitHub releases
```

### Android Prerequisites

#### Android SDK and NDK
```bash
# Set environment variables
export ANDROID_SDK_ROOT=/path/to/android/sdk
export ANDROID_NDK_ROOT=/path/to/android/ndk
export JAVA_HOME=/path/to/java

# Or use Android Studio to install SDK/NDK
```

#### Qt6 for Android
- Install Qt6 with Android support from Qt Online Installer
- Set environment variable:
```bash
export QT_ANDROID=/path/to/Qt/6.x.x/android_arm64_v8a
```

## Building

### Desktop Build

#### Quick Build (Recommended)
```bash
# Make script executable
chmod +x build_qt_desktop.sh

# Build (Release by default)
./build_qt_desktop.sh

# Build Debug version
./build_qt_desktop.sh Debug

# Build and deploy
./build_qt_desktop.sh Release deploy
```

#### Manual Build
```bash
# Create build directory
mkdir build_qt_desktop && cd build_qt_desktop

# Configure
cmake -DCMAKE_BUILD_TYPE=Release -f ../CMakeLists_Qt6.txt ..

# Build
cmake --build . --config Release -j$(nproc)

# Deploy (optional)
make deploy
```

### Android Build

#### Quick Build (Recommended)
```bash
# Make script executable
chmod +x build_qt_android.sh

# Build for arm64-v8a (default)
./build_qt_android.sh

# Build for specific ABI
./build_qt_android.sh Release armeabi-v7a

# Build Debug version
./build_qt_android.sh Debug arm64-v8a
```

#### Manual Build
```bash
# Set environment variables
export ANDROID_SDK_ROOT=/path/to/android/sdk
export ANDROID_NDK_ROOT=/path/to/android/ndk
export QT_ANDROID=/path/to/Qt/6.x.x/android_arm64_v8a

# Create build directory
mkdir build_qt_android && cd build_qt_android

# Configure for Android
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DCMAKE_PREFIX_PATH=$QT_ANDROID \
    -DANDROID=ON \
    -f ../CMakeLists_Qt6.txt \
    ..

# Build
cmake --build . --config Release -j$(nproc)
```

## Project Structure

```
├── src/
│   ├── main_qt.cpp              # Qt6 main entry point
│   ├── main.cpp                 # Original SDL/TGUI entry point
│   ├── gui/
│   │   ├── qt/                  # Qt6 GUI components
│   │   │   ├── QtMainWindow.*
│   │   │   ├── QtKaraokeDisplay.*
│   │   │   ├── QtResourcePackGUI.*
│   │   │   ├── QtSongBrowser.*
│   │   │   ├── QtEqualizer.*
│   │   │   └── QtKeybindEditor.*
│   │   └── ...                  # Original TGUI components
│   └── ...
├── resources/
│   └── resources.qrc            # Qt resource file
├── CMakeLists.txt               # Original SDL/TGUI build
├── CMakeLists_Qt6.txt           # Qt6 build configuration
├── lyricstator.pro              # Qt6 qmake project file
├── cqtdeployer.json             # cQtdeployer configuration
├── deploy_qt.sh.in              # Deployment script template
├── build_qt_desktop.sh          # Desktop build script
├── build_qt_android.sh          # Android build script
└── BUILD_QT6.md                 # This file
```

## Qt6 GUI Features

### Main Components

1. **QtMainWindow** - Main application window with menu bar, toolbar, and status bar
2. **QtKaraokeDisplay** - Real-time karaoke display with pitch visualization
3. **QtResourcePackGUI** - Theme and resource pack manager
4. **QtSongBrowser** - Song library browser with search and filtering
5. **QtEqualizer** - Audio equalizer interface (placeholder)
6. **QtKeybindEditor** - Keyboard shortcut configuration (placeholder)

### Key Features

- **Modern Qt6 styling** - Native platform appearance
- **Drag & drop support** - Load files by dragging onto the window
- **Animated UI elements** - Smooth transitions and effects
- **Resource pack support** - Theming system with JSON configuration
- **Cross-platform compatibility** - Works on Linux, Windows, macOS, and Android

### Controls

- **Menu Bar** - File operations and tools
- **Toolbar** - Quick access buttons
- **Keyboard Shortcuts:**
  - `Space` - Play/Pause
  - `Escape` - Stop
  - `F1` - Resource Pack GUI
  - `F2` - Equalizer
  - `F3` - Keybind Editor
  - `Ctrl+Q` - Quit
  - `Ctrl+O` - Open File
  - `Ctrl+M` - Open MIDI
  - `Ctrl+A` - Open Audio
  - `Ctrl+L` - Open Lyrics

## Deployment

### Desktop Deployment with cQtdeployer

cQtdeployer automatically bundles Qt6 libraries and dependencies for distribution.

#### Using Build Script
```bash
# Build and deploy in one step
./build_qt_desktop.sh Release deploy
```

#### Manual Deployment
```bash
# After building
cd build_qt_desktop
make deploy

# Or run deployment script directly
./deploy_qt.sh
```

#### Custom cQtdeployer Configuration
```bash
# Using configuration file
cqtdeployer -bin ./Lyricstator_Qt6 -confFile ../cqtdeployer.json
```

The deployed application will be in `build_qt_desktop/deploy/` and includes:
- Application binary
- Qt6 libraries
- System libraries
- Assets and samples
- Launcher scripts
- Documentation

### Android Deployment

Android builds create a `.so` library that can be integrated into Android projects.

#### APK Creation (if androiddeployqt is available)
```bash
# APK will be created automatically if tools are available
./build_qt_android.sh

# APK location: build_qt_android_*/android/build/outputs/apk/release/
```

#### Manual Integration
```bash
# Copy the .so file to your Android project
cp build_qt_android_*/libLyricstator_Qt6_*.so /path/to/android/project/app/src/main/jniLibs/
```

## Platform-Specific Notes

### Linux
- Supports all major distributions
- Creates desktop file and installs icon
- Uses system Qt6 or bundled libraries

### Windows
- Supports Windows 10+
- Creates `.exe` with all dependencies
- Optional installer creation with NSIS

### macOS
- Supports macOS 10.15+
- Creates `.app` bundle
- Optional `.dmg` creation

### Android
- Minimum API level 21 (Android 5.0)
- Supports armeabi-v7a and arm64-v8a
- No cQtdeployer needed (uses Qt's Android tools)

## Troubleshooting

### Common Build Issues

**Qt6 not found:**
```bash
# Make sure Qt6 is in PATH
export PATH=/path/to/Qt/6.x.x/gcc_64/bin:$PATH

# Or set CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64
```

**CMake configuration fails:**
```bash
# Check Qt6 installation
qmake6 -query QT_VERSION

# Clear CMake cache
rm -rf build_qt_desktop CMakeCache.txt
```

**Android build fails:**
```bash
# Check environment variables
echo $ANDROID_SDK_ROOT
echo $ANDROID_NDK_ROOT
echo $QT_ANDROID

# Verify NDK toolchain exists
ls $ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake
```

### Deployment Issues

**cQtdeployer not found:**
```bash
# Install cQtdeployer from GitHub releases
# Or use package manager if available
```

**Missing libraries in deployment:**
```bash
# Check library dependencies
ldd ./Lyricstator_Qt6

# Add missing libraries to cqtdeployer.json extraLibs
```

**Android APK creation fails:**
```bash
# Check androiddeployqt exists
ls $QT_ANDROID/bin/androiddeployqt

# Verify JAVA_HOME is set
echo $JAVA_HOME
```

## Comparison: Qt6 vs SDL/TGUI

| Feature | Qt6 Build | SDL/TGUI Build |
|---------|-----------|----------------|
| **GUI Framework** | Native Qt6 | SDL2 + TGUI |
| **Platform Look** | Native | Custom |
| **Deployment** | cQtdeployer | Manual |
| **Android Support** | Yes | Limited |
| **File Size** | Larger (Qt6 libs) | Smaller |
| **Development** | Modern C++/Qt | SDL/OpenGL |
| **Maintenance** | Qt ecosystem | Custom |

## Performance Considerations

### Binary Size
- **Qt6 build**: ~30-50MB (after deployment)
- **SDL build**: ~10-20MB
- Size increase is due to Qt6 libraries

### Runtime Performance
- **Qt6**: Hardware-accelerated rendering
- **SDL**: Direct OpenGL rendering
- Performance is comparable for most use cases

### Memory Usage
- **Qt6**: Higher due to Qt6 runtime
- **SDL**: Lower memory footprint
- Difference is typically 20-30MB

## Contributing

When contributing to the Qt6 implementation:

1. **Maintain compatibility** - Both build systems should work
2. **Follow Qt conventions** - Use Qt naming and patterns
3. **Test on multiple platforms** - Ensure cross-platform compatibility
4. **Update documentation** - Keep this guide current

## Future Enhancements

Planned Qt6 improvements:

- [ ] Complete QtEqualizer implementation
- [ ] Complete QtKeybindEditor implementation
- [ ] QML-based mobile UI for Android
- [ ] Qt6 multimedia integration
- [ ] Advanced audio visualization
- [ ] Plugin system using Qt6
- [ ] Automated CI/CD with Qt6 builds

## License

This Qt6 implementation follows the same license as the main project. See LICENSE file for details.