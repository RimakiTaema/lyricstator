#ifndef QTAUDIOSETTINGS_H
#define QTAUDIOSETTINGS_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>

#include "core/Application.h"

struct AudioDevice {
    QString name;
    QString id;
    QString type; // "input", "output", "both"
    int sampleRate;
    int channels;
    QString format;
    bool isDefault;
    bool isEnabled;
};

struct AudioFormat {
    QString name;
    QString description;
    int sampleRate;
    int channels;
    QString bitDepth;
    bool isSupported;
};

class QtAudioSettings : public QDialog
{
    Q_OBJECT

public:
    explicit QtAudioSettings(QWidget *parent = nullptr);
    ~QtAudioSettings();

    void setApplication(Lyricstator::Application* app);
    void setAudioDevice(const QString& deviceType, const QString& deviceId);
    QString getAudioDevice(const QString& deviceType) const;
    void setSampleRate(int sampleRate);
    int getSampleRate() const;
    void setChannels(int channels);
    int getChannels() const;
    void setBufferSize(int bufferSize);
    int getBufferSize() const;
    void setLatency(int latency);
    int getLatency() const;
    void setAudioFormat(const QString& format);
    QString getAudioFormat() const;
    void refreshDeviceList();
    void testAudioDevice(const QString& deviceId);
    void resetToDefault();

public slots:
    void show();
    void hide();
    void toggle();

private slots:
    void onCloseClicked();
    void onApplyClicked();
    void onResetClicked();
    void onTestInputClicked();
    void onTestOutputClicked();
    void onInputDeviceChanged();
    void onOutputDeviceChanged();
    void onSampleRateChanged();
    void onChannelsChanged();
    void onBufferSizeChanged();
    void onLatencyChanged();
    void onFormatChanged();
    void onEnableInputChanged();
    void onEnableOutputChanged();
    void onShowAdvancedToggled();
    void updateAnimation();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void createMainPanel();
    void createDeviceSettings();
    void createFormatSettings();
    void createAdvancedSettings();
    void createControlPanel();
    void setupStyling();
    void setupCallbacks();
    void loadAudioSettings();
    void saveAudioSettings();
    void initializeDefaultSettings();
    void scanAudioDevices();
    void updateDeviceComboBoxes();
    void showWithAnimation();
    void hideWithAnimation();
    void applyModernStyling();
    void updateDeviceInfo();

    Lyricstator::Application* application_;
    
    // UI Components
    QWidget* mainPanel_;
    QLabel* titleLabel_;
    QPushButton* closeButton_;
    QPushButton* applyButton_;
    QPushButton* resetButton_;
    QPushButton* testInputButton_;
    QPushButton* testOutputButton_;
    
    // Device settings
    QGroupBox* deviceGroup_;
    QComboBox* inputDeviceCombo_;
    QComboBox* outputDeviceCombo_;
    QCheckBox* enableInputCheck_;
    QCheckBox* enableOutputCheck_;
    QLabel* inputDeviceInfo_;
    QLabel* outputDeviceInfo_;
    
    // Format settings
    QGroupBox* formatGroup_;
    QComboBox* sampleRateCombo_;
    QComboBox* channelsCombo_;
    QComboBox* formatCombo_;
    QLabel* formatInfo_;
    
    // Advanced settings
    QGroupBox* advancedGroup_;
    QCheckBox* showAdvancedCheck_;
    QSlider* bufferSizeSlider_;
    QLabel* bufferSizeLabel_;
    QSlider* latencySlider_;
    QLabel* latencyLabel_;
    QSpinBox* bufferCountSpin_;
    QCheckBox* exclusiveModeCheck_;
    QCheckBox* wasapiExclusiveCheck_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QGridLayout* deviceLayout_;
    QGridLayout* formatLayout_;
    QGridLayout* advancedLayout_;
    
    // Data
    QVector<AudioDevice> audioDevices_;
    QVector<AudioFormat> audioFormats_;
    QString currentInputDevice_;
    QString currentOutputDevice_;
    int currentSampleRate_;
    int currentChannels_;
    QString currentFormat_;
    int currentBufferSize_;
    int currentLatency_;
    bool enableInput_;
    bool enableOutput_;
    bool showAdvanced_;
    
    // Animation
    QPropertyAnimation* slideAnimation_;
    QPropertyAnimation* fadeAnimation_;
    QParallelAnimationGroup* animationGroup_;
    QTimer* animationTimer_;
    float animationTime_;
    bool isVisible_;
    bool isAnimating_;
    
    // Settings
    QSettings* settings_;
    
    // Constants
    static const QVector<int> SUPPORTED_SAMPLE_RATES;
    static const QVector<int> SUPPORTED_CHANNELS;
    static const QStringList SUPPORTED_FORMATS;
    static const int MIN_BUFFER_SIZE = 64;
    static const int MAX_BUFFER_SIZE = 8192;
    static const int MIN_LATENCY = 1;
    static const int MAX_LATENCY = 100;
};

#endif // QTAUDIOSETTINGS_H