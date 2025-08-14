#pragma once
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <unordered_map>

namespace Lyricstator {

// Time representation in milliseconds
using TimeMs = std::chrono::milliseconds;
using TimePoint = std::chrono::steady_clock::time_point;

// MIDI note representation
struct MidiNote {
    uint8_t note;           // MIDI note number (0-127)
    uint8_t velocity;       // Note velocity (0-127)
    uint32_t startTime;     // Start time in ticks
    uint32_t duration;      // Duration in ticks
    uint8_t channel;        // MIDI channel (0-15)
};

// Lyric event with timing
struct LyricEvent {
    std::string text;       // Lyric text
    uint32_t startTime;     // Start time in milliseconds
    uint32_t endTime;       // End time in milliseconds
    float pitch;            // Expected pitch (Hz) - optional
    bool highlighted;       // Currently highlighted
};

// Audio format information
struct AudioFormat {
    int sampleRate;         // Sample rate in Hz
    int channels;           // Number of channels
    int bitDepth;           // Bit depth
    std::string format;     // Format string (wav, mp3, ogg)
};

// MIDI tempo event
struct TempoEvent {
    uint32_t tick;          // MIDI tick position
    uint32_t microsecondsPerQuarter;  // Microseconds per quarter note
    double bpm;             // Beats per minute
};

// MIDI time signature
struct TimeSignature {
    uint32_t tick;          // MIDI tick position
    uint8_t numerator;      // Time signature numerator
    uint8_t denominator;    // Time signature denominator (power of 2)
};

// Lystr script command types
enum class LystrCommandType {
    DISPLAY_LYRIC,
    SET_TIMING,
    ANIMATE_TEXT,
    SET_COLOR,
    SET_POSITION,
    FADE_IN,
    FADE_OUT,
    HIGHLIGHT,
    WAIT
};

// Lystr script command
struct LystrCommand {
    LystrCommandType type;
    std::unordered_map<std::string, std::string> parameters;
    uint32_t timestamp;     // Execution time in milliseconds
};

// Color representation
struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

// Screen position
struct Position {
    int x, y;
    
    Position(int px = 0, int py = 0) : x(px), y(py) {}
};

// Rectangle for UI elements
struct Rect {
    int x, y, w, h;
    
    Rect(int px = 0, int py = 0, int width = 0, int height = 0)
        : x(px), y(py), w(width), h(height) {}
};

// AI detection result
struct PitchDetectionResult {
    float frequency;        // Detected frequency in Hz
    float confidence;       // Confidence level (0.0-1.0)
    uint32_t timestamp;     // Timestamp in milliseconds
    bool voiceDetected;     // Whether voice was detected
};

// Playback state
enum class PlaybackState {
    STOPPED,
    PLAYING,
    PAUSED,
    LOADING
};

// Export format types
enum class ExportFormat {
    XMK,
    EXMK,
    LYSTEMK,
    MIDI_WITH_LYRICS
};

// Error types
enum class ErrorType {
    FILE_NOT_FOUND,
    UNSUPPORTED_FORMAT,
    PARSING_ERROR,
    AUDIO_ERROR,
    SDL_ERROR,
    MEMORY_ERROR,
    GENERIC_ERROR
};

// Event types for the application
enum class EventType {
    AUDIO_LOADED,
    MIDI_LOADED,
    LYRIC_SCRIPT_LOADED,
    PLAYBACK_STARTED,
    PLAYBACK_STOPPED,
    PLAYBACK_PAUSED,
    LYRIC_HIGHLIGHT,
    NOTE_DETECTED,
    ERROR_OCCURRED
};

// Application event
struct AppEvent {
    EventType type;
    std::string data;
    void* userData;
    
    AppEvent(EventType t, const std::string& d = "", void* user = nullptr)
        : type(t), data(d), userData(user) {}
};

} // namespace Lyricstator
