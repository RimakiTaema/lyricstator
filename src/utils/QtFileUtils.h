#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>

namespace Lyricstator {

class QtFileUtils : public QObject {
    Q_OBJECT

public:
    static QtFileUtils& getInstance();
    
    // File existence and validation
    static bool fileExists(const QString& filepath);
    static bool isDirectory(const QString& path);
    static bool isFile(const QString& path);
    
    // File path manipulation
    static QString getFileExtension(const QString& filepath);
    static QString getFileName(const QString& filepath);
    static QString getBaseName(const QString& filepath);
    static QString getDirectory(const QString& filepath);
    static QString getAbsolutePath(const QString& filepath);
    static QString getRelativePath(const QString& filepath, const QString& basePath);
    
    // File listing and discovery
    static QStringList listFiles(const QString& directory, const QString& extension = "");
    static QStringList listDirectories(const QString& directory);
    static QStringList findFilesRecursive(const QString& directory, const QString& extension = "");
    
    // File type detection
    static bool isAudioFile(const QString& filepath);
    static bool isMidiFile(const QString& filepath);
    static bool isLyricFile(const QString& filepath);
    static bool isImageFile(const QString& filepath);
    static bool isTextFile(const QString& filepath);
    
    // File operations
    static bool copyFile(const QString& source, const QString& destination);
    static bool moveFile(const QString& source, const QString& destination);
    static bool deleteFile(const QString& filepath);
    static bool createDirectory(const QString& path);
    static bool createDirectories(const QString& path);
    
    // File information
    static qint64 getFileSize(const QString& filepath);
    static QDateTime getFileModifiedTime(const QString& filepath);
    static QDateTime getFileCreatedTime(const QString& filepath);
    static QString getFilePermissions(const QString& filepath);
    
    // Path utilities
    static QString combinePaths(const QString& path1, const QString& path2);
    static QString normalizePath(const QString& path);
    static QString getCurrentWorkingDirectory();
    static QString getHomeDirectory();
    static QString getTempDirectory();
    
    // File format support
    static QStringList getSupportedAudioFormats();
    static QStringList getSupportedMidiFormats();
    static QStringList getSupportedLyricFormats();
    static QStringList getSupportedImageFormats();
    
    // Error handling
    static QString getLastError();
    static void clearLastError();

private:
    QtFileUtils(QObject* parent = nullptr);
    ~QtFileUtils();
    
    static QString lastError_;
    
    // Helper methods
    static void setLastError(const QString& error);
    static bool isValidPath(const QString& path);
    static QString sanitizePath(const QString& path);
};

// Global instance
#define FileUtils QtFileUtils::getInstance()

} // namespace Lyricstator