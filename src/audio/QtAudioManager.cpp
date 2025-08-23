#include "QtAudioManager.h"
#include <QAudioDevice>
#include <QAudioInput>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QMediaFormat>
#include <QAudioFormat>
#include <QAudioSink>
#include <QAudioSource>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QTimer>

namespace Lyricstator {

QtAudioManager& QtAudioManager::getInstance() {
    static QtAudioManager instance;
    return instance;
}

QtAudioManager::QtAudioManager(QObject* parent)
    : QObject(parent)
    , mediaPlayer_(nullptr)
    , audioOutput_(nullptr)
    , audioInput_(nullptr)
    , volume_(1.0f)
    , equalizerEnabled_(true)
    , isRecording_(false)
    , lastError_()
    , spectrumTimer_(nullptr)
{
    initializeAudio();
    setupAudioFormat();
    
    // Initialize equalizer with default bands
    equalizerBands_.resize(12);
    for (int i = 0; i < 12; ++i) {
        equalizerBands_[i].frequency = 31.25f * (1 << i);
        equalizerBands_[i].gain = 1.0f;
        equalizerBands_[i].enabled = true;
    }
    
    // Initialize spectrum timer
    spectrumTimer_ = new QTimer(this);
    connect(spectrumTimer_, &QTimer::timeout, this, &QtAudioManager::updateSpectrum);
    spectrumTimer_->start(50); // 20 FPS
}

QtAudioManager::~QtAudioManager() {
    if (isRecording_) {
        stopRecording();
    }
    
    if (mediaPlayer_) {
        mediaPlayer_->stop();
        delete mediaPlayer_;
    }
    
    if (audioOutput_) {
        delete audioOutput_;
    }
    
    if (audioInput_) {
        delete audioInput_;
    }
}

QVector<QtAudioDevice> QtAudioManager::getAvailableDevices() {
    QVector<QtAudioDevice> devices;
    
    // Get audio output devices
    const auto outputDevices = QMediaDevices::audioOutputs();
    for (const auto& device : outputDevices) {
        QtAudioDevice qtDevice;
        qtDevice.name = device.description();
        qtDevice.id = device.id().toString();
        qtDevice.isDefault = device.isDefault();
        qtDevice.sampleRate = 44100; // Default
        qtDevice.channels = 2; // Default stereo
        
        // Get supported formats
        const auto formats = device.supportedAudioFormats();
        for (const auto& format : formats) {
            qtDevice.supportedFormats.append(formatToString(format));
        }
        
        devices.append(qtDevice);
    }
    
    return devices;
}

bool QtAudioManager::setAudioDevice(const QString& deviceId) {
    if (!isValidDevice(deviceId)) {
        lastError_ = QString("Invalid audio device: %1").arg(deviceId);
        emit error(lastError_);
        return false;
    }
    
    // Find the device
    const auto devices = QMediaDevices::audioOutputs();
    for (const auto& device : devices) {
        if (device.id().toString() == deviceId) {
            currentDevice_ = device;
            
            // Recreate audio output with new device
            if (audioOutput_) {
                delete audioOutput_;
            }
            audioOutput_ = new QAudioOutput(device, this);
            
            // Recreate media player with new audio output
            if (mediaPlayer_) {
                mediaPlayer_->setAudioOutput(audioOutput_);
            }
            
            qDebug() << "Audio device changed to:" << device.description();
            return true;
        }
    }
    
    lastError_ = QString("Device not found: %1").arg(deviceId);
    emit error(lastError_);
    return false;
}

QtAudioDevice QtAudioManager::getCurrentDevice() const {
    QtAudioDevice device;
    device.name = currentDevice_.description();
    device.id = currentDevice_.id().toString();
    device.isDefault = currentDevice_.isDefault();
    device.sampleRate = currentFormat_.sampleRate();
    device.channels = currentFormat_.channelCount();
    device.supportedFormats.append(formatToString(currentFormat_));
    
    return device;
}

bool QtAudioManager::setAudioFormat(const QtAudioFormat& format) {
    QAudioFormat qtFormat;
    qtFormat.setSampleRate(format.sampleRate);
    qtFormat.setChannelCount(format.channels);
    qtFormat.setSampleFormat(QAudioFormat::Int16); // Default to 16-bit
    
    if (!currentDevice_.isFormatSupported(qtFormat)) {
        lastError_ = QString("Audio format not supported: %1").arg(formatToString(qtFormat));
        emit error(lastError_);
        return false;
    }
    
    currentFormat_ = qtFormat;
    setupAudioFormat();
    
    qDebug() << "Audio format changed to:" << formatToString(currentFormat_);
    return true;
}

QtAudioFormat QtAudioManager::getCurrentFormat() const {
    QtAudioFormat format;
    format.sampleRate = currentFormat_.sampleRate();
    format.channels = currentFormat_.channelCount();
    format.sampleSize = currentFormat_.bytesPerSample() * 8;
    format.codec = "PCM";
    
    return format;
}

QVector<QtAudioFormat> QtAudioManager::getSupportedFormats(const QString& deviceId) {
    QVector<QtAudioFormat> formats;
    
    // Find the device
    const auto devices = QMediaDevices::audioOutputs();
    for (const auto& device : devices) {
        if (device.id().toString() == deviceId) {
            const auto supportedFormats = device.supportedAudioFormats();
            for (const auto& qtFormat : supportedFormats) {
                QtAudioFormat format;
                format.sampleRate = qtFormat.sampleRate();
                format.channels = qtFormat.channelCount();
                format.sampleSize = qtFormat.bytesPerSample() * 8;
                format.codec = "PCM";
                formats.append(format);
            }
            break;
        }
    }
    
    return formats;
}

bool QtAudioManager::loadAudioFile(const QString& filepath) {
    if (!mediaPlayer_) {
        lastError_ = "Media player not initialized";
        emit error(lastError_);
        return false;
    }
    
    QFileInfo fileInfo(filepath);
    if (!fileInfo.exists()) {
        lastError_ = QString("Audio file not found: %1").arg(filepath);
        emit error(lastError_);
        return false;
    }
    
    currentFile_ = filepath;
    mediaPlayer_->setSource(QUrl::fromLocalFile(filepath));
    
    emit audioFileLoaded(filepath);
    qDebug() << "Audio file loaded:" << filepath;
    return true;
}

void QtAudioManager::play() {
    if (mediaPlayer_ && !currentFile_.isEmpty()) {
        mediaPlayer_->play();
        emit playbackStarted();
        qDebug() << "Playback started";
    }
}

void QtAudioManager::pause() {
    if (mediaPlayer_) {
        mediaPlayer_->pause();
        emit playbackPaused();
        qDebug() << "Playback paused";
    }
}

void QtAudioManager::stop() {
    if (mediaPlayer_) {
        mediaPlayer_->stop();
        emit playbackStopped();
        qDebug() << "Playback stopped";
    }
}

void QtAudioManager::seek(uint32_t positionMs) {
    if (mediaPlayer_) {
        mediaPlayer_->setPosition(positionMs);
        emit playbackPositionChanged(positionMs);
    }
}

void QtAudioManager::setVolume(float volume) {
    volume_ = qBound(0.0f, volume, 1.0f);
    
    if (audioOutput_) {
        audioOutput_->setVolume(volume_);
    }
    
    emit volumeChanged(volume_);
    qDebug() << "Volume set to:" << volume_;
}

float QtAudioManager::getVolume() const {
    return volume_;
}

void QtAudioManager::setEqualizerBands(const QVector<QtEqualizerBand>& bands) {
    equalizerBands_ = bands;
    emit equalizerChanged();
    qDebug() << "Equalizer bands updated";
}

void QtAudioManager::enableEqualizer(bool enabled) {
    equalizerEnabled_ = enabled;
    emit equalizerChanged();
    qDebug() << "Equalizer" << (enabled ? "enabled" : "disabled");
}

bool QtAudioManager::startRecording(const QString& filepath) {
    if (isRecording_) {
        stopRecording();
    }
    
    // Create audio input device
    const auto inputDevices = QMediaDevices::audioInputs();
    if (inputDevices.isEmpty()) {
        lastError_ = "No audio input devices available";
        emit error(lastError_);
        return false;
    }
    
    QAudioDevice inputDevice = inputDevices.first();
    if (audioInput_) {
        delete audioInput_;
    }
    
    audioInput_ = new QAudioInput(inputDevice, this);
    recordingFile_ = filepath;
    isRecording_ = true;
    
    emit recordingStarted();
    qDebug() << "Recording started:" << filepath;
    return true;
}

void QtAudioManager::stopRecording() {
    if (isRecording_ && audioInput_) {
        isRecording_ = false;
        recordingFile_.clear();
        
        emit recordingStopped();
        qDebug() << "Recording stopped";
    }
}

bool QtAudioManager::isRecording() const {
    return isRecording_;
}

bool QtAudioManager::isPlaying() const {
    return mediaPlayer_ && mediaPlayer_->playbackState() == QMediaPlayer::PlayingState;
}

bool QtAudioManager::isPaused() const {
    return mediaPlayer_ && mediaPlayer_->playbackState() == QMediaPlayer::PausedState;
}

uint32_t QtAudioManager::getCurrentPosition() const {
    return mediaPlayer_ ? mediaPlayer_->position() : 0;
}

uint32_t QtAudioManager::getDuration() const {
    return mediaPlayer_ ? mediaPlayer_->duration() : 0;
}

QString QtAudioManager::getCurrentFile() const {
    return currentFile_;
}

QVector<float> QtAudioManager::getSpectrumData() {
    return spectrumData_;
}

float QtAudioManager::getPitch() {
    // Placeholder implementation - would need FFT analysis
    return 440.0f; // A4 note
}

float QtAudioManager::getTempo() {
    // Placeholder implementation - would need beat detection
    return 120.0f; // 120 BPM
}

QString QtAudioManager::getLastError() const {
    return lastError_;
}

void QtAudioManager::clearError() {
    lastError_.clear();
}

void QtAudioManager::initializeAudio() {
    // Get default audio output device
    const auto devices = QMediaDevices::audioOutputs();
    if (!devices.isEmpty()) {
        currentDevice_ = devices.first();
    }
    
    // Create audio output
    audioOutput_ = new QAudioOutput(currentDevice_, this);
    
    // Create media player
    mediaPlayer_ = new QMediaPlayer(this);
    mediaPlayer_->setAudioOutput(audioOutput_);
    
    // Connect signals
    connect(mediaPlayer_, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        emit playbackPositionChanged(position);
    });
    
    connect(mediaPlayer_, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString& errorString) {
        lastError_ = errorString;
        emit this->error(errorString);
    });
    
    qDebug() << "Audio system initialized";
}

void QtAudioManager::setupAudioFormat() {
    // Set default format
    currentFormat_.setSampleRate(44100);
    currentFormat_.setChannelCount(2);
    currentFormat_.setSampleFormat(QAudioFormat::Int16);
    
    // Ensure format is supported
    if (!currentDevice_.isFormatSupported(currentFormat_)) {
        // Try to find a supported format
        const auto supportedFormats = currentDevice_.supportedAudioFormats();
        if (!supportedFormats.isEmpty()) {
            currentFormat_ = supportedFormats.first();
        }
    }
}

void QtAudioManager::processEqualizer() {
    if (!equalizerEnabled_) {
        return;
    }
    
    // Placeholder implementation - would apply equalizer effects to audio
    // This would require real-time audio processing
}

void QtAudioManager::updateSpectrum() {
    if (!isPlaying()) {
        spectrumData_.clear();
        return;
    }
    
    // Placeholder implementation - would analyze audio data for spectrum
    // This would require FFT analysis of audio samples
    spectrumData_.resize(64);
    for (int i = 0; i < 64; ++i) {
        spectrumData_[i] = (float)rand() / RAND_MAX * 0.5f; // Random data for now
    }
}

void QtAudioManager::handleMediaPlayerError() {
    if (mediaPlayer_) {
        lastError_ = mediaPlayer_->errorString();
        emit error(lastError_);
    }
}

void QtAudioManager::handleAudioOutputError() {
    if (audioOutput_) {
        lastError_ = "Audio output error";
        emit error(lastError_);
    }
}

QString QtAudioManager::formatToString(const QAudioFormat& format) {
    return QString("%1Hz %2ch %3bit")
        .arg(format.sampleRate())
        .arg(format.channelCount())
        .arg(format.bytesPerSample() * 8);
}

QAudioFormat QtAudioManager::stringToFormat(const QString& formatString) {
    QAudioFormat format;
    
    // Parse format string like "44100Hz 2ch 16bit"
    QRegExp rx("(\\d+)Hz\\s+(\\d+)ch\\s+(\\d+)bit");
    if (rx.indexIn(formatString) != -1) {
        format.setSampleRate(rx.cap(1).toInt());
        format.setChannelCount(rx.cap(2).toInt());
        format.setSampleFormat(QAudioFormat::Int16); // Assume 16-bit
    }
    
    return format;
}

bool QtAudioManager::isValidDevice(const QString& deviceId) {
    const auto devices = QMediaDevices::audioOutputs();
    for (const auto& device : devices) {
        if (device.id().toString() == deviceId) {
            return true;
        }
    }
    return false;
}

} // namespace Lyricstator