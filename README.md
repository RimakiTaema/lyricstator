# 🎤 Lyricstator - Advanced Karaoke System

**Lyricstator** is a sophisticated karaoke and lyric visualization application featuring AI-powered pitch detection, real-time audio synchronization, and beautiful visual effects.

## ✨ Features

### 🎵 Audio & MIDI Support
- **Multi-format audio**: WAV, MP3, OGG support
- **MIDI integration**: Full MIDI file parsing and playback
- **Real-time synchronization**: Perfect timing between audio, MIDI, and lyrics

### 🤖 AI-Powered Features
- **Pitch detection**: Real-time voice analysis and scoring
- **Note recognition**: Advanced algorithms for musical note detection
- **Performance feedback**: Visual indicators for singing accuracy

### 🎨 Beautiful Visuals
- **Animated backgrounds**: Smooth gradient animations
- **Enhanced typography**: Professional text rendering with SDL_ttf
- **Real-time effects**: Glow effects, highlight animations, and visual feedback
- **Customizable themes**: Configurable color schemes and visual styles

### 📝 Advanced Scripting
- **Custom .lystr format**: Powerful scripting language for lyrics
- **Timing control**: Precise control over lyric display timing
- **Animation commands**: Text effects, fades, highlights, and positioning
- **Export capabilities**: Multiple format support (XMK, EXMK, LYSTEMK)

## 🚀 Quick Start

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

**macOS (Homebrew):**
```bash
brew install cmake sdl2 sdl2_mixer sdl2_ttf
```

**Windows (vcpkg):**
```bash
vcpkg install sdl2 sdl2-mixer sdl2-ttf
```

### Building

```bash
# Clone and enter directory
git clone <repository-url>
cd lyricstator

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)  # Linux/macOS
# or: cmake --build . --config Release  # Windows
```

### Running

```bash
# Basic execution
./Lyricstator

# Load files directly
./Lyricstator song.mp3 song.mid lyrics.lystr

# Try samples
./Lyricstator ../samples/demo.lystr
```

## 🎮 Controls

| Key | Action |
|-----|--------|
| `Space` | Play/Pause |
| `Escape` | Stop |
| `Ctrl+Q` | Quit |
| **Drag & Drop** | Load files |

## 📁 Supported Formats

- **Audio**: `.wav`, `.mp3`, `.ogg`
- **MIDI**: `.mid`, `.midi`
- **Lyrics**: `.lystr` (custom format)

## 🎨 Visual Features

### Enhanced Graphics
- **Animated gradient backgrounds** with smooth transitions
- **Professional text rendering** using SDL_ttf
- **Real-time pitch visualization** with smooth animations
- **Glow effects and highlights** for active lyrics
- **Musical note decorations** floating across the screen

### Color Schemes
- **Deep space theme**: Dark blues with gold highlights
- **Customizable colors**: Easy theme modification
- **High contrast**: Optimized for readability

## 🔧 Development

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Code Style
- **Modern C++17** features throughout
- **Modular architecture** with clear separation of concerns
- **Comprehensive error handling** with logging
- **Cross-platform compatibility**

### Testing
```bash
# Test with samples
./Lyricstator ../samples/demo.lystr

# Test AI features (requires audio device)
./Lyricstator --test-ai
```

## 📊 Architecture

```
Lyricstator/
├── src/
│   ├── core/           # Application core and lifecycle
│   ├── audio/          # Audio management and MIDI parsing
│   ├── ai/             # AI-powered note detection
│   ├── gui/            # Enhanced visual interface
│   ├── scripting/      # .lystr format parsing and interpretation
│   ├── sync/           # Synchronization management
│   ├── export/         # Format export capabilities
│   ├── utils/          # Utilities and error handling
│   └── common/         # Shared types and definitions
├── assets/             # Fonts, themes, and resources
├── samples/            # Example .lystr files
└── build/              # Build output directory
```

## 🎯 Recent Enhancements

- ✅ **Enhanced visual rendering** with SDL_ttf integration
- ✅ **Animated backgrounds** and smooth transitions
- ✅ **Improved pitch visualization** with real-time feedback
- ✅ **Professional error handling** with comprehensive logging
- ✅ **Modern CMake configuration** with better organization
- ✅ **Cross-platform font loading** with fallback support

## 🤝 Contributing

We welcome contributions! Please see our contributing guidelines for:
- Code style requirements
- Testing procedures
- Documentation standards
- Feature request process

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🎵 Get Started Today!

Transform your karaoke experience with Lyricstator's advanced features and beautiful visuals. Perfect for:
- **Home karaoke systems**
- **Professional venues**
- **Music education**
- **Performance analysis**

---

*Made with ❤️ for music lovers everywhere*
