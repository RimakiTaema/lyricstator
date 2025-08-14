#pragma once
#include "common/Types.h"
#include <memory>
#include <functional>
#include <queue>

// Forward declarations
struct SDL_Window;
struct SDL_Renderer;

namespace Lyricstator {

class AudioManager;
class MidiParser;
class NoteDetector;
class LystrParser;
class LystrInterpreter;
class Window;
class KaraokeDisplay;
class UserInterface;
class ResourcePackGUI;  // Added ResourcePackGUI forward declaration
class SynchronizationManager;
class FormatExporter;
class ErrorHandler;
class AssetManager;  // Added AssetManager forward declaration

class Application {
public:
    Application();
    ~Application();
    
    // Application lifecycle
    bool Initialize();
    void Run();
    void Shutdown();
    
    // File operations
    bool LoadAudioFile(const std::string& filepath);
    bool LoadMidiFile(const std::string& filepath);
    bool LoadLyricScript(const std::string& filepath);
    
    // Playback control
    void Play();
    void Pause();
    void Stop();
    void Seek(uint32_t timeMs);
    void SetTempo(float multiplier);
    
    // Export functionality
    bool ExportProject(const std::string& filepath, ExportFormat format);
    
    // Event handling
    void PushEvent(const AppEvent& event);
    void ProcessEvents();
    
    // Getters
    PlaybackState GetPlaybackState() const { return playbackState_; }
    uint32_t GetCurrentTimeMs() const;
    
    // Settings
    void SetWindowSize(int width, int height);
    void SetVolume(float volume);
    void SetPitchDetectionEnabled(bool enabled);
    
private:
    // Core subsystems
    std::unique_ptr<AssetManager> assetManager_;  // Added AssetManager
    std::unique_ptr<AudioManager> audioManager_;
    std::unique_ptr<MidiParser> midiParser_;
    std::unique_ptr<NoteDetector> noteDetector_;
    std::unique_ptr<LystrParser> lystrParser_;
    std::unique_ptr<LystrInterpreter> lystrInterpreter_;
    std::unique_ptr<Window> window_;
    std::unique_ptr<KaraokeDisplay> karaokeDisplay_;
    std::unique_ptr<UserInterface> userInterface_;
    std::unique_ptr<ResourcePackGUI> resourcePackGUI_;  // Added ResourcePackGUI
    std::unique_ptr<SynchronizationManager> syncManager_;
    std::unique_ptr<FormatExporter> formatExporter_;
    std::unique_ptr<ErrorHandler> errorHandler_;
    
    // Application state
    bool running_;
    bool initialized_;
    PlaybackState playbackState_;
    std::string currentAudioFile_;
    std::string currentMidiFile_;
    std::string currentLyricScript_;
    
    // Event system
    std::queue<AppEvent> eventQueue_;
    std::function<void(const AppEvent&)> eventCallback_;
    
    // Settings
    int windowWidth_;
    int windowHeight_;
    float volume_;
    float tempoMultiplier_;
    bool pitchDetectionEnabled_;
    
    // Internal methods
    bool InitializeSDL();
    void UpdateSystems(float deltaTime);
    void HandleSDLEvents();
    void OnEvent(const AppEvent& event);
    void ShowErrorDialog(const std::string& message, ErrorType type);
    
    // Timing
    TimePoint lastFrameTime_;
    float GetDeltaTime();
};

} // namespace Lyricstator
