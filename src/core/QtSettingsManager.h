#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QKeySequence>
#include <functional>

namespace Lyricstator {

struct QtKeyBinding {
    QKeySequence key;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    QString action;
    QString description;
};

struct QtEqualizerBand {
    float frequency;
    float gain;
    bool enabled;
};

struct QtAudioSettings {
    QVector<QtEqualizerBand> equalizerBands;
    float masterVolume = 1.0f;
    int sampleRate = 44100;
    int bufferSize = 1024;
    bool enableEqualizer = true;
    int equalizerBandCount = 12; // Default to 12-band
};

struct QtDirectorySettings {
    QStringList songDirectories;
    QString defaultExportPath;
    QString resourcePackPath;
    bool recursiveSearch = true;
    QStringList supportedFormats = {"mp3", "wav", "ogg", "flac", "mid", "midi", "lystr"};
};

struct QtUISettings {
    QString theme = "default";
    int fontSize = 16;
    bool showSpectrum = true;
    bool showPitchDetection = true;
    bool fullscreen = false;
    int windowWidth = 1280;
    int windowHeight = 720;
};

struct QtKaraokeSettings {
    bool enablePitchDetection = true;
    float pitchSensitivity = 0.7f;
    bool showNoteIndicators = true;
    bool enableScoring = true;
    float lyricFadeTime = 0.5f;
};

class QtSettingsManager : public QObject {
    Q_OBJECT

public:
    static QtSettingsManager& getInstance();
    
    // Settings file management
    bool loadSettings(const QString& filepath = "");
    bool saveSettings(const QString& filepath = "") const;
    void resetToDefaults();
    
    // Audio settings
    QtAudioSettings& getAudioSettings() { return audioSettings_; }
    const QtAudioSettings& getAudioSettings() const { return audioSettings_; }
    void setEqualizerBandCount(int count);
    void setEqualizerBand(int index, float frequency, float gain);
    void enableEqualizer(bool enable) { audioSettings_.enableEqualizer = enable; }
    
    // Directory settings
    QtDirectorySettings& getDirectorySettings() { return directorySettings_; }
    const QtDirectorySettings& getDirectorySettings() const { return directorySettings_; }
    void addSongDirectory(const QString& path);
    void removeSongDirectory(const QString& path);
    
    // UI settings
    QtUISettings& getUISettings() { return uiSettings_; }
    const QtUISettings& getUISettings() const { return uiSettings_; }
    void setTheme(const QString& theme) { uiSettings_.theme = theme; }
    
    // Karaoke settings
    QtKaraokeSettings& getKaraokeSettings() { return karaokeSettings_; }
    const QtKaraokeSettings& getKaraokeSettings() const { return karaokeSettings_; }
    
    // Keybind management
    QVector<QtKeyBinding>& getKeyBindings() { return keyBindings_; }
    const QVector<QtKeyBinding>& getKeyBindings() const { return keyBindings_; }
    void setKeyBinding(const QString& action, const QKeySequence& key, bool ctrl = false, bool shift = false, bool alt = false);
    QtKeyBinding* findKeyBinding(const QKeySequence& key, bool ctrl, bool shift, bool alt);
    QString getKeyBindingString(const QtKeyBinding& binding) const;
    
    // Change notifications
    void setChangeCallback(std::function<void(const QString&)> callback) { changeCallback_ = callback; }

signals:
    void settingsChanged(const QString& section);
    void audioSettingsChanged();
    void uiSettingsChanged();
    void keyBindingsChanged();

private:
    QtSettingsManager(QObject* parent = nullptr);
    ~QtSettingsManager();
    
    QtAudioSettings audioSettings_;
    QtDirectorySettings directorySettings_;
    QtUISettings uiSettings_;
    QtKaraokeSettings karaokeSettings_;
    QVector<QtKeyBinding> keyBindings_;
    
    std::function<void(const QString&)> changeCallback_;
    
    void initializeDefaults();
    void loadKeyBindings();
    void saveKeyBindings();
    void emitChangeSignal(const QString& section);
};

// Global instance
#define SettingsManager QtSettingsManager::getInstance()

} // namespace Lyricstator