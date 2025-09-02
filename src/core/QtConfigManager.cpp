#include "QtConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QApplication>

namespace Lyricstator {

// Static constants
const QString QtConfigManager::CONFIG_AUDIO_DEVICE = "audio/device";
const QString QtConfigManager::CONFIG_AUDIO_SAMPLE_RATE = "audio/sample_rate";
const QString QtConfigManager::CONFIG_AUDIO_CHANNELS = "audio/channels";
const QString QtConfigManager::CONFIG_AUDIO_BUFFER_SIZE = "audio/buffer_size";
const QString QtConfigManager::CONFIG_AUDIO_VOLUME = "audio/volume";
const QString QtConfigManager::CONFIG_DISPLAY_WIDTH = "display/width";
const QString QtConfigManager::CONFIG_DISPLAY_HEIGHT = "display/height";
const QString QtConfigManager::CONFIG_DISPLAY_FULLSCREEN = "display/fullscreen";
const QString QtConfigManager::CONFIG_DISPLAY_THEME = "display/theme";
const QString QtConfigManager::CONFIG_MIDI_ENABLED = "midi/enabled";
const QString QtConfigManager::CONFIG_PITCH_DETECTION_ENABLED = "pitch_detection/enabled";
const QString QtConfigManager::CONFIG_LANGUAGE = "general/language";
const QString QtConfigManager::CONFIG_AUTOSAVE_ENABLED = "general/autosave_enabled";
const QString QtConfigManager::CONFIG_AUTOSAVE_INTERVAL = "general/autosave_interval";

QtConfigManager::QtConfigManager(QObject* parent)
    : QObject(parent)
    , settings_(nullptr)
    , currentGroup_()
{
    InitializeDefaults();
    LoadDefaults();
}

QtConfigManager::~QtConfigManager() {
    if (settings_) {
        settings_->sync();
        delete settings_;
    }
}

bool QtConfigManager::LoadConfig(const QString& filepath) {
    QString configPath = filepath;
    if (configPath.isEmpty()) {
        configPath = GetConfigPath();
    }
    
    try {
        if (settings_) {
            delete settings_;
        }
        
        settings_ = new QSettings(configPath, QSettings::IniFormat, this);
        settings_->setIniCodec("UTF-8");
        
        qDebug() << "Configuration loaded from:" << configPath;
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Failed to load configuration:" << e.what();
        return false;
    }
}

bool QtConfigManager::SaveConfig(const QString& filepath) {
    if (!settings_) {
        qDebug() << "No configuration loaded";
        return false;
    }
    
    try {
        settings_->sync();
        qDebug() << "Configuration saved";
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Failed to save configuration:" << e.what();
        return false;
    }
}

QVariant QtConfigManager::GetValue(const QString& key, const QVariant& defaultValue) const {
    if (!settings_) {
        return defaults_.value(key, defaultValue);
    }
    
    QVariant value = settings_->value(key, defaultValue);
    if (!value.isValid()) {
        value = defaults_.value(key, defaultValue);
    }
    
    return value;
}

void QtConfigManager::SetValue(const QString& key, const QVariant& value) {
    if (!settings_) {
        defaults_[key] = value;
        return;
    }
    
    settings_->setValue(key, value);
}

void QtConfigManager::BeginGroup(const QString& group) {
    if (settings_) {
        settings_->beginGroup(group);
    }
    currentGroup_ = group;
}

void QtConfigManager::EndGroup() {
    if (settings_) {
        settings_->endGroup();
    }
    currentGroup_.clear();
}

QString QtConfigManager::GetConfigPath() const {
    QString appName = QApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "Lyricstator";
    }
    
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    return dir.filePath(appName + ".ini");
}

void QtConfigManager::ResetToDefaults() {
    if (settings_) {
        settings_->clear();
    }
    LoadDefaults();
    qDebug() << "Configuration reset to defaults";
}

bool QtConfigManager::HasKey(const QString& key) const {
    if (!settings_) {
        return defaults_.contains(key);
    }
    
    return settings_->contains(key);
}

void QtConfigManager::InitializeDefaults() {
    // Audio defaults
    defaults_[CONFIG_AUDIO_DEVICE] = "default";
    defaults_[CONFIG_AUDIO_SAMPLE_RATE] = 44100;
    defaults_[CONFIG_AUDIO_CHANNELS] = 2;
    defaults_[CONFIG_AUDIO_BUFFER_SIZE] = 1024;
    defaults_[CONFIG_AUDIO_VOLUME] = 1.0;
    
    // Display defaults
    defaults_[CONFIG_DISPLAY_WIDTH] = 1280;
    defaults_[CONFIG_DISPLAY_HEIGHT] = 720;
    defaults_[CONFIG_DISPLAY_FULLSCREEN] = false;
    defaults_[CONFIG_DISPLAY_THEME] = "default";
    
    // Feature defaults
    defaults_[CONFIG_MIDI_ENABLED] = true;
    defaults_[CONFIG_PITCH_DETECTION_ENABLED] = true;
    
    // General defaults
    defaults_[CONFIG_LANGUAGE] = "en";
    defaults_[CONFIG_AUTOSAVE_ENABLED] = true;
    defaults_[CONFIG_AUTOSAVE_INTERVAL] = 300; // 5 minutes
}

void QtConfigManager::LoadDefaults() {
    // Load configuration with defaults
    LoadConfig();
}

} // namespace Lyricstator