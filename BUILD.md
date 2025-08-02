# Lyricstator Build Guide

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

### macOS (with Homebrew)
```bash
brew install cmake sdl2 sdl2_mixer sdl2_ttf
```

### Windows (with vcpkg)
```bash
vcpkg install sdl2 sdl2-mixer sdl2-ttf
```

## Building

1. Clone the repository:
```bash
git clone <repository-url>
cd lyricstator
```

2. Create build directory:
```bash
mkdir build && cd build
```

3. Configure with CMake:
```bash
cmake ..
```

4. Build:
```bash
make -j$(nproc)  # Linux/macOS
# or
cmake --build . --config Release  # Windows
```

## Running

```bash
# Basic execution
./Lyricstator

# With files
./Lyricstator song.mp3 song.mid lyrics.lystr

# With samples
./Lyricstator ../samples/sample_song.lystr
```

## Troubleshooting

### SDL2 Not Found
- Ensure SDL2 development packages are installed
- On Windows, make sure vcpkg is in the CMAKE_TOOLCHAIN_FILE

### Audio Issues
- Check that audio devices are available
- Verify ALSA/PulseAudio configuration on Linux
- Try running with `ALSA_PCM_CARD=1` environment variable

### Graphics Issues
- Ensure hardware acceleration is available
- Try setting `SDL_VIDEODRIVER=software` for software rendering

## Development

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Code Style
- Use C++17 features
- Follow existing naming conventions
- Add comments for complex algorithms
- Update documentation for new features

### Testing
```bash
# Test with sample files
./Lyricstator ../samples/demo.lystr

# Test MIDI parsing
./Lyricstator ../samples/test.mid

# Test AI detection (if audio device available)
./Lyricstator --test-ai
```