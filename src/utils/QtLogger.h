#pragma once
#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

namespace Lyricstator {

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
};

class QtLogger : public QObject {
    Q_OBJECT

public:
    static QtLogger& getInstance();
    
    // Logging methods
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void critical(const QString& message);
    
    // Log with custom level
    void log(LogLevel level, const QString& message);
    
    // Configuration
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    
    void setLogToFile(bool enabled);
    bool isLogToFileEnabled() const;
    
    void setLogFilePath(const QString& filepath);
    QString getLogFilePath() const;
    
    void setLogToConsole(bool enabled);
    bool isLogToConsoleEnabled() const;
    
    void setTimestampFormat(const QString& format);
    QString getTimestampFormat() const;
    
    // Utility methods
    QString levelToString(LogLevel level) const;
    LogLevel stringToLevel(const QString& levelString) const;
    
    // File management
    void rotateLogFile();
    void clearLogFile();
    QString getLogFileContents() const;

signals:
    void logMessage(LogLevel level, const QString& message, const QString& timestamp);

private:
    QtLogger(QObject* parent = nullptr);
    ~QtLogger();
    
    // Logging state
    LogLevel currentLevel_;
    bool logToFile_;
    bool logToConsole_;
    QString logFilePath_;
    QString timestampFormat_;
    
    // File handling
    QFile* logFile_;
    QTextStream* logStream_;
    mutable QMutex logMutex_;
    
    // Private methods
    void writeToFile(const QString& message);
    void writeToConsole(const QString& message);
    QString formatMessage(LogLevel level, const QString& message);
    QString getCurrentTimestamp() const;
    void initializeLogFile();
    void closeLogFile();
    
    // Static convenience methods
    static void staticDebug(const QString& message);
    static void staticInfo(const QString& message);
    static void staticWarning(const QString& message);
    static void staticError(const QString& message);
    static void staticCritical(const QString& message);
};

// Global instance
#define Logger QtLogger::getInstance()

// Convenience macros
#define LOG_DEBUG(msg) QtLogger::staticDebug(msg)
#define LOG_INFO(msg) QtLogger::staticInfo(msg)
#define LOG_WARNING(msg) QtLogger::staticWarning(msg)
#define LOG_ERROR(msg) QtLogger::staticError(msg)
#define LOG_CRITICAL(msg) QtLogger::staticCritical(msg)

} // namespace Lyricstator