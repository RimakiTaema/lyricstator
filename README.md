# Lyricstator v1.0.0

**Karaoke and Lyric Visualization System**

Lyricstator is a complete karaoke and lyric visualization system built in C++ using SDL2. It provides real-time audio/MIDI playback, AI-powered note detection, custom lyric scripting, and beautiful karaoke visualizations.

## Features

### 🎵 Audio & MIDI Support
- **Multi-format Audio**: Load and play WAV, MP3, and OGG audio files
- **MIDI Integration**: Parse MIDI files (.mid) with tempo maps, note events, and lyric meta events
- **Real-time Playback**: Synchronized audio and MIDI playback with precise timing control

### 🤖 AI Note Detection
- **Pitch Detection**: Multiple algorithms (YIN, Autocorrelation, FFT) for voice pitch analysis
- **Voice Activity Detection**: Intelligent detection of singing vs. silence
- **Calibration System**: Train the AI on user's voice for improved accuracy
- **Real-time Processing**: Live pitch feedback during karaoke sessions

### 📝 Lyric Scripting (.lystr)
- **Custom Language**: Powerful scripting language for timed lyrics and animations
- **Rich Commands**: Display text, animations, colors, positioning, and effects
- **Conditional Logic**: Support for if/else statements and loops
- **Timeline Control**: Precise timing control with millisecond accuracy

### 🎨 Visual Interface
- **SDL2 Graphics**: Hardware-accelerated rendering with smooth animations
- **Karaoke Display**: Real-time lyric highlighting and pitch visualization
- **User Controls**: Intuitive interface for playback control and file loading
- **Drag & Drop**: Easy file loading by dragging files onto the window

### 🔄 Synchronization
- **Multi-source Sync**: Coordinate audio, MIDI, and lyrics in perfect timing
- **Seek Support**: Jump to any position while maintaining synchronization
- **Tempo Control**: Adjust playback speed without losing sync

### 📤 Export System
- **Multiple Formats**: Export to XMK, EXMK, LYSTEMK, and MIDI with lyrics
- **Project Packaging**: Combine audio, MIDI, and lyric files into karaoke packages
- **Format Conversion**: Convert between different karaoke file formats

## Installation

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher
- SDL2 development libraries
- SDL2_mixer for audio support
- SDL2_ttf for text rendering

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

### Building
```bash
git clone <repository-url>
cd lyricstator
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage

### Basic Usage
```bash
# Run the application
./Lyricstator

# Load files via command line
./Lyricstator song.mp3 song.mid lyrics.lystr

# The application will open with a GUI window
```

### Controls
- **Space**: Play/Pause
- **Escape**: Stop playback
- **Ctrl+Q**: Quit application
- **Mouse**: Click UI elements for control
- **Drag & Drop**: Drop audio/MIDI/lyric files onto the window

### Supported File Formats

#### Audio Files
- `.wav` - Uncompressed audio
- `.mp3` - MPEG audio (requires SDL2_mixer with MP3 support)
- `.ogg` - Ogg Vorbis audio

#### MIDI Files
- `.mid`, `.midi` - Standard MIDI files (format 0, 1, 2)
- Supports tempo changes, time signatures, and lyric events

#### Lyric Scripts
- `.lystr` - Lyricstator script files (custom format)
- `.txt` - Plain text files (basic support)

## Lyric Scripting Language (.lystr)

The `.lystr` format is a powerful scripting language for creating timed karaoke experiences.

### Basic Commands

```lystr
# Set timing (in milliseconds)
timing(1000);

# Display text with duration
display("Hello World", 2000);

# Set text color (R, G, B, A)
color(255, 255, 255, 255);

# Set text position (X, Y)
position(400, 300);

# Highlight specific text
highlight("Hello");

# Animation effects
fade_in(500);
fade_out(500);
animate("slide_left", 1000);

# Wait for specified time
wait(1000);
```

### Advanced Features

```lystr
# Conditional display based on pitch detection
if (pitch > 440) {
    display("High note!", 1000);
    color(255, 100, 100, 255);
}

# Loop animations
repeat(3) {
    animate("pulse", 500);
    wait(500);
}
```

### Sample Script
See `samples/sample_song.lystr` for a complete example.

## AI Note Detection

### Algorithms
Lyricstator supports multiple pitch detection algorithms:

1. **YIN Algorithm**: High accuracy, good for monophonic vocals
2. **Autocorrelation**: Fast processing, suitable for real-time use
3. **FFT-based**: Frequency domain analysis, good for harmonic content
4. **Hybrid**: Combines multiple algorithms for best results

### Configuration
```cpp
// Set detection algorithm
noteDetector.SetAlgorithm(NoteDetector::Algorithm::YIN);

// Adjust sensitivity (0.0 - 1.0)
noteDetector.SetSensitivity(0.7f);

// Set frequency range for vocal detection
noteDetector.SetFrequencyRange(80.0f, 800.0f);

// Set confidence threshold
noteDetector.SetConfidenceThreshold(0.5f);
```

### Calibration
The AI can be calibrated to individual voices:

1. Start calibration mode
2. Sing along with a reference track
3. Save calibration data for future use
4. Load calibration data for improved accuracy

## Export Formats

### XMK (eXtended Music Karaoke)
Standard karaoke format with embedded audio and lyrics.

### EXMK (Extended XMK)
Enhanced XMK format with additional metadata and effects.

### LYSTEMK (Lyricstator Extended Music Karaoke)
Native Lyricstator format supporting all features including AI detection data.

### MIDI with Lyrics
Standard MIDI file with embedded lyric events.

## Architecture

### Core Components
- **Application**: Main application controller and event system
- **AudioManager**: SDL2-based audio playback and analysis
- **MidiParser**: Complete MIDI file parser with tempo and lyric support
- **NoteDetector**: Multi-algorithm AI pitch detection system
- **LystrParser/Interpreter**: Custom scripting language implementation
- **Window/GUI**: SDL2 graphics and user interface
- **SynchronizationManager**: Timing coordination between all systems
- **FormatExporter**: Multi-format export system

### Plugin Architecture
The note detection system is designed to be extensible:

```cpp
class CustomPitchAlgorithm : public PitchDetectionAlgorithm {
public:
    PitchDetectionResult DetectPitch(const std::vector<float>& audioSamples, int sampleRate) override {
        // Implement custom algorithm
        return result;
    }
};
```

## Development

### Project Structure
```
lyricstator/
├── src/
│   ├── core/           # Application core and main loop
│   ├── audio/          # Audio playback and MIDI parsing
│   ├── ai/             # AI note detection algorithms
│   ├── scripting/      # .lystr language parser/interpreter
│   ├── gui/            # SDL2 graphics and UI
│   ├── sync/           # Synchronization management
│   ├── export/         # Format export system
│   ├── utils/          # Utility functions and helpers
│   └── common/         # Shared types and definitions
├── samples/            # Sample files and test data
├── assets/             # Graphics, fonts, and resources
├── CMakeLists.txt      # Build configuration
└── README.md           # This file
```

### Building with Debug Support
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Running Tests
```bash
# Run with sample files
./Lyricstator ../samples/sample_song.lystr

# Enable verbose logging
./Lyricstator --verbose

# Test AI detection
./Lyricstator --test-ai
```

## Contributing

We welcome contributions! Please see CONTRIBUTING.md for guidelines.

### Areas for Improvement
1. **Advanced AI Models**: Integrate deep learning models for pitch detection
2. **More Export Formats**: Support for additional karaoke formats
3. **Real-time Effects**: Advanced audio processing and effects
4. **Web Interface**: Browser-based control interface
5. **Mobile Support**: Android/iOS companion apps
6. **Cloud Integration**: Online lyric and song databases

## Error Handling

### Common Issues

#### "Failed to initialize SDL"
- Ensure SDL2 development libraries are installed
- Check that your system supports hardware acceleration

#### "Failed to load audio file"
- Verify the audio file format is supported
- Check file permissions and path

#### "MIDI parsing failed"
- Ensure the MIDI file is not corrupted
- Check for unsupported MIDI features (SMPTE timing)

#### "No voice detected"
- Adjust microphone levels
- Lower the sensitivity threshold
- Check audio input device selection

### DAW Export Tips

For best results when exporting from Digital Audio Workstations:

#### Ableton Live
1. Export audio as WAV/AIFF
2. Export MIDI with lyrics as separate track
3. Use "Export MIDI Track" option

#### FL Studio
1. Export as WAV (recommended) or MP3
2. Use "Export MIDI" from the File menu
3. Include lyric events in MIDI export

#### Logic Pro
1. Export audio as WAV or AIFF
2. Use "Export as MIDI File" with lyrics
3. Enable "Include Tempo Information"

#### Pro Tools
1. Export audio as WAV
2. Use "Export MIDI" for timing data
3. Manually create .lystr files for advanced features

## Performance

### System Requirements
- **Minimum**: 2GB RAM, dual-core CPU, integrated graphics
- **Recommended**: 4GB RAM, quad-core CPU, dedicated graphics
- **Audio Latency**: < 50ms for real-time pitch detection
- **Video**: 60 FPS rendering with hardware acceleration

### Optimization Tips
1. Use WAV files for best audio performance
2. Enable hardware-accelerated rendering
3. Adjust buffer sizes for your system
4. Use appropriate pitch detection algorithms for your needs

## License

Lyricstator is released under the MIT License. See LICENSE file for details.

## Acknowledgments

- SDL2 team for the excellent multimedia library
- YIN algorithm researchers for pitch detection
- The open source community for inspiration and support

## Support

For support, bug reports, or feature requests:
- GitHub Issues: [Project Issues Page]
- Email: support@lyricstator.dev
- Discord: [Community Server]

---

**Lyricstator** - Making karaoke more intelligent, one note at a time! 🎤🎵
