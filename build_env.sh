#!/bin/bash
# Environment configuration script for Lyricstator build

# TGUI Configuration
export TGUI_BACKEND="SDL_RENDERER"  # Options: SDL_RENDERER, SDL_OPENGL3, SDL_TTF_OPENGL3, etc.
export TGUI_VERSION="v1.10.0"       # TGUI version tag

# Build Configuration
export CMAKE_BUILD_TYPE="Release"    # Release or Debug

# Optional: Set custom install prefix
# export CMAKE_INSTALL_PREFIX="/usr/local"

echo "Environment configured for Lyricstator build:"
echo "  TGUI Backend: $TGUI_BACKEND"
echo "  TGUI Version: $TGUI_VERSION"
echo "  Build Type: $CMAKE_BUILD_TYPE"
echo ""
echo "Run: source build_env.sh && mkdir -p build && cd build && cmake .. && make -j\$(nproc)"
