#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <SDL.h>

namespace Lyricstator {

struct KeyBinding {
    SDL_Scancode key;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    std::string action;
    std::string description;
};

struct EqualizerBand {
    float frequency;
    float gain;
    bool enabled;
};

struct AudioSettings {
    std::vector<EqualizerBand> equalizerBands;
    float masterVolume = 1.0f;
    int sampleRate = 44100;
    int bufferSize = 1024;
    bool enableEqualizer = true;
    int equalizerBandCount = 12; // Default to 12-band
};

struct DirectorySettings {
    std::vector<std::string> songDirectories;
    std::string defaultExportPath;
    std::string resourcePackPath;
    bool recursiveSearch = true;
    std::vector<std::string> supportedFormats = {"mp3", "wav", "ogg", "flac", "mid", "midi", "lystr"};
};

struct UISettings {
    std::string theme = "default";
    int fontSize = 16;
    bool showSpectrum = true;
    bool showPitchDetection = true;
    bool fullscreen = false;
    int windowWidth = 1280;
    int windowHeight = 720;
};

struct KaraokeSettings {
    bool enablePitchDetection = true;
    float pitchSensitivity = 0.7f;
    bool showNoteIndicators = true;
    bool enableScoring = true;
    float lyricFadeTime = 0.5f;
};

class SettingsManager {
public:
    static SettingsManager& getInstance();
    
    // Settings file management
    bool loadSettings(const std::string& filepath = "");
    bool saveSettings(const std::string& filepath = "") const;
    void resetToDefaults();
    
    // Audio settings
    AudioSettings& getAudioSettings() { return audioSettings_; }
    const AudioSettings& getAudioSettings() const { return audioSettings_; }
    void setEqualizerBandCount(int count);
    void setEqualizerBand(int index, float frequency, float gain);
    void enableEqualizer(bool enable) { audioSettings_.enableEqualizer = enable; }
    
    // Directory settings
    DirectorySettings& getDirectorySettings() { return directorySettings_; }
    const DirectorySettings& getDirectorySettings() const { return directorySettings_; }
    void addSongDirectory(const std::string& path);
    void removeSongDirectory(const std::string& path);
    
    // UI settings
    UISettings& getUISettings() { return uiSettings_; }
    const UISettings& getUISettings() const { return uiSettings_; }
    void setTheme(const std::string& theme) { uiSettings_.theme = theme; }
    
    // Karaoke settings
    KaraokeSettings& getKaraokeSettings() { return karaokeSettings_; }
    const KaraokeSettings& getKaraokeSettings() const { return karaokeSettings_; }
    
    // Keybind management
    std::vector<KeyBinding>& getKeyBindings() { return keyBindings_; }
    const std::vector<KeyBinding>& getKeyBindings() const { return keyBindings_; }
    void setKeyBinding(const std::string& action, SDL_Scancode key, bool ctrl = false, bool shift = false, bool alt = false);
    KeyBinding* findKeyBinding(SDL_Scancode key, bool ctrl, bool shift, bool alt);
    std::string getKeyBindingString(const KeyBinding& binding) const;
    
    // Change notifications
    void setChangeCallback(std::function<void(const std::string&)> callback) { changeCallback_ = callback; }
    
private:
    SettingsManager() = default;
    ~SettingsManager() = default;
    
    AudioSettings audioSettings_;
    DirectorySettings directorySettings_;
    UISettings uiSettings_;
    KaraokeSettings karaokeSettings_;
    std::vector<KeyBinding> keyBindings_;
    
    std::function<void(const std::string&)> changeCallback_;
    std::string settingsFilePath_;
    
    void initializeDefaults();
    void initializeDefaultKeyBindings();
    void initializeDefaultEqualizer(int bandCount);
    void notifyChange(const std::string& setting);
};

} // namespace Lyricstator
