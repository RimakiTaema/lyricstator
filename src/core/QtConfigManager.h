#pragma once
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QMap>

namespace Lyricstator {

class QtConfigManager : public QObject {
    Q_OBJECT

public:
    QtConfigManager(QObject* parent = nullptr);
    ~QtConfigManager();

    // Configuration management
    bool LoadConfig(const QString& filepath = QString());
    bool SaveConfig(const QString& filepath = QString());
    
    // Value access
    QVariant GetValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void SetValue(const QString& key, const QVariant& value);
    
    // Section management
    void BeginGroup(const QString& group);
    void EndGroup();
    
    // Utility methods
    QString GetConfigPath() const;
    void ResetToDefaults();
    bool HasKey(const QString& key) const;
    
    // Common configuration keys
    static const QString CONFIG_AUDIO_DEVICE;
    static const QString CONFIG_AUDIO_SAMPLE_RATE;
    static const QString CONFIG_AUDIO_CHANNELS;
    static const QString CONFIG_AUDIO_BUFFER_SIZE;
    static const QString CONFIG_AUDIO_VOLUME;
    static const QString CONFIG_DISPLAY_WIDTH;
    static const QString CONFIG_DISPLAY_HEIGHT;
    static const QString CONFIG_DISPLAY_FULLSCREEN;
    static const QString CONFIG_DISPLAY_THEME;
    static const QString CONFIG_MIDI_ENABLED;
    static const QString CONFIG_PITCH_DETECTION_ENABLED;
    static const QString CONFIG_LANGUAGE;
    static const QString CONFIG_AUTOSAVE_ENABLED;
    static const QString CONFIG_AUTOSAVE_INTERVAL;

private:
    QSettings* settings_;
    QString currentGroup_;
    QMap<QString, QVariant> defaults_;
    
    void InitializeDefaults();
    void LoadDefaults();
};

} // namespace Lyricstator