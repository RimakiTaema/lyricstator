#include "QtLogger.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QDateTime>

namespace Lyricstator {

QtLogger& QtLogger::getInstance() {
    static QtLogger instance;
    return instance;
}

QtLogger::QtLogger(QObject* parent)
    : QObject(parent)
    , currentLevel_(LogLevel::Info)
    , logToFile_(true)
    , logToConsole_(true)
    , logFilePath_()
    , timestampFormat_("yyyy-MM-dd hh:mm:ss.zzz")
    , logFile_(nullptr)
    , logStream_(nullptr)
{
    // Set default log file path
    QString appName = QApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "Lyricstator";
    }
    
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);
    logFilePath_ = logDir + "/" + appName.toLower() + ".log";
    
    initializeLogFile();
}

QtLogger::~QtLogger() {
    closeLogFile();
}

void QtLogger::debug(const QString& message) {
    log(LogLevel::Debug, message);
}

void QtLogger::info(const QString& message) {
    log(LogLevel::Info, message);
}

void QtLogger::warning(const QString& message) {
    log(LogLevel::Warning, message);
}

void QtLogger::error(const QString& message) {
    log(LogLevel::Error, message);
}

void QtLogger::critical(const QString& message) {
    log(LogLevel::Critical, message);
}

void QtLogger::log(LogLevel level, const QString& message) {
    if (level < currentLevel_) {
        return;
    }
    
    QString formattedMessage = formatMessage(level, message);
    QString timestamp = getCurrentTimestamp();
    
    // Write to console
    if (logToConsole_) {
        writeToConsole(formattedMessage);
    }
    
    // Write to file
    if (logToFile_) {
        writeToFile(formattedMessage);
    }
    
    // Emit signal
    emit logMessage(level, message, timestamp);
}

void QtLogger::setLogLevel(LogLevel level) {
    currentLevel_ = level;
}

LogLevel QtLogger::getLogLevel() const {
    return currentLevel_;
}

void QtLogger::setLogToFile(bool enabled) {
    logToFile_ = enabled;
    if (enabled && !logFile_) {
        initializeLogFile();
    } else if (!enabled && logFile_) {
        closeLogFile();
    }
}

bool QtLogger::isLogToFileEnabled() const {
    return logToFile_;
}

void QtLogger::setLogFilePath(const QString& filepath) {
    if (logFilePath_ != filepath) {
        closeLogFile();
        logFilePath_ = filepath;
        if (logToFile_) {
            initializeLogFile();
        }
    }
}

QString QtLogger::getLogFilePath() const {
    return logFilePath_;
}

void QtLogger::setLogToConsole(bool enabled) {
    logToConsole_ = enabled;
}

bool QtLogger::isLogToConsoleEnabled() const {
    return logToConsole_;
}

void QtLogger::setTimestampFormat(const QString& format) {
    timestampFormat_ = format;
}

QString QtLogger::getTimestampFormat() const {
    return timestampFormat_;
}

QString QtLogger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

LogLevel QtLogger::stringToLevel(const QString& levelString) const {
    QString level = levelString.toUpper();
    if (level == "DEBUG") return LogLevel::Debug;
    if (level == "INFO") return LogLevel::Info;
    if (level == "WARNING") return LogLevel::Warning;
    if (level == "ERROR") return LogLevel::Error;
    if (level == "CRITICAL") return LogLevel::Critical;
    return LogLevel::Info; // Default
}

void QtLogger::rotateLogFile() {
    if (!logFile_) {
        return;
    }
    
    closeLogFile();
    
    // Create backup with timestamp
    QString backupPath = logFilePath_ + "." + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QFile::copy(logFilePath_, backupPath);
    
    initializeLogFile();
}

void QtLogger::clearLogFile() {
    closeLogFile();
    
    // Create new empty file
    QFile file(logFilePath_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.close();
    }
    
    if (logToFile_) {
        initializeLogFile();
    }
}

QString QtLogger::getLogFileContents() const {
    QFile file(logFilePath_);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString contents = stream.readAll();
        file.close();
        return contents;
    }
    return QString();
}

void QtLogger::writeToFile(const QString& message) {
    QMutexLocker locker(&logMutex_);
    
    if (!logFile_ || !logStream_) {
        return;
    }
    
    *logStream_ << message << Qt::endl;
    logStream_->flush();
}

void QtLogger::writeToConsole(const QString& message) {
    QMutexLocker locker(&logMutex_);
    
    // Use Qt's debug output
    qDebug().noquote() << message;
}

QString QtLogger::formatMessage(LogLevel level, const QString& message) {
    QString timestamp = getCurrentTimestamp();
    QString levelStr = levelToString(level);
    
    return QString("[%1] [%2] %3")
        .arg(timestamp)
        .arg(levelStr)
        .arg(message);
}

QString QtLogger::getCurrentTimestamp() const {
    return QDateTime::currentDateTime().toString(timestampFormat_);
}

void QtLogger::initializeLogFile() {
    if (logFilePath_.isEmpty()) {
        return;
    }
    
    // Ensure directory exists
    QFileInfo fileInfo(logFilePath_);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Open log file
    logFile_ = new QFile(logFilePath_);
    if (logFile_->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logStream_ = new QTextStream(logFile_);
        logStream_->setCodec("UTF-8");
        
        // Write header if file is empty
        if (logFile_->size() == 0) {
            *logStream_ << "=== Lyricstator Log Started ===" << Qt::endl;
            *logStream_ << "Timestamp: " << getCurrentTimestamp() << Qt::endl;
            *logStream_ << "===============================" << Qt::endl;
            logStream_->flush();
        }
    } else {
        qDebug() << "Failed to open log file:" << logFilePath_;
        delete logFile_;
        logFile_ = nullptr;
        logStream_ = nullptr;
    }
}

void QtLogger::closeLogFile() {
    if (logStream_) {
        delete logStream_;
        logStream_ = nullptr;
    }
    
    if (logFile_) {
        logFile_->close();
        delete logFile_;
        logFile_ = nullptr;
    }
}

// Static convenience methods
void QtLogger::staticDebug(const QString& message) {
    getInstance().debug(message);
}

void QtLogger::staticInfo(const QString& message) {
    getInstance().info(message);
}

void QtLogger::staticWarning(const QString& message) {
    getInstance().warning(message);
}

void QtLogger::staticError(const QString& message) {
    getInstance().error(message);
}

void QtLogger::staticCritical(const QString& message) {
    getInstance().critical(message);
}

} // namespace Lyricstator