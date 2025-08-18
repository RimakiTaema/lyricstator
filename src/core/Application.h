#pragma once
#include <SDL.h>
#include "common/Types.h"
#include <memory>
#include <functional>
#include <queue>

// Forward declarations
struct SDL_Window;
struct SDL_Renderer;

namespace tgui {
    class Gui;
}

namespace Lyricstator {

class AudioManager;
class MidiParser;
class NoteDetector;
class LystrParser;
class LystrInterpreter;
class Window;
class TGUIKaraokeDisplay;
class UserInterface;
class TGUIResourcePackGUI;
class TGUISongBrowser; // Added TGUISongBrowser forward declaration
class TGUIEqualizer; // Added TGUIEqualizer forward declaration
class TGUIKeybindEditor; // Added TGUIKeybindEditor forward declaration
class SynchronizationManager;
class FormatExporter;
class ErrorHandler;
class AssetManager;
class SettingsManager; // Added SettingsManager forward declaration

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
    
    void HandleKeyBinding(const std::string& action);
    
private:
    // Core subsystems
    std::unique_ptr<AssetManager> assetManager_;
    std::unique_ptr<AudioManager> audioManager_;
    std::unique_ptr<MidiParser> midiParser_;
    std::unique_ptr<NoteDetector> noteDetector_;
    std::unique_ptr<LystrParser> lystrParser_;
    std::unique_ptr<LystrInterpreter> lystrInterpreter_;
    std::unique_ptr<Window> window_;
    std::unique_ptr<TGUIKaraokeDisplay> karaokeDisplay_;
    std::unique_ptr<UserInterface> userInterface_;
    std::unique_ptr<TGUIResourcePackGUI> resourcePackGUI_;
    std::unique_ptr<TGUISongBrowser> songBrowser_; // Added TGUISongBrowser member
    std::unique_ptr<TGUIEqualizer> equalizer_; // Added TGUIEqualizer member
    std::unique_ptr<TGUIKeybindEditor> keybindEditor_; // Added TGUIKeybindEditor member
    std::unique_ptr<SynchronizationManager> syncManager_;
    std::unique_ptr<FormatExporter> formatExporter_;
    std::unique_ptr<ErrorHandler> errorHandler_;
    
    std::unique_ptr<tgui::Gui> gui_;
    
    SettingsManager* settingsManager_;
    
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
    
    void InitializeSettings();
    void OnSettingsChanged(const std::string& setting);
    void ProcessKeyboardInput(const SDL_Event& event);
    
    // Timing
    TimePoint lastFrameTime_;
    float GetDeltaTime();
};

} // namespace Lyricstator
