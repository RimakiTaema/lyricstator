#include "QtEqualizer.h"
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
#include <QRadialGradient>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QSettings>
#include <QLineEdit>
#include <cmath>
#include <algorithm>

QtEqualizer::QtEqualizer(QWidget *parent)
    : QDialog(parent)
    , application_(nullptr)
    , mainPanel_(nullptr)
    , titleLabel_(nullptr)
    , closeButton_(nullptr)
    , enableButton_(nullptr)
    , resetButton_(nullptr)
    , presetComboBox_(nullptr)
    , savePresetButton_(nullptr)
    , searchBox_(nullptr)
    , spectrumCanvas_(nullptr)
    , bandsContainer_(nullptr)
    , slideAnimation_(nullptr)
    , fadeAnimation_(nullptr)
    , animationGroup_(nullptr)
    , animationTimer_(nullptr)
    , spectrumUpdateTimer_(nullptr)
    , settings_(nullptr)
    , animationTime_(0.0f)
    , isVisible_(false)
    , isAnimating_(false)
    , equalizerEnabled_(true)
    , showSpectrum_(true)
    , bandCount_(DEFAULT_BAND_COUNT)
{
    setWindowTitle("Audio Equalizer");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(false);
    
    // Initialize spectrum data
    spectrumData_.resize(64, 0.0f);
    smoothedSpectrum_.resize(64, 0.0f);
    
    // Initialize settings
    settings_ = new QSettings("Lyricstator", "Equalizer", this);
    
    setupUI();
    setupStyling();
    setupCallbacks();
    loadEqualizerSettings();
    initializeDefaultPresets();
    
    // Setup animation timer
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &QtEqualizer::updateAnimation);
    animationTimer_->start(16); // ~60 FPS
    
    // Setup spectrum update timer
    spectrumUpdateTimer_ = new QTimer(this);
    connect(spectrumUpdateTimer_, &QTimer::timeout, this, &QtEqualizer::updateSpectrumVisualization);
    spectrumUpdateTimer_->start(50); // 20 FPS for spectrum updates
    
    // Initialize bands
    setBandCount(bandCount_);
}

QtEqualizer::~QtEqualizer()
{
    saveEqualizerSettings();
    if (animationTimer_) {
        animationTimer_->stop();
    }
    if (spectrumUpdateTimer_) {
        spectrumUpdateTimer_->stop();
    }
}

void QtEqualizer::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtEqualizer::setBandCount(int bandCount)
{
    bandCount = std::clamp(bandCount, 3, 48);
    bandCount_ = bandCount;
    
    // Clear existing bands
    bands_.clear();
    for (auto widget : bandControlWidgets_) {
        widget->deleteLater();
    }
    bandControlWidgets_.clear();
    bandSliders_.clear();
    bandLabels_.clear();
    
    // Create new bands
    for (int i = 0; i < bandCount; ++i) {
        float frequency = getFrequencyForBand(i, bandCount);
        EqualizerBand band;
        band.index = i;
        band.frequency = frequency;
        band.gain = 0.0f;
        band.qFactor = DEFAULT_Q_FACTOR;
        band.enabled = true;
        bands_.append(band);
        
        createBandControl(i, frequency);
    }
    
    // Load saved gains if available
    loadEqualizerSettings();
    
    // Update layout
    updateBandColors();
}

void QtEqualizer::setBandGain(int bandIndex, float gain)
{
    if (bandIndex >= 0 && bandIndex < bands_.size()) {
        bands_[bandIndex].gain = std::clamp(gain, MIN_GAIN, MAX_GAIN);
        
        // Update slider if it exists
        if (bandIndex < bandSliders_.size() && bandSliders_[bandIndex]) {
            bandSliders_[bandIndex]->setValue(bands_[bandIndex].gain);
        }
        
        // Update label if it exists
        if (bandIndex < bandLabels_.size() && bandLabels_[bandIndex]) {
            bandLabels_[bandIndex]->setText(QString("%1dB").arg(bands_[bandIndex].gain, 0, 'f', 1));
        }
        
        // Apply to audio engine if available
        if (application_) {
            // TODO: Call application equalizer methods
            // application_->setEqualizerBandGain(bandIndex, bands_[bandIndex].gain);
        }
        
        saveEqualizerSettings();
    }
}

float QtEqualizer::getBandGain(int bandIndex) const
{
    if (bandIndex >= 0 && bandIndex < bands_.size()) {
        return bands_[bandIndex].gain;
    }
    return 0.0f;
}

void QtEqualizer::setEqualizerEnabled(bool enabled)
{
    equalizerEnabled_ = enabled;
    
    if (enableButton_) {
        enableButton_->setText(enabled ? "ON" : "OFF");
        enableButton_->setStyleSheet(enabled ? 
            "background-color: rgb(50, 200, 50);" : 
            "background-color: rgb(200, 50, 50);");
    }
    
    // Apply to audio engine if available
    if (application_) {
        // TODO: Call application equalizer methods
        // application_->setEqualizerEnabled(enabled);
    }
    
    saveEqualizerSettings();
}

bool QtEqualizer::isEqualizerEnabled() const
{
    return equalizerEnabled_;
}

void QtEqualizer::resetToDefault()
{
    for (int i = 0; i < bands_.size(); ++i) {
        setBandGain(i, 0.0f);
    }
    
    currentPreset_ = "Flat";
    if (presetComboBox_) {
        presetComboBox_->setCurrentText("Flat");
    }
    
    saveEqualizerSettings();
}

void QtEqualizer::loadPreset(const QString& presetName)
{
    if (presets_.contains(presetName)) {
        applyPreset(presets_[presetName]);
        currentPreset_ = presetName;
        saveEqualizerSettings();
    }
}

void QtEqualizer::savePreset(const QString& presetName, const QString& description)
{
    EqualizerPreset preset;
    preset.name = presetName;
    preset.description = description;
    preset.bandGains.resize(bands_.size());
    
    for (int i = 0; i < bands_.size(); ++i) {
        preset.bandGains[i] = bands_[i].gain;
    }
    
    presets_[presetName] = preset;
    refreshPresetList();
    
    // Save to file
    savePresetsToFile();
}

void QtEqualizer::deletePreset(const QString& presetName)
{
    if (presets_.contains(presetName)) {
        presets_.remove(presetName);
        refreshPresetList();
        savePresetsToFile();
    }
}

QStringList QtEqualizer::getAvailablePresets() const
{
    return presets_.keys();
}

void QtEqualizer::updateSpectrumData(const QVector<float>& spectrumData)
{
    if (spectrumData.size() == spectrumData_.size()) {
        spectrumData_ = spectrumData;
        
        // Smooth the spectrum data
        for (int i = 0; i < smoothedSpectrum_.size(); ++i) {
            smoothedSpectrum_[i] = smoothedSpectrum_[i] * 0.7f + spectrumData_[i] * 0.3f;
        }
    }
}

void QtEqualizer::setupUI()
{
    resize(400, 600);
    
    // Center on screen
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    createMainPanel();
    createControlPanel();
    createSpectrumVisualization();
    createBandControls();
}

void QtEqualizer::createMainPanel()
{
    mainPanel_ = new QWidget(this);
    mainLayout_ = new QVBoxLayout(mainPanel_);
    mainLayout_->setContentsMargins(15, 15, 15, 15);
    mainLayout_->setSpacing(10);
    
    setCentralWidget(mainPanel_);
}

void QtEqualizer::createControlPanel()
{
    // Title bar
    titleLabel_ = new QLabel("Audio Equalizer");
    titleLabel_->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 22px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
    )");
    
    closeButton_ = new QPushButton("×");
    closeButton_->setFixedSize(30, 30);
    closeButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(220, 60, 60);
            color: white;
            border: none;
            border-radius: 15px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(255, 80, 80);
        }
    )");
    
    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->addWidget(titleLabel_);
    titleLayout->addStretch();
    titleLayout->addWidget(closeButton_);
    
    mainLayout_->addLayout(titleLayout);
    
    // Control buttons
    controlLayout_ = new QHBoxLayout();
    
    enableButton_ = new QPushButton("ON");
    enableButton_->setFixedSize(60, 30);
    enableButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(50, 200, 50);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(70, 220, 70);
        }
    )");
    
    resetButton_ = new QPushButton("Reset");
    resetButton_->setFixedSize(60, 30);
    resetButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(200, 150, 50);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(220, 170, 70);
        }
    )");
    
    presetComboBox_ = new QComboBox();
    presetComboBox_->setStyleSheet(R"(
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
    
    savePresetButton_ = new QPushButton("Save");
    savePresetButton_->setFixedSize(60, 30);
    savePresetButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 100, 200);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(120, 120, 220);
        }
    )");
    
    controlLayout_->addWidget(enableButton_);
    controlLayout_->addWidget(resetButton_);
    controlLayout_->addWidget(presetComboBox_);
    controlLayout_->addWidget(savePresetButton_);
    controlLayout_->addStretch();
    
    mainLayout_->addLayout(controlLayout_);
    
    // Search box
    searchBox_ = new QLineEdit();
    searchBox_->setPlaceholderText("Search presets...");
    searchBox_->setStyleSheet(R"(
        QLineEdit {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
        }
        QLineEdit:focus {
            border-color: rgb(120, 180, 255);
        }
    )");
    
    mainLayout_->addWidget(searchBox_);
}

void QtEqualizer::createSpectrumVisualization()
{
    spectrumCanvas_ = new QFrame();
    spectrumCanvas_->setFixedHeight(100);
    spectrumCanvas_->setStyleSheet(R"(
        QFrame {
            background-color: rgb(15, 20, 30);
            border: 1px solid rgb(80, 120, 200);
            border-radius: 8px;
        }
    )");
    
    mainLayout_->addWidget(spectrumCanvas_);
}

void QtEqualizer::createBandControls()
{
    bandsContainer_ = new QWidget();
    bandsLayout_ = new QGridLayout(bandsContainer_);
    bandsLayout_->setSpacing(5);
    bandsLayout_->setContentsMargins(5, 5, 5, 5);
    
    mainLayout_->addWidget(bandsContainer_);
}

void QtEqualizer::createBandControl(int bandIndex, float frequency)
{
    QWidget* bandWidget = new QWidget();
    QVBoxLayout* bandLayout = new QVBoxLayout(bandWidget);
    bandLayout->setSpacing(5);
    bandLayout->setContentsMargins(5, 5, 5, 5);
    
    // Frequency label
    QLabel* freqLabel = new QLabel(formatFrequency(frequency));
    freqLabel->setStyleSheet(R"(
        QLabel {
            color: rgb(200, 200, 200);
            font-size: 10px;
            background: transparent;
            border: none;
        }
    )");
    freqLabel->setAlignment(Qt::AlignCenter);
    bandLayout->addWidget(freqLabel);
    
    // Gain slider (vertical)
    QSlider* gainSlider = new QSlider(Qt::Vertical);
    gainSlider->setRange(MIN_GAIN * 10, MAX_GAIN * 10);
    gainSlider->setValue(0);
    gainSlider->setStyleSheet(R"(
        QSlider::groove:vertical {
            background-color: rgb(50, 55, 65);
            border: 1px solid rgb(80, 90, 100);
            border-radius: 4px;
            width: 8px;
        }
        QSlider::handle:vertical {
            background-color: rgb(100, 150, 255);
            border: 1px solid rgb(80, 120, 200);
            border-radius: 4px;
            height: 16px;
            margin: 0 -4px;
        }
        QSlider::handle:vertical:hover {
            background-color: white;
        }
    )");
    bandLayout->addWidget(gainSlider);
    
    // Gain label
    QLabel* gainLabel = new QLabel("0.0dB");
    gainLabel->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 9px;
            background: transparent;
            border: none;
        }
    )");
    gainLabel->setAlignment(Qt::AlignCenter);
    bandLayout->addWidget(gainLabel);
    
    // Store references
    bandControlWidgets_.append(bandWidget);
    bandSliders_.append(gainSlider);
    bandLabels_.append(gainLabel);
    
    // Connect slider
    connect(gainSlider, QOverload<int>::of(&QSlider::valueChanged), 
            [this, bandIndex](int value) {
                onBandGainChanged(bandIndex, value / 10.0);
            });
    
    // Add to grid layout
    int row = bandIndex / 5;
    int col = bandIndex % 5;
    bandsLayout_->addWidget(bandWidget, row, col);
}

void QtEqualizer::setupStyling()
{
    setStyleSheet(R"(
        QDialog {
            background-color: rgba(20, 25, 35, 245);
            border: 2px solid rgb(100, 200, 255);
            border-radius: 15px;
        }
    )");
    
    applyModernStyling();
}

void QtEqualizer::applyModernStyling()
{
    // Add shadow effects
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 150));
    shadowEffect->setOffset(5, 5);
    setGraphicsEffect(shadowEffect);
}

void QtEqualizer::setupCallbacks()
{
    connect(enableButton_, &QPushButton::clicked, this, &QtEqualizer::onEnableToggled);
    connect(resetButton_, &QPushButton::clicked, this, &QtEqualizer::onResetClicked);
    connect(presetComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtEqualizer::onPresetSelected);
    connect(savePresetButton_, &QPushButton::clicked, this, &QtEqualizer::onSavePresetClicked);
    connect(closeButton_, &QPushButton::clicked, this, &QtEqualizer::onCloseClicked);
    connect(searchBox_, &QLineEdit::textChanged, this, &QtEqualizer::onSearchTextChanged);
}

void QtEqualizer::loadEqualizerSettings()
{
    settings_->beginGroup("Equalizer");
    equalizerEnabled_ = settings_->value("enabled", true).toBool();
    bandCount_ = settings_->value("bandCount", DEFAULT_BAND_COUNT).toInt();
    currentPreset_ = settings_->value("currentPreset", "Flat").toString();
    
    // Load band gains
    for (int i = 0; i < bands_.size(); ++i) {
        float gain = settings_->value(QString("band%1_gain").arg(i), 0.0f).toFloat();
        bands_[i].gain = gain;
    }
    settings_->endGroup();
    
    // Apply loaded settings
    setEqualizerEnabled(equalizerEnabled_);
    for (int i = 0; i < bands_.size(); ++i) {
        if (i < bandSliders_.size()) {
            bandSliders_[i]->setValue(bands_[i].gain * 10);
        }
        if (i < bandLabels_.size()) {
            bandLabels_[i]->setText(QString("%1dB").arg(bands_[i].gain, 0, 'f', 1));
        }
    }
}

void QtEqualizer::saveEqualizerSettings()
{
    settings_->beginGroup("Equalizer");
    settings_->setValue("enabled", equalizerEnabled_);
    settings_->setValue("bandCount", bandCount_);
    settings_->setValue("currentPreset", currentPreset_);
    
    // Save band gains
    for (int i = 0; i < bands_.size(); ++i) {
        settings_->setValue(QString("band%1_gain").arg(i), bands_[i].gain);
    }
    settings_->endGroup();
}

void QtEqualizer::initializeDefaultPresets()
{
    // Flat response
    EqualizerPreset flat;
    flat.name = "Flat";
    flat.description = "No frequency modification";
    flat.bandGains.resize(DEFAULT_BAND_COUNT, 0.0f);
    presets_["Flat"] = flat;
    
    // Bass boost
    EqualizerPreset bassBoost;
    bassBoost.name = "Bass Boost";
    bassBoost.description = "Enhanced low frequencies";
    bassBoost.bandGains.resize(DEFAULT_BAND_COUNT, 0.0f);
    for (int i = 0; i < 3; ++i) {
        bassBoost.bandGains[i] = 6.0f;
    }
    presets_["Bass Boost"] = bassBoost;
    
    // Treble boost
    EqualizerPreset trebleBoost;
    trebleBoost.name = "Treble Boost";
    trebleBoost.description = "Enhanced high frequencies";
    trebleBoost.bandGains.resize(DEFAULT_BAND_COUNT, 0.0f);
    for (int i = 7; i < DEFAULT_BAND_COUNT; ++i) {
        trebleBoost.bandGains[i] = 6.0f;
    }
    presets_["Treble Boost"] = trebleBoost;
    
    // Rock
    EqualizerPreset rock;
    rock.name = "Rock";
    rock.description = "Rock music optimization";
    rock.bandGains.resize(DEFAULT_BAND_COUNT, 0.0f);
    rock.bandGains[0] = 4.0f;  // Bass
    rock.bandGains[1] = 2.0f;
    rock.bandGains[7] = 3.0f;  // Treble
    rock.bandGains[8] = 4.0f;
    presets_["Rock"] = rock;
    
    // Jazz
    EqualizerPreset jazz;
    jazz.name = "Jazz";
    jazz.description = "Jazz music optimization";
    jazz.bandGains.resize(DEFAULT_BAND_COUNT, 0.0f);
    jazz.bandGains[1] = 3.0f;  // Low-mid
    jazz.bandGains[2] = 2.0f;
    jazz.bandGains[6] = 2.0f;  // High-mid
    presets_["Jazz"] = jazz;
    
    // Load custom presets from file
    loadPresetsFromFile();
    
    refreshPresetList();
}

void QtEqualizer::refreshPresetList()
{
    if (!presetComboBox_) return;
    
    presetComboBox_->clear();
    for (auto it = presets_.begin(); it != presets_.end(); ++it) {
        presetComboBox_->addItem(it->name);
    }
    
    // Set current preset
    int index = presetComboBox_->findText(currentPreset_);
    if (index >= 0) {
        presetComboBox_->setCurrentIndex(index);
    }
}

void QtEqualizer::applyPreset(const EqualizerPreset& preset)
{
    if (preset.bandGains.size() != bands_.size()) return;
    
    for (int i = 0; i < bands_.size(); ++i) {
        setBandGain(i, preset.bandGains[i]);
    }
}

void QtEqualizer::showWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    // Setup animations
    slideAnimation_ = new QPropertyAnimation(this, "pos");
    slideAnimation_->setDuration(400);
    slideAnimation_->setStartValue(QPoint(x() + width(), y()));
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

void QtEqualizer::hideWithAnimation()
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

void QtEqualizer::updateBandColors()
{
    for (int i = 0; i < bandSliders_.size(); ++i) {
        QColor bandColor = getBandColor(i, bandSliders_.size());
        QString styleSheet = QString(R"(
            QSlider::handle:vertical {
                background-color: rgb(%1, %2, %3);
                border: 1px solid rgb(%4, %5, %6);
                border-radius: 4px;
                height: 16px;
                margin: 0 -4px;
            }
            QSlider::handle:vertical:hover {
                background-color: white;
            }
        )").arg(bandColor.red()).arg(bandColor.green()).arg(bandColor.blue())
          .arg(bandColor.darker(120).red()).arg(bandColor.darker(120).green()).arg(bandColor.darker(120).blue());
        
        bandSliders_[i]->setStyleSheet(styleSheet);
    }
}

void QtEqualizer::updateSpectrumData()
{
    // This would typically get real-time spectrum data from the audio engine
    if (application_ && equalizerEnabled_) {
        // TODO: Get spectrum data from application
        // QVector<float> newSpectrum = application_->getAudioSpectrum();
        // updateSpectrumData(newSpectrum);
    }
}

float QtEqualizer::getFrequencyForBand(int bandIndex, int totalBands) const
{
    // Logarithmic frequency distribution
    float minFreq = 20.0f;  // 20 Hz
    float maxFreq = 20000.0f; // 20 kHz
    
    if (totalBands <= 1) return minFreq;
    
    float ratio = std::pow(maxFreq / minFreq, 1.0f / (totalBands - 1));
    return minFreq * std::pow(ratio, bandIndex);
}

QString QtEqualizer::formatFrequency(float frequency) const
{
    if (frequency >= 1000.0f) {
        return QString("%1k").arg(frequency / 1000.0f, 0, 'f', 1);
    } else {
        return QString("%1").arg(frequency, 0, 'f', 0);
    }
}

QColor QtEqualizer::getBandColor(int bandIndex, int totalBands) const
{
    // Create a color gradient across the frequency spectrum
    float hue = (float)bandIndex / totalBands * 240.0f; // 0 = red, 240 = blue
    
    QColor color;
    color.setHsv(hue, 200, 255);
    return color;
}

void QtEqualizer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    if (spectrumCanvas_) {
        QPainter painter(spectrumCanvas_);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Draw spectrum bars
        int barWidth = spectrumCanvas_->width() / spectrumData_.size();
        int barHeight = spectrumCanvas_->height();
        
        for (int i = 0; i < spectrumData_.size(); ++i) {
            float value = smoothedSpectrum_[i];
            int height = (int)(value * barHeight);
            
            QColor barColor = getBandColor(i, spectrumData_.size());
            barColor.setAlpha(180);
            
            QRect barRect(i * barWidth, barHeight - height, barWidth, height);
            painter.fillRect(barRect, barColor);
        }
    }
}

void QtEqualizer::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    // Update spectrum visualization if needed
}

// Slot implementations
void QtEqualizer::onEnableToggled()
{
    setEqualizerEnabled(!equalizerEnabled_);
}

void QtEqualizer::onResetClicked()
{
    resetToDefault();
}

void QtEqualizer::onPresetSelected()
{
    if (presetComboBox_) {
        QString presetName = presetComboBox_->currentText();
        if (presets_.contains(presetName)) {
            loadPreset(presetName);
        }
    }
}

void QtEqualizer::onSavePresetClicked()
{
    bool ok;
    QString presetName = QInputDialog::getText(this, "Save Preset", 
                                              "Preset name:", QLineEdit::Normal, "", &ok);
    if (ok && !presetName.isEmpty()) {
        QString description = QInputDialog::getText(this, "Save Preset", 
                                                  "Description (optional):", QLineEdit::Normal, "", &ok);
        savePreset(presetName, description);
    }
}

void QtEqualizer::onCloseClicked()
{
    hideWithAnimation();
}

void QtEqualizer::onBandGainChanged(int bandIndex, double value)
{
    setBandGain(bandIndex, value);
}

void QtEqualizer::onSearchTextChanged()
{
    QString searchText = searchBox_->text().toLower();
    
    // Filter presets based on search text
    if (presetComboBox_) {
        presetComboBox_->clear();
        for (auto it = presets_.begin(); it != presets_.end(); ++it) {
            if (it->name.toLower().contains(searchText) || 
                it->description.toLower().contains(searchText)) {
                presetComboBox_->addItem(it->name);
            }
        }
    }
}

void QtEqualizer::updateAnimation()
{
    animationTime_ += 0.016f; // Assume 60 FPS
    
    // Subtle animations could be added here
    if (isVisible_ && !isAnimating_) {
        // Could add subtle effects like spectrum bar animations, etc.
    }
}

void QtEqualizer::updateSpectrumVisualization()
{
    if (spectrumCanvas_ && equalizerEnabled_) {
        spectrumCanvas_->update();
    }
}

void QtEqualizer::show()
{
    isVisible_ = true;
    QDialog::show();
    showWithAnimation();
}

void QtEqualizer::hide()
{
    hideWithAnimation();
}

void QtEqualizer::toggle()
{
    if (isVisible_) {
        hide();
    } else {
        show();
    }
}

// Preset file I/O methods
void QtEqualizer::loadPresetsFromFile()
{
    QString presetsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/presets.json";
    QFile file(presetsPath);
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject rootObj = doc.object();
        
        if (rootObj.contains("presets")) {
            QJsonArray presetsArray = rootObj["presets"].toArray();
            
            for (const QJsonValue& value : presetsArray) {
                QJsonObject presetObj = value.toObject();
                
                EqualizerPreset preset;
                preset.name = presetObj["name"].toString();
                preset.description = presetObj["description"].toString();
                
                QJsonArray gainsArray = presetObj["bandGains"].toArray();
                preset.bandGains.resize(gainsArray.size());
                
                for (int i = 0; i < gainsArray.size(); ++i) {
                    preset.bandGains[i] = gainsArray[i].toDouble();
                }
                
                presets_[preset.name] = preset;
            }
        }
    }
}

void QtEqualizer::savePresetsToFile()
{
    QString presetsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/presets.json";
    QDir().mkpath(QFileInfo(presetsPath).absolutePath());
    
    QFile file(presetsPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject rootObj;
        QJsonArray presetsArray;
        
        for (auto it = presets_.begin(); it != presets_.end(); ++it) {
            QJsonObject presetObj;
            presetObj["name"] = it->name;
            presetObj["description"] = it->description;
            
            QJsonArray gainsArray;
            for (float gain : it->bandGains) {
                gainsArray.append(gain);
            }
            presetObj["bandGains"] = gainsArray;
            
            presetsArray.append(presetObj);
        }
        
        rootObj["presets"] = presetsArray;
        
        QJsonDocument doc(rootObj);
        file.write(doc.toJson());
    }
}