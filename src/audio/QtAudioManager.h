#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QAudioInput>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QBuffer>
#include <QTimer>

namespace Lyricstator {

struct QtAudioDevice {
    QString name;
    QString id;
    bool isDefault;
    int sampleRate;
    int channels;
    QStringList supportedFormats;
};

struct QtAudioFormat {
    int sampleRate;
    int channels;
    int sampleSize;
    QString codec;
};

struct QtEqualizerBand {
    float frequency;
    float gain;
    bool enabled;
};

class QtAudioManager : public QObject {
    Q_OBJECT

public:
    static QtAudioManager& getInstance();
    
    // Audio device management
    QVector<QtAudioDevice> getAvailableDevices();
    bool setAudioDevice(const QString& deviceId);
    QtAudioDevice getCurrentDevice() const;
    
    // Audio format management
    bool setAudioFormat(const QtAudioFormat& format);
    QtAudioFormat getCurrentFormat() const;
    QVector<QtAudioFormat> getSupportedFormats(const QString& deviceId);
    
    // Playback control
    bool loadAudioFile(const QString& filepath);
    void play();
    void pause();
    void stop();
    void seek(uint32_t positionMs);
    
    // Volume and effects
    void setVolume(float volume);
    float getVolume() const;
    void setEqualizerBands(const QVector<QtEqualizerBand>& bands);
    void enableEqualizer(bool enabled);
    
    // Recording
    bool startRecording(const QString& filepath);
    void stopRecording();
    bool isRecording() const;
    
    // Status and information
    bool isPlaying() const;
    bool isPaused() const;
    uint32_t getCurrentPosition() const;
    uint32_t getDuration() const;
    QString getCurrentFile() const;
    
    // Audio analysis
    QVector<float> getSpectrumData();
    float getPitch();
    float getTempo();
    
    // Error handling
    QString getLastError() const;
    void clearError();

signals:
    void audioFileLoaded(const QString& filename);
    void playbackStarted();
    void playbackPaused();
    void playbackStopped();
    void playbackPositionChanged(uint32_t position);
    void volumeChanged(float volume);
    void equalizerChanged();
    void recordingStarted();
    void recordingStopped();
    void error(const QString& error);

private:
    QtAudioManager(QObject* parent = nullptr);
    ~QtAudioManager();
    
    // Qt6 audio components
    QMediaPlayer* mediaPlayer_;
    QAudioOutput* audioOutput_;
    QAudioInput* audioInput_;
    QAudioDevice currentDevice_;
    QAudioFormat currentFormat_;
    
    // Audio state
    QString currentFile_;
    float volume_;
    bool equalizerEnabled_;
    QVector<QtEqualizerBand> equalizerBands_;
    bool isRecording_;
    QString recordingFile_;
    
    // Error handling
    QString lastError_;
    
    // Audio processing
    QTimer* spectrumTimer_;
    QVector<float> spectrumData_;
    
    // Private methods
    void initializeAudio();
    void setupAudioFormat();
    void processEqualizer();
    void updateSpectrum();
    void handleMediaPlayerError();
    void handleAudioOutputError();
    
    // Utility methods
    QString formatToString(const QAudioFormat& format);
    QAudioFormat stringToFormat(const QString& formatString);
    bool isValidDevice(const QString& deviceId);
};

// Global instance
#define AudioManager QtAudioManager::getInstance()

} // namespace Lyricstator