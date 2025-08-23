#include "QtSettingsManager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Lyricstator {

QtSettingsManager& QtSettingsManager::getInstance() {
    static QtSettingsManager instance;
    return instance;
}

QtSettingsManager::QtSettingsManager(QObject* parent)
    : QObject(parent)
    , changeCallback_(nullptr)
{
    initializeDefaults();
    loadKeyBindings();
}

QtSettingsManager::~QtSettingsManager() {
    saveSettings();
}

bool QtSettingsManager::loadSettings(const QString& filepath) {
    QString settingsPath = filepath;
    if (settingsPath.isEmpty()) {
        QString appName = QApplication::applicationName();
        if (appName.isEmpty()) {
            appName = "Lyricstator";
        }
        
        QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir dir(configDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        settingsPath = dir.filePath(appName + "_settings.ini");
    }
    
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    
    try {
        // Load audio settings
        settings.beginGroup("Audio");
        audioSettings_.masterVolume = settings.value("masterVolume", 1.0).toFloat();
        audioSettings_.sampleRate = settings.value("sampleRate", 44100).toInt();
        audioSettings_.bufferSize = settings.value("bufferSize", 1024).toInt();
        audioSettings_.enableEqualizer = settings.value("enableEqualizer", true).toBool();
        audioSettings_.equalizerBandCount = settings.value("equalizerBandCount", 12).toInt();
        settings.endGroup();
        
        // Load directory settings
        settings.beginGroup("Directories");
        directorySettings_.songDirectories = settings.value("songDirectories").toStringList();
        directorySettings_.defaultExportPath = settings.value("defaultExportPath").toString();
        directorySettings_.resourcePackPath = settings.value("resourcePackPath").toString();
        directorySettings_.recursiveSearch = settings.value("recursiveSearch", true).toBool();
        settings.endGroup();
        
        // Load UI settings
        settings.beginGroup("UI");
        uiSettings_.theme = settings.value("theme", "default").toString();
        uiSettings_.fontSize = settings.value("fontSize", 16).toInt();
        uiSettings_.showSpectrum = settings.value("showSpectrum", true).toBool();
        uiSettings_.showPitchDetection = settings.value("showPitchDetection", true).toBool();
        uiSettings_.fullscreen = settings.value("fullscreen", false).toBool();
        uiSettings_.windowWidth = settings.value("windowWidth", 1280).toInt();
        uiSettings_.windowHeight = settings.value("windowHeight", 720).toInt();
        settings.endGroup();
        
        // Load karaoke settings
        settings.beginGroup("Karaoke");
        karaokeSettings_.enablePitchDetection = settings.value("enablePitchDetection", true).toBool();
        karaokeSettings_.pitchSensitivity = settings.value("pitchSensitivity", 0.7).toFloat();
        karaokeSettings_.showNoteIndicators = settings.value("showNoteIndicators", true).toBool();
        karaokeSettings_.enableScoring = settings.value("enableScoring", true).toBool();
        karaokeSettings_.lyricFadeTime = settings.value("lyricFadeTime", 0.5).toFloat();
        settings.endGroup();
        
        qDebug() << "Settings loaded from:" << settingsPath;
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "Failed to load settings:" << e.what();
        return false;
    }
}

bool QtSettingsManager::saveSettings(const QString& filepath) const {
    QString settingsPath = filepath;
    if (settingsPath.isEmpty()) {
        QString appName = QApplication::applicationName();
        if (appName.isEmpty()) {
            appName = "Lyricstator";
        }
        
        QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir dir(configDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        settingsPath = dir.filePath(appName + "_settings.ini");
    }
    
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    
    try {
        // Save audio settings
        settings.beginGroup("Audio");
        settings.setValue("masterVolume", audioSettings_.masterVolume);
        settings.setValue("sampleRate", audioSettings_.sampleRate);
        settings.setValue("bufferSize", audioSettings_.bufferSize);
        settings.setValue("enableEqualizer", audioSettings_.enableEqualizer);
        settings.setValue("equalizerBandCount", audioSettings_.equalizerBandCount);
        settings.endGroup();
        
        // Save directory settings
        settings.beginGroup("Directories");
        settings.setValue("songDirectories", directorySettings_.songDirectories);
        settings.setValue("defaultExportPath", directorySettings_.defaultExportPath);
        settings.setValue("resourcePackPath", directorySettings_.resourcePackPath);
        settings.setValue("recursiveSearch", directorySettings_.recursiveSearch);
        settings.endGroup();
        
        // Save UI settings
        settings.beginGroup("UI");
        settings.setValue("theme", uiSettings_.theme);
        settings.setValue("fontSize", uiSettings_.fontSize);
        settings.setValue("showSpectrum", uiSettings_.showSpectrum);
        settings.setValue("showPitchDetection", uiSettings_.showPitchDetection);
        settings.setValue("fullscreen", uiSettings_.fullscreen);
        settings.setValue("windowWidth", uiSettings_.windowWidth);
        settings.setValue("windowHeight", uiSettings_.windowHeight);
        settings.endGroup();
        
        // Save karaoke settings
        settings.beginGroup("Karaoke");
        settings.setValue("enablePitchDetection", karaokeSettings_.enablePitchDetection);
        settings.setValue("pitchSensitivity", karaokeSettings_.pitchSensitivity);
        settings.setValue("showNoteIndicators", karaokeSettings_.showNoteIndicators);
        settings.setValue("enableScoring", karaokeSettings_.enableScoring);
        settings.setValue("lyricFadeTime", karaokeSettings_.lyricFadeTime);
        settings.endGroup();
        
        settings.sync();
        qDebug() << "Settings saved to:" << settingsPath;
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "Failed to save settings:" << e.what();
        return false;
    }
}

void QtSettingsManager::resetToDefaults() {
    initializeDefaults();
    emitChangeSignal("all");
    qDebug() << "Settings reset to defaults";
}

void QtSettingsManager::setEqualizerBandCount(int count) {
    audioSettings_.equalizerBandCount = count;
    audioSettings_.equalizerBands.resize(count);
    
    // Initialize default frequencies
    for (int i = 0; i < count; ++i) {
        audioSettings_.equalizerBands[i].frequency = 31.25f * (1 << i); // Logarithmic spacing
        audioSettings_.equalizerBands[i].gain = 1.0f;
        audioSettings_.equalizerBands[i].enabled = true;
    }
    
    emitChangeSignal("audio");
    emit audioSettingsChanged();
}

void QtSettingsManager::setEqualizerBand(int index, float frequency, float gain) {
    if (index >= 0 && index < audioSettings_.equalizerBands.size()) {
        audioSettings_.equalizerBands[index].frequency = frequency;
        audioSettings_.equalizerBands[index].gain = gain;
        emitChangeSignal("audio");
        emit audioSettingsChanged();
    }
}

void QtSettingsManager::addSongDirectory(const QString& path) {
    if (!directorySettings_.songDirectories.contains(path)) {
        directorySettings_.songDirectories.append(path);
        emitChangeSignal("directories");
    }
}

void QtSettingsManager::removeSongDirectory(const QString& path) {
    directorySettings_.songDirectories.removeOne(path);
    emitChangeSignal("directories");
}

void QtSettingsManager::setKeyBinding(const QString& action, const QKeySequence& key, bool ctrl, bool shift, bool alt) {
    // Remove existing binding for this action
    for (int i = 0; i < keyBindings_.size(); ++i) {
        if (keyBindings_[i].action == action) {
            keyBindings_.removeAt(i);
            break;
        }
    }
    
    // Add new binding
    QtKeyBinding binding;
    binding.key = key;
    binding.ctrl = ctrl;
    binding.shift = shift;
    binding.alt = alt;
    binding.action = action;
    binding.description = action; // TODO: Add proper descriptions
    
    keyBindings_.append(binding);
    saveKeyBindings();
    emitChangeSignal("keybindings");
    emit keyBindingsChanged();
}

QtKeyBinding* QtSettingsManager::findKeyBinding(const QKeySequence& key, bool ctrl, bool shift, bool alt) {
    for (int i = 0; i < keyBindings_.size(); ++i) {
        if (keyBindings_[i].key == key &&
            keyBindings_[i].ctrl == ctrl &&
            keyBindings_[i].shift == shift &&
            keyBindings_[i].alt == alt) {
            return &keyBindings_[i];
        }
    }
    return nullptr;
}

QString QtSettingsManager::getKeyBindingString(const QtKeyBinding& binding) const {
    QString modifiers;
    if (binding.ctrl) modifiers += "Ctrl+";
    if (binding.shift) modifiers += "Shift+";
    if (binding.alt) modifiers += "Alt+";
    
    return modifiers + binding.key.toString();
}

void QtSettingsManager::initializeDefaults() {
    // Initialize equalizer bands
    setEqualizerBandCount(12);
    
    // Set default directories
    QStringList defaultDirs;
    defaultDirs << QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    defaultDirs << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Lyricstator";
    directorySettings_.songDirectories = defaultDirs;
    
    // Set default export path
    directorySettings_.defaultExportPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Lyricstator/Exports";
    
    // Set default resource pack path
    directorySettings_.resourcePackPath = QApplication::applicationDirPath() + "/assets/resource_packs";
    
    // Initialize default key bindings
    keyBindings_.clear();
    
    QtKeyBinding binding;
    binding.action = "play_pause";
    binding.key = QKeySequence(Qt::Key_Space);
    binding.description = "Play/Pause";
    keyBindings_.append(binding);
    
    binding.action = "stop";
    binding.key = QKeySequence(Qt::Key_Escape);
    binding.description = "Stop";
    keyBindings_.append(binding);
    
    binding.action = "quit";
    binding.key = QKeySequence("Ctrl+Q");
    binding.ctrl = true;
    binding.description = "Quit Application";
    keyBindings_.append(binding);
}

void QtSettingsManager::loadKeyBindings() {
    // Key bindings are loaded from settings file
    // This method can be extended to load from custom key binding files
}

void QtSettingsManager::saveKeyBindings() {
    // Key bindings are saved with other settings
    // This method can be extended to save to custom key binding files
}

void QtSettingsManager::emitChangeSignal(const QString& section) {
    emit settingsChanged(section);
    
    if (changeCallback_) {
        changeCallback_(section);
    }
}

} // namespace Lyricstator