#pragma once
#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <memory>
#include <functional>
#include <queue>

// Forward declarations
class QtMainWindow;

namespace Lyricstator {

class QtApplication : public QObject {
    Q_OBJECT

public:
    QtApplication(int argc, char* argv[]);
    ~QtApplication();
    
    // Application lifecycle
    bool Initialize();
    int Run();
    void Shutdown();
    
    // File operations
    bool LoadAudioFile(const QString& filepath);
    bool LoadMidiFile(const QString& filepath);
    bool LoadLyricScript(const QString& filepath);
    
    // Playback control
    void Play();
    void Pause();
    void Stop();
    void Seek(uint32_t timeMs);
    void SetTempo(float multiplier);
    
    // Export functionality
    bool ExportProject(const QString& filepath, const QString& format);
    
    // Event handling
    void ProcessEvents();
    
    // Getters
    uint32_t GetCurrentTimeMs() const;
    
    // Settings
    void SetWindowSize(int width, int height);
    void SetVolume(float volume);
    void SetPitchDetectionEnabled(bool enabled);
    
    void HandleKeyBinding(const QString& action);
    
    // Qt-specific
    QApplication* GetQApplication() { return qApp; }
    QtMainWindow* GetMainWindow() { return mainWindow_.get(); }

private:
    // Core subsystems
    std::unique_ptr<QtMainWindow> mainWindow_;
    
    // Application state
    bool running_;
    uint32_t currentTimeMs_;
    float volume_;
    bool pitchDetectionEnabled_;
    
    // Qt application
    QApplication* qApp_;
};

} // namespace Lyricstator