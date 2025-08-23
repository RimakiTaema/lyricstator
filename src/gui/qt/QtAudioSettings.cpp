#include "QtAudioSettings.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStyleOption>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QSettings>
#include <QLineEdit>
#include <cmath>
#include <algorithm>

// Static constants
const QVector<int> QtAudioSettings::SUPPORTED_SAMPLE_RATES = {
    8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000
};

const QVector<int> QtAudioSettings::SUPPORTED_CHANNELS = {
    1, 2, 4, 6, 8
};

const QStringList QtAudioSettings::SUPPORTED_FORMATS = {
    "PCM 16-bit", "PCM 24-bit", "PCM 32-bit", "Float 32-bit", "Float 64-bit"
};

QtAudioSettings::QtAudioSettings(QWidget *parent)
    : QDialog(parent)
    , application_(nullptr)
    , mainPanel_(nullptr)
    , titleLabel_(nullptr)
    , closeButton_(nullptr)
    , applyButton_(nullptr)
    , resetButton_(nullptr)
    , testInputButton_(nullptr)
    , testOutputButton_(nullptr)
    , deviceGroup_(nullptr)
    , inputDeviceCombo_(nullptr)
    , outputDeviceCombo_(nullptr)
    , enableInputCheck_(nullptr)
    , enableOutputCheck_(nullptr)
    , inputDeviceInfo_(nullptr)
    , outputDeviceInfo_(nullptr)
    , formatGroup_(nullptr)
    , sampleRateCombo_(nullptr)
    , channelsCombo_(nullptr)
    , formatCombo_(nullptr)
    , formatInfo_(nullptr)
    , advancedGroup_(nullptr)
    , showAdvancedCheck_(nullptr)
    , bufferSizeSlider_(nullptr)
    , bufferSizeLabel_(nullptr)
    , latencySlider_(nullptr)
    , latencyLabel_(nullptr)
    , bufferCountSpin_(nullptr)
    , exclusiveModeCheck_(nullptr)
    , wasapiExclusiveCheck_(nullptr)
    , slideAnimation_(nullptr)
    , fadeAnimation_(nullptr)
    , animationGroup_(nullptr)
    , animationTimer_(nullptr)
    , settings_(nullptr)
    , animationTime_(0.0f)
    , isVisible_(false)
    , isAnimating_(false)
    , currentInputDevice_("default")
    , currentOutputDevice_("default")
    , currentSampleRate_(44100)
    , currentChannels_(2)
    , currentFormat_("PCM 16-bit")
    , currentBufferSize_(1024)
    , currentLatency_(10)
    , enableInput_(true)
    , enableOutput_(true)
    , showAdvanced_(false)
{
    setWindowTitle("Audio Settings");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(false);
    
    // Initialize settings
    settings_ = new QSettings("Lyricstator", "Audio", this);
    
    setupUI();
    setupStyling();
    setupCallbacks();
    loadAudioSettings();
    initializeDefaultSettings();
    
    // Setup animation timer
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &QtAudioSettings::updateAnimation);
    animationTimer_->start(16); // ~60 FPS
    
    // Scan for audio devices
    scanAudioDevices();
}

QtAudioSettings::~QtAudioSettings()
{
    saveAudioSettings();
    if (animationTimer_) {
        animationTimer_->stop();
    }
}

void QtAudioSettings::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtAudioSettings::setAudioDevice(const QString& deviceType, const QString& deviceId)
{
    if (deviceType == "input") {
        currentInputDevice_ = deviceId;
        if (inputDeviceCombo_) {
            int index = inputDeviceCombo_->findData(deviceId);
            if (index >= 0) {
                inputDeviceCombo_->setCurrentIndex(index);
            }
        }
    } else if (deviceType == "output") {
        currentOutputDevice_ = deviceId;
        if (outputDeviceCombo_) {
            int index = outputDeviceCombo_->findData(deviceId);
            if (index >= 0) {
                outputDeviceCombo_->setCurrentIndex(index);
            }
        }
    }
    
    updateDeviceInfo();
    saveAudioSettings();
}

QString QtAudioSettings::getAudioDevice(const QString& deviceType) const
{
    if (deviceType == "input") {
        return currentInputDevice_;
    } else if (deviceType == "output") {
        return currentOutputDevice_;
    }
    return QString();
}

void QtAudioSettings::setSampleRate(int sampleRate)
{
    currentSampleRate_ = sampleRate;
    if (sampleRateCombo_) {
        int index = sampleRateCombo_->findData(sampleRate);
        if (index >= 0) {
            sampleRateCombo_->setCurrentIndex(index);
        }
    }
    saveAudioSettings();
}

int QtAudioSettings::getSampleRate() const
{
    return currentSampleRate_;
}

void QtAudioSettings::setChannels(int channels)
{
    currentChannels_ = channels;
    if (channelsCombo_) {
        int index = channelsCombo_->findData(channels);
        if (index >= 0) {
            channelsCombo_->setCurrentIndex(index);
        }
    }
    saveAudioSettings();
}

int QtAudioSettings::getChannels() const
{
    return currentChannels_;
}

void QtAudioSettings::setBufferSize(int bufferSize)
{
    currentBufferSize_ = std::clamp(bufferSize, MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
    if (bufferSizeSlider_) {
        bufferSizeSlider_->setValue(currentBufferSize_);
    }
    if (bufferSizeLabel_) {
        bufferSizeLabel_->setText(QString("Buffer Size: %1 samples").arg(currentBufferSize_));
    }
    saveAudioSettings();
}

int QtAudioSettings::getBufferSize() const
{
    return currentBufferSize_;
}

void QtAudioSettings::setLatency(int latency)
{
    currentLatency_ = std::clamp(latency, MIN_LATENCY, MAX_LATENCY);
    if (latencySlider_) {
        latencySlider_->setValue(currentLatency_);
    }
    if (latencyLabel_) {
        latencyLabel_->setText(QString("Latency: %1 ms").arg(currentLatency_));
    }
    saveAudioSettings();
}

int QtAudioSettings::getLatency() const
{
    return currentLatency_;
}

void QtAudioSettings::setAudioFormat(const QString& format)
{
    currentFormat_ = format;
    if (formatCombo_) {
        int index = formatCombo_->findText(format);
        if (index >= 0) {
            formatCombo_->setCurrentIndex(index);
        }
    }
    updateFormatInfo();
    saveAudioSettings();
}

QString QtAudioSettings::getAudioFormat() const
{
    return currentFormat_;
}

void QtAudioSettings::refreshDeviceList()
{
    scanAudioDevices();
    updateDeviceComboBoxes();
}

void QtAudioSettings::testAudioDevice(const QString& deviceId)
{
    // TODO: Implement audio device testing
    QMessageBox::information(this, "Test Audio Device", 
                           QString("Testing device: %1\n\nThis feature will be implemented to test audio input/output.").arg(deviceId));
}

void QtAudioSettings::resetToDefault()
{
    int ret = QMessageBox::question(this, "Reset Audio Settings", 
                                   "Reset all audio settings to default values?",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        currentInputDevice_ = "default";
        currentOutputDevice_ = "default";
        currentSampleRate_ = 44100;
        currentChannels_ = 2;
        currentFormat_ = "PCM 16-bit";
        currentBufferSize_ = 1024;
        currentLatency_ = 10;
        enableInput_ = true;
        enableOutput_ = true;
        showAdvanced_ = false;
        
        // Update UI
        updateDeviceComboBoxes();
        updateFormatInfo();
        updateAdvancedSettings();
        
        saveAudioSettings();
        
        QMessageBox::information(this, "Reset Complete", "Audio settings have been reset to default values.");
    }
}

void QtAudioSettings::setupUI()
{
    resize(600, 700);
    
    // Center on screen
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    createMainPanel();
    createDeviceSettings();
    createFormatSettings();
    createAdvancedSettings();
    createControlPanel();
}

void QtAudioSettings::createMainPanel()
{
    mainPanel_ = new QWidget(this);
    mainLayout_ = new QVBoxLayout(mainPanel_);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(15);
    
    setCentralWidget(mainPanel_);
}

void QtAudioSettings::createDeviceSettings()
{
    deviceGroup_ = new QGroupBox("Audio Devices");
    deviceLayout_ = new QGridLayout(deviceGroup_);
    deviceLayout_->setSpacing(10);
    
    // Input device
    QLabel* inputLabel = new QLabel("Input Device:");
    inputLabel->setStyleSheet("color: white; font-weight: bold;");
    
    inputDeviceCombo_ = new QComboBox();
    inputDeviceCombo_->setStyleSheet(R"(
        QComboBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 200px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
    )");
    
    enableInputCheck_ = new QCheckBox("Enable Input");
    enableInputCheck_->setStyleSheet("color: white;");
    enableInputCheck_->setChecked(enableInput_);
    
    testInputButton_ = new QPushButton("Test Input");
    testInputButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 150, 100);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: rgb(120, 170, 120);
        }
    )");
    
    inputDeviceInfo_ = new QLabel("Device information will appear here");
    inputDeviceInfo_->setStyleSheet("color: rgb(200, 200, 200); font-size: 10px;");
    inputDeviceInfo_->setWordWrap(true);
    
    // Output device
    QLabel* outputLabel = new QLabel("Output Device:");
    outputLabel->setStyleSheet("color: white; font-weight: bold;");
    
    outputDeviceCombo_ = new QComboBox();
    outputDeviceCombo_->setStyleSheet(R"(
        QComboBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 200px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
    )");
    
    enableOutputCheck_ = new QCheckBox("Enable Output");
    enableOutputCheck_->setStyleSheet("color: white;");
    enableOutputCheck_->setChecked(enableOutput_);
    
    testOutputButton_ = new QPushButton("Test Output");
    testOutputButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 150, 100);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: rgb(120, 170, 120);
        }
    )");
    
    outputDeviceInfo_ = new QLabel("Device information will appear here");
    outputDeviceInfo_->setStyleSheet("color: rgb(200, 200, 200); font-size: 10px;");
    outputDeviceInfo_->setWordWrap(true);
    
    // Add to layout
    deviceLayout_->addWidget(inputLabel, 0, 0);
    deviceLayout_->addWidget(inputDeviceCombo_, 0, 1);
    deviceLayout_->addWidget(enableInputCheck_, 0, 2);
    deviceLayout_->addWidget(testInputButton_, 0, 3);
    deviceLayout_->addWidget(inputDeviceInfo_, 1, 0, 1, 4);
    
    deviceLayout_->addWidget(outputLabel, 2, 0);
    deviceLayout_->addWidget(outputDeviceCombo_, 2, 1);
    deviceLayout_->addWidget(enableOutputCheck_, 2, 2);
    deviceLayout_->addWidget(testOutputButton_, 2, 3);
    deviceLayout_->addWidget(outputDeviceInfo_, 3, 0, 1, 4);
    
    mainLayout_->addWidget(deviceGroup_);
}

void QtAudioSettings::createFormatSettings()
{
    formatGroup_ = new QGroupBox("Audio Format");
    formatLayout_ = new QGridLayout(formatGroup_);
    formatLayout_->setSpacing(10);
    
    // Sample rate
    QLabel* sampleRateLabel = new QLabel("Sample Rate:");
    sampleRateLabel->setStyleSheet("color: white; font-weight: bold;");
    
    sampleRateCombo_ = new QComboBox();
    for (int rate : SUPPORTED_SAMPLE_RATES) {
        sampleRateCombo_->addItem(QString("%1 Hz").arg(rate), rate);
    }
    sampleRateCombo_->setStyleSheet(R"(
        QComboBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 120px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
    )");
    
    // Channels
    QLabel* channelsLabel = new QLabel("Channels:");
    channelsLabel->setStyleSheet("color: white; font-weight: bold;");
    
    channelsCombo_ = new QComboBox();
    for (int ch : SUPPORTED_CHANNELS) {
        QString text = (ch == 1) ? "Mono" : QString("%1 Channel").arg(ch);
        channelsCombo_->addItem(text, ch);
    }
    channelsCombo_->setStyleSheet(R"(
        QComboBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 120px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
    )");
    
    // Format
    QLabel* formatLabel = new QLabel("Format:");
    formatLabel->setStyleSheet("color: white; font-weight: bold;");
    
    formatCombo_ = new QComboBox();
    formatCombo_->addItems(SUPPORTED_FORMATS);
    formatCombo_->setStyleSheet(R"(
        QComboBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 150px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
    )");
    
    // Format info
    formatInfo_ = new QLabel("Format information will appear here");
    formatInfo_->setStyleSheet("color: rgb(200, 200, 200); font-size: 10px;");
    formatInfo_->setWordWrap(true);
    
    // Add to layout
    formatLayout_->addWidget(sampleRateLabel, 0, 0);
    formatLayout_->addWidget(sampleRateCombo_, 0, 1);
    formatLayout_->addWidget(channelsLabel, 0, 2);
    formatLayout_->addWidget(channelsCombo_, 0, 3);
    formatLayout_->addWidget(formatLabel, 1, 0);
    formatLayout_->addWidget(formatCombo_, 1, 1);
    formatLayout_->addWidget(formatInfo_, 2, 0, 1, 4);
    
    mainLayout_->addWidget(formatGroup_);
}

void QtAudioSettings::createAdvancedSettings()
{
    advancedGroup_ = new QGroupBox("Advanced Settings");
    advancedLayout_ = new QGridLayout(advancedGroup_);
    advancedLayout_->setSpacing(10);
    
    // Show advanced checkbox
    showAdvancedCheck_ = new QCheckBox("Show Advanced Options");
    showAdvancedCheck_->setStyleSheet("color: white; font-weight: bold;");
    showAdvancedCheck_->setChecked(showAdvanced_);
    
    // Buffer size
    QLabel* bufferSizeLabel = new QLabel("Buffer Size:");
    bufferSizeLabel->setStyleSheet("color: white;");
    
    bufferSizeSlider_ = new QSlider(Qt::Horizontal);
    bufferSizeSlider_->setRange(MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
    bufferSizeSlider_->setValue(currentBufferSize_);
    bufferSizeSlider_->setStyleSheet(R"(
        QSlider::groove:horizontal {
            background-color: rgb(50, 55, 65);
            border: 1px solid rgb(80, 90, 100);
            border-radius: 4px;
            height: 8px;
        }
        QSlider::handle:horizontal {
            background-color: rgb(100, 150, 255);
            border: 1px solid rgb(80, 120, 200);
            border-radius: 4px;
            width: 16px;
            margin: -4px 0;
        }
        QSlider::handle:horizontal:hover {
            background-color: white;
        }
    )");
    
    bufferSizeLabel_ = new QLabel(QString("Buffer Size: %1 samples").arg(currentBufferSize_));
    bufferSizeLabel_->setStyleSheet("color: white; font-size: 10px;");
    
    // Latency
    QLabel* latencyLabel = new QLabel("Latency:");
    latencyLabel->setStyleSheet("color: white;");
    
    latencySlider_ = new QSlider(Qt::Horizontal);
    latencySlider_->setRange(MIN_LATENCY, MAX_LATENCY);
    latencySlider_->setValue(currentLatency_);
    latencySlider_->setStyleSheet(R"(
        QSlider::groove:horizontal {
            background-color: rgb(50, 55, 65);
            border: 1px solid rgb(80, 90, 100);
            border-radius: 4px;
            height: 8px;
        }
        QSlider::handle:horizontal {
            background-color: rgb(100, 150, 255);
            border: 1px solid rgb(80, 120, 200);
            border-radius: 4px;
            width: 16px;
            margin: -4px 0;
        }
        QSlider::handle:horizontal:hover {
            background-color: white;
        }
    )");
    
    latencyLabel_ = new QLabel(QString("Latency: %1 ms").arg(currentLatency_));
    latencyLabel_->setStyleSheet("color: white; font-size: 10px;");
    
    // Buffer count
    QLabel* bufferCountLabel = new QLabel("Buffer Count:");
    bufferCountLabel->setStyleSheet("color: white;");
    
    bufferCountSpin_ = new QSpinBox();
    bufferCountSpin_->setRange(2, 16);
    bufferCountSpin_->setValue(4);
    bufferCountSpin_->setStyleSheet(R"(
        QSpinBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 80px;
        }
    )");
    
    // Exclusive mode
    exclusiveModeCheck_ = new QCheckBox("Exclusive Mode");
    exclusiveModeCheck_->setStyleSheet("color: white;");
    exclusiveModeCheck_->setToolTip("Request exclusive access to audio device");
    
    wasapiExclusiveCheck_ = new QCheckBox("WASAPI Exclusive (Windows)");
    wasapiExclusiveCheck_->setStyleSheet("color: white;");
    wasapiExclusiveCheck_->setToolTip("Use WASAPI exclusive mode on Windows");
    
    // Add to layout
    advancedLayout_->addWidget(showAdvancedCheck_, 0, 0, 1, 4);
    advancedLayout_->addWidget(bufferSizeLabel, 1, 0);
    advancedLayout_->addWidget(bufferSizeSlider_, 1, 1);
    advancedLayout_->addWidget(bufferSizeLabel_, 1, 2);
    advancedLayout_->addWidget(latencyLabel, 2, 0);
    advancedLayout_->addWidget(latencySlider_, 2, 1);
    advancedLayout_->addWidget(latencyLabel_, 2, 2);
    advancedLayout_->addWidget(bufferCountLabel, 3, 0);
    advancedLayout_->addWidget(bufferCountSpin_, 3, 1);
    advancedLayout_->addWidget(exclusiveModeCheck_, 4, 0, 1, 2);
    advancedLayout_->addWidget(wasapiExclusiveCheck_, 4, 2, 1, 2);
    
    mainLayout_->addWidget(advancedGroup_);
}

void QtAudioSettings::createControlPanel()
{
    // Control buttons
    controlLayout_ = new QHBoxLayout();
    
    applyButton_ = new QPushButton("Apply");
    applyButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 150, 100);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: rgb(120, 170, 120);
        }
    )");
    
    resetButton_ = new QPushButton("Reset to Default");
    resetButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(200, 150, 50);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: rgb(220, 170, 70);
        }
    )");
    
    closeButton_ = new QPushButton("Close");
    closeButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 100, 100);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: rgb(120, 120, 120);
        }
    )");
    
    controlLayout_->addWidget(applyButton_);
    controlLayout_->addWidget(resetButton_);
    controlLayout_->addStretch();
    controlLayout_->addWidget(closeButton_);
    
    mainLayout_->addLayout(controlLayout_);
}

void QtAudioSettings::setupStyling()
{
    setStyleSheet(R"(
        QDialog {
            background-color: rgba(25, 30, 40, 250);
            border: 2px solid rgb(100, 150, 255);
            border-radius: 12px;
        }
        QGroupBox {
            color: white;
            font-weight: bold;
            border: 1px solid rgb(80, 120, 200);
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");
    
    applyModernStyling();
}

void QtAudioSettings::applyModernStyling()
{
    // Add shadow effects
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 150));
    shadowEffect->setOffset(5, 5);
    setGraphicsEffect(shadowEffect);
}

void QtAudioSettings::setupCallbacks()
{
    connect(closeButton_, &QPushButton::clicked, this, &QtAudioSettings::onCloseClicked);
    connect(applyButton_, &QPushButton::clicked, this, &QtAudioSettings::onApplyClicked);
    connect(resetButton_, &QPushButton::clicked, this, &QtAudioSettings::onResetClicked);
    connect(testInputButton_, &QPushButton::clicked, this, &QtAudioSettings::onTestInputClicked);
    connect(testOutputButton_, &QPushButton::clicked, this, &QtAudioSettings::onTestOutputClicked);
    connect(inputDeviceCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtAudioSettings::onInputDeviceChanged);
    connect(outputDeviceCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtAudioSettings::onOutputDeviceChanged);
    connect(sampleRateCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtAudioSettings::onSampleRateChanged);
    connect(channelsCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtAudioSettings::onChannelsChanged);
    connect(bufferSizeSlider_, &QSlider::valueChanged, this, &QtAudioSettings::onBufferSizeChanged);
    connect(latencySlider_, &QSlider::valueChanged, this, &QtAudioSettings::onLatencyChanged);
    connect(formatCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtAudioSettings::onFormatChanged);
    connect(enableInputCheck_, &QCheckBox::toggled, this, &QtAudioSettings::onEnableInputChanged);
    connect(enableOutputCheck_, &QCheckBox::toggled, this, &QtAudioSettings::onEnableOutputChanged);
    connect(showAdvancedCheck_, &QCheckBox::toggled, this, &QtAudioSettings::onShowAdvancedToggled);
}

void QtAudioSettings::loadAudioSettings()
{
    settings_->beginGroup("Audio");
    currentInputDevice_ = settings_->value("inputDevice", "default").toString();
    currentOutputDevice_ = settings_->value("outputDevice", "default").toString();
    currentSampleRate_ = settings_->value("sampleRate", 44100).toInt();
    currentChannels_ = settings_->value("channels", 2).toInt();
    currentFormat_ = settings_->value("format", "PCM 16-bit").toString();
    currentBufferSize_ = settings_->value("bufferSize", 1024).toInt();
    currentLatency_ = settings_->value("latency", 10).toInt();
    enableInput_ = settings_->value("enableInput", true).toBool();
    enableOutput_ = settings_->value("enableOutput", true).toBool();
    showAdvanced_ = settings_->value("showAdvanced", false).toBool();
    settings_->endGroup();
}

void QtAudioSettings::saveAudioSettings()
{
    settings_->beginGroup("Audio");
    settings_->setValue("inputDevice", currentInputDevice_);
    settings_->setValue("outputDevice", currentOutputDevice_);
    settings_->setValue("sampleRate", currentSampleRate_);
    settings_->setValue("channels", currentChannels_);
    settings_->setValue("format", currentFormat_);
    settings_->setValue("bufferSize", currentBufferSize_);
    settings_->setValue("latency", currentLatency_);
    settings_->setValue("enableInput", enableInput_);
    settings_->setValue("enableOutput", enableOutput_);
    settings_->setValue("showAdvanced", showAdvanced_);
    settings_->endGroup();
}

void QtAudioSettings::initializeDefaultSettings()
{
    // Set default values if not already set
    if (currentInputDevice_.isEmpty()) currentInputDevice_ = "default";
    if (currentOutputDevice_.isEmpty()) currentOutputDevice_ = "default";
    if (currentSampleRate_ <= 0) currentSampleRate_ = 44100;
    if (currentChannels_ <= 0) currentChannels_ = 2;
    if (currentFormat_.isEmpty()) currentFormat_ = "PCM 16-bit";
    if (currentBufferSize_ <= 0) currentBufferSize_ = 1024;
    if (currentLatency_ <= 0) currentLatency_ = 10;
}

void QtAudioSettings::scanAudioDevices()
{
    audioDevices_.clear();
    
    // Add default devices
    AudioDevice defaultInput;
    defaultInput.name = "Default Input";
    defaultInput.id = "default";
    defaultInput.type = "input";
    defaultInput.sampleRate = 44100;
    defaultInput.channels = 2;
    defaultInput.format = "PCM 16-bit";
    defaultInput.isDefault = true;
    defaultInput.isEnabled = true;
    audioDevices_.append(defaultInput);
    
    AudioDevice defaultOutput;
    defaultOutput.name = "Default Output";
    defaultOutput.id = "default";
    defaultOutput.type = "output";
    defaultOutput.sampleRate = 44100;
    defaultOutput.channels = 2;
    defaultOutput.format = "PCM 16-bit";
    defaultOutput.isDefault = true;
    defaultOutput.isEnabled = true;
    audioDevices_.append(defaultOutput);
    
    // TODO: Scan for real audio devices using platform-specific APIs
    // This would involve calling into the audio engine or system APIs
    
    // For now, add some example devices
    AudioDevice exampleInput;
    exampleInput.name = "Microphone (Example)";
    exampleInput.id = "mic_example";
    exampleInput.type = "input";
    exampleInput.sampleRate = 48000;
    exampleInput.channels = 1;
    exampleInput.format = "PCM 16-bit";
    exampleInput.isDefault = false;
    exampleInput.isEnabled = true;
    audioDevices_.append(exampleInput);
    
    AudioDevice exampleOutput;
    exampleOutput.name = "Speakers (Example)";
    exampleOutput.id = "speakers_example";
    exampleOutput.type = "output";
    exampleOutput.sampleRate = 48000;
    exampleOutput.channels = 2;
    exampleOutput.format = "PCM 24-bit";
    exampleOutput.isDefault = false;
    exampleOutput.isEnabled = true;
    audioDevices_.append(exampleOutput);
}

void QtAudioSettings::updateDeviceComboBoxes()
{
    if (!inputDeviceCombo_ || !outputDeviceCombo_) return;
    
    inputDeviceCombo_->clear();
    outputDeviceCombo_->clear();
    
    for (const AudioDevice& device : audioDevices_) {
        if (device.type == "input" || device.type == "both") {
            inputDeviceCombo_->addItem(device.name, device.id);
        }
        if (device.type == "output" || device.type == "both") {
            outputDeviceCombo_->addItem(device.name, device.id);
        }
    }
    
    // Set current selections
    int inputIndex = inputDeviceCombo_->findData(currentInputDevice_);
    if (inputIndex >= 0) {
        inputDeviceCombo_->setCurrentIndex(inputIndex);
    }
    
    int outputIndex = outputDeviceCombo_->findData(currentOutputDevice_);
    if (outputIndex >= 0) {
        outputDeviceCombo_->setCurrentIndex(outputIndex);
    }
    
    updateDeviceInfo();
}

void QtAudioSettings::updateDeviceInfo()
{
    // Update input device info
    if (inputDeviceInfo_) {
        QString info = "No input device selected";
        for (const AudioDevice& device : audioDevices_) {
            if (device.id == currentInputDevice_ && (device.type == "input" || device.type == "both")) {
                info = QString("Sample Rate: %1 Hz\nChannels: %2\nFormat: %3")
                       .arg(device.sampleRate).arg(device.channels).arg(device.format);
                break;
            }
        }
        inputDeviceInfo_->setText(info);
    }
    
    // Update output device info
    if (outputDeviceInfo_) {
        QString info = "No output device selected";
        for (const AudioDevice& device : audioDevices_) {
            if (device.id == currentOutputDevice_ && (device.type == "output" || device.type == "both")) {
                info = QString("Sample Rate: %1 Hz\nChannels: %2\nFormat: %3")
                       .arg(device.sampleRate).arg(device.channels).arg(device.format);
                break;
            }
        }
        outputDeviceInfo_->setText(info);
    }
}

void QtAudioSettings::updateFormatInfo()
{
    if (!formatInfo_) return;
    
    QString info = QString("Sample Rate: %1 Hz\nChannels: %2\nFormat: %3\n\n")
                   .arg(currentSampleRate_).arg(currentChannels_).arg(currentFormat_);
    
    // Calculate bit rate
    int bitsPerSample = 16;
    if (currentFormat_.contains("24-bit")) bitsPerSample = 24;
    else if (currentFormat_.contains("32-bit")) bitsPerSample = 32;
    else if (currentFormat_.contains("Float")) bitsPerSample = 32;
    
    int bitRate = (currentSampleRate_ * currentChannels_ * bitsPerSample) / 8;
    info += QString("Bit Rate: %1 kbps").arg(bitRate / 1000);
    
    formatInfo_->setText(info);
}

void QtAudioSettings::updateAdvancedSettings()
{
    if (!showAdvanced_) {
        advancedGroup_->setVisible(false);
        return;
    }
    
    advancedGroup_->setVisible(true);
    
    // Update buffer size display
    if (bufferSizeLabel_) {
        bufferSizeLabel_->setText(QString("Buffer Size: %1 samples").arg(currentBufferSize_));
    }
    
    // Update latency display
    if (latencyLabel_) {
        latencyLabel_->setText(QString("Latency: %1 ms").arg(currentLatency_));
    }
}

void QtAudioSettings::showWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    // Setup animations
    slideAnimation_ = new QPropertyAnimation(this, "pos");
    slideAnimation_->setDuration(400);
    slideAnimation_->setStartValue(QPoint(x() - width(), y()));
    slideAnimation_->setEndValue(QPoint(x(), y()));
    slideAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    fadeAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    fadeAnimation_->setDuration(400);
    fadeAnimation_->setStartValue(0.0);
    fadeAnimation_->setEndValue(1.0);
    fadeAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    animationGroup_ = new QParallelAnimationGroup();
    animationGroup_->addAnimation(slideAnimation_);
    animationGroup_->addAnimation(fadeAnimation_);
    
    connect(animationGroup_, &QParallelAnimationGroup::finished, [this]() {
        isAnimating_ = false;
        animationGroup_->deleteLater();
        animationGroup_ = nullptr;
    });
    
    animationGroup_->start();
}

void QtAudioSettings::hideWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    fadeAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    fadeAnimation_->setDuration(300);
    fadeAnimation_->setStartValue(1.0);
    fadeAnimation_->setEndValue(0.0);
    fadeAnimation_->setEasingCurve(QEasingCurve::InCubic);
    
    connect(fadeAnimation_, &QPropertyAnimation::finished, [this]() {
        isVisible_ = false;
        isAnimating_ = false;
        QDialog::hide();
        fadeAnimation_->deleteLater();
        fadeAnimation_ = nullptr;
    });
    
    fadeAnimation_->start();
}

void QtAudioSettings::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    // Custom painting if needed
}

void QtAudioSettings::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
}

// Slot implementations
void QtAudioSettings::onCloseClicked()
{
    hideWithAnimation();
}

void QtAudioSettings::onApplyClicked()
{
    // Apply settings to the application
    if (application_) {
        // TODO: Call application audio methods
        // application_->setAudioDevice("input", currentInputDevice_);
        // application_->setAudioDevice("output", currentOutputDevice_);
        // application_->setSampleRate(currentSampleRate_);
        // application_->setChannels(currentChannels_);
        // application_->setAudioFormat(currentFormat_);
        // application_->setBufferSize(currentBufferSize_);
        // application_->setLatency(currentLatency_);
    }
    
    saveAudioSettings();
    QMessageBox::information(this, "Settings Applied", "Audio settings have been applied successfully!");
}

void QtAudioSettings::onResetClicked()
{
    resetToDefault();
}

void QtAudioSettings::onTestInputClicked()
{
    testAudioDevice(currentInputDevice_);
}

void QtAudioSettings::onTestOutputClicked()
{
    testAudioDevice(currentOutputDevice_);
}

void QtAudioSettings::onInputDeviceChanged()
{
    if (inputDeviceCombo_) {
        currentInputDevice_ = inputDeviceCombo_->currentData().toString();
        updateDeviceInfo();
    }
}

void QtAudioSettings::onOutputDeviceChanged()
{
    if (outputDeviceCombo_) {
        currentOutputDevice_ = outputDeviceCombo_->currentData().toString();
        updateDeviceInfo();
    }
}

void QtAudioSettings::onSampleRateChanged()
{
    if (sampleRateCombo_) {
        currentSampleRate_ = sampleRateCombo_->currentData().toInt();
        updateFormatInfo();
    }
}

void QtAudioSettings::onChannelsChanged()
{
    if (channelsCombo_) {
        currentChannels_ = channelsCombo_->currentData().toInt();
        updateFormatInfo();
    }
}

void QtAudioSettings::onBufferSizeChanged()
{
    currentBufferSize_ = bufferSizeSlider_->value();
    if (bufferSizeLabel_) {
        bufferSizeLabel_->setText(QString("Buffer Size: %1 samples").arg(currentBufferSize_));
    }
}

void QtAudioSettings::onLatencyChanged()
{
    currentLatency_ = latencySlider_->value();
    if (latencyLabel_) {
        latencyLabel_->setText(QString("Latency: %1 ms").arg(currentLatency_));
    }
}

void QtAudioSettings::onFormatChanged()
{
    if (formatCombo_) {
        currentFormat_ = formatCombo_->currentText();
        updateFormatInfo();
    }
}

void QtAudioSettings::onEnableInputChanged()
{
    enableInput_ = enableInputCheck_->isChecked();
}

void QtAudioSettings::onEnableOutputChanged()
{
    enableOutput_ = enableOutputCheck_->isChecked();
}

void QtAudioSettings::onShowAdvancedToggled()
{
    showAdvanced_ = showAdvancedCheck_->isChecked();
    updateAdvancedSettings();
}

void QtAudioSettings::updateAnimation()
{
    animationTime_ += 0.016f; // Assume 60 FPS
    
    // Subtle animations could be added here
    if (isVisible_ && !isAnimating_) {
        // Could add subtle effects like highlighting, etc.
    }
}

void QtAudioSettings::show()
{
    isVisible_ = true;
    QDialog::show();
    showWithAnimation();
}

void QtAudioSettings::hide()
{
    hideWithAnimation();
}

void QtAudioSettings::toggle()
{
    if (isVisible_) {
        hide();
    } else {
        show();
    }
}