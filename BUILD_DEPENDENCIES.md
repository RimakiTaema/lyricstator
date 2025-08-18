# Lyricstator Build Dependencies Guide

## Automatic Dependency Management (Recommended)

The CMakeLists.txt now uses **FetchContent** to automatically download and build all dependencies. Simply run:

\`\`\`bash
mkdir build && cd build
cmake ..
make -j$(nproc)
\`\`\`

## Dependencies Automatically Downloaded

- **SDL2** (2.30.0) - Core multimedia library
- **SDL2_mixer** (2.8.0) - Audio mixing and playback
- **SDL2_ttf** (2.22.0) - TrueType font rendering
- **SDL2_image** (2.8.2) - Image loading support
- **SFML** (2.6.1) - Required for TGUI backend
- **TGUI** (1.2) - Modern GUI framework
- **jsoncpp** (1.9.6) - JSON configuration support

## Alternative: Git Submodules

If you prefer git submodules for development:

\`\`\`bash
# Add dependencies as submodules
git submodule add https://github.com/libsdl-org/SDL.git external/SDL2
git submodule add https://github.com/libsdl-org/SDL_mixer.git external/SDL2_mixer
git submodule add https://github.com/libsdl-org/SDL_ttf.git external/SDL2_ttf
git submodule add https://github.com/libsdl-org/SDL_image.git external/SDL2_image
git submodule add https://github.com/SFML/SFML.git external/SFML
git submodule add https://github.com/texus/TGUI.git external/TGUI
git submodule add https://github.com/open-source-parsers/jsoncpp.git external/jsoncpp

# Initialize and update submodules
git submodule update --init --recursive
\`\`\`

## System Requirements

- **CMake** 3.16 or higher
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **Git** for dependency downloading
- **Internet connection** for first build (dependencies cached afterward)

## Build Time

First build may take 10-15 minutes as dependencies are downloaded and compiled. Subsequent builds are much faster as dependencies are cached.
