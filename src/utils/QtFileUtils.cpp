#include "QtFileUtils.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDirIterator>

namespace Lyricstator {

QString QtFileUtils::lastError_;

QtFileUtils& QtFileUtils::getInstance() {
    static QtFileUtils instance;
    return instance;
}

QtFileUtils::QtFileUtils(QObject* parent)
    : QObject(parent)
{
}

QtFileUtils::~QtFileUtils() {
}

bool QtFileUtils::fileExists(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.exists();
}

bool QtFileUtils::isDirectory(const QString& path) {
    QFileInfo fileInfo(path);
    return fileInfo.isDir();
}

bool QtFileUtils::isFile(const QString& path) {
    QFileInfo fileInfo(path);
    return fileInfo.isFile();
}

QString QtFileUtils::getFileExtension(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.suffix().toLower();
}

QString QtFileUtils::getFileName(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.fileName();
}

QString QtFileUtils::getBaseName(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.baseName();
}

QString QtFileUtils::getDirectory(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.absolutePath();
}

QString QtFileUtils::getAbsolutePath(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.absoluteFilePath();
}

QString QtFileUtils::getRelativePath(const QString& filepath, const QString& basePath) {
    QDir baseDir(basePath);
    return baseDir.relativeFilePath(filepath);
}

QStringList QtFileUtils::listFiles(const QString& directory, const QString& extension) {
    QStringList files;
    QDir dir(directory);
    
    if (!dir.exists()) {
        setLastError(QString("Directory does not exist: %1").arg(directory));
        return files;
    }
    
    if (extension.isEmpty()) {
        files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    } else {
        QString filter = QString("*.%1").arg(extension);
        files = dir.entryList(QStringList() << filter, QDir::Files | QDir::NoDotAndDotDot);
    }
    
    // Convert to absolute paths
    for (int i = 0; i < files.size(); ++i) {
        files[i] = dir.absoluteFilePath(files[i]);
    }
    
    return files;
}

QStringList QtFileUtils::listDirectories(const QString& directory) {
    QStringList dirs;
    QDir dir(directory);
    
    if (!dir.exists()) {
        setLastError(QString("Directory does not exist: %1").arg(directory));
        return dirs;
    }
    
    dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    // Convert to absolute paths
    for (int i = 0; i < dirs.size(); ++i) {
        dirs[i] = dir.absoluteFilePath(dirs[i]);
    }
    
    return dirs;
}

QStringList QtFileUtils::findFilesRecursive(const QString& directory, const QString& extension) {
    QStringList files;
    QDir dir(directory);
    
    if (!dir.exists()) {
        setLastError(QString("Directory does not exist: %1").arg(directory));
        return files;
    }
    
    QDirIterator iterator(directory, 
                         extension.isEmpty() ? QStringList() : QStringList() << QString("*.%1").arg(extension),
                         QDir::Files | QDir::NoDotAndDotDot,
                         QDirIterator::Subdirectories);
    
    while (iterator.hasNext()) {
        files.append(iterator.next());
    }
    
    return files;
}

bool QtFileUtils::isAudioFile(const QString& filepath) {
    QString extension = getFileExtension(filepath);
    QStringList audioFormats = getSupportedAudioFormats();
    return audioFormats.contains(extension);
}

bool QtFileUtils::isMidiFile(const QString& filepath) {
    QString extension = getFileExtension(filepath);
    QStringList midiFormats = getSupportedMidiFormats();
    return midiFormats.contains(extension);
}

bool QtFileUtils::isLyricFile(const QString& filepath) {
    QString extension = getFileExtension(filepath);
    QStringList lyricFormats = getSupportedLyricFormats();
    return lyricFormats.contains(extension);
}

bool QtFileUtils::isImageFile(const QString& filepath) {
    QString extension = getFileExtension(filepath);
    QStringList imageFormats = getSupportedImageFormats();
    return imageFormats.contains(extension);
}

bool QtFileUtils::isTextFile(const QString& filepath) {
    QString extension = getFileExtension(filepath);
    QStringList textFormats = {"txt", "md", "html", "htm", "xml", "json", "csv", "log"};
    return textFormats.contains(extension);
}

bool QtFileUtils::copyFile(const QString& source, const QString& destination) {
    if (!fileExists(source)) {
        setLastError(QString("Source file does not exist: %1").arg(source));
        return false;
    }
    
    QFile sourceFile(source);
    if (!sourceFile.copy(destination)) {
        setLastError(QString("Failed to copy file: %1").arg(sourceFile.errorString()));
        return false;
    }
    
    return true;
}

bool QtFileUtils::moveFile(const QString& source, const QString& destination) {
    if (!fileExists(source)) {
        setLastError(QString("Source file does not exist: %1").arg(source));
        return false;
    }
    
    QFile sourceFile(source);
    if (!sourceFile.rename(destination)) {
        setLastError(QString("Failed to move file: %1").arg(sourceFile.errorString()));
        return false;
    }
    
    return true;
}

bool QtFileUtils::deleteFile(const QString& filepath) {
    if (!fileExists(filepath)) {
        setLastError(QString("File does not exist: %1").arg(filepath));
        return false;
    }
    
    QFile file(filepath);
    if (!file.remove()) {
        setLastError(QString("Failed to delete file: %1").arg(file.errorString()));
        return false;
    }
    
    return true;
}

bool QtFileUtils::createDirectory(const QString& path) {
    QDir dir;
    if (!dir.mkpath(path)) {
        setLastError(QString("Failed to create directory: %1").arg(path));
        return false;
    }
    
    return true;
}

bool QtFileUtils::createDirectories(const QString& path) {
    return createDirectory(path); // mkpath already creates parent directories
}

qint64 QtFileUtils::getFileSize(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.size();
}

QDateTime QtFileUtils::getFileModifiedTime(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.lastModified();
}

QDateTime QtFileUtils::getFileCreatedTime(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    return fileInfo.birthTime();
}

QString QtFileUtils::getFilePermissions(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    QFile::Permissions permissions = fileInfo.permissions();
    
    QString perms;
    perms += (permissions & QFile::ReadOwner) ? "r" : "-";
    perms += (permissions & QFile::WriteOwner) ? "w" : "-";
    perms += (permissions & QFile::ExeOwner) ? "x" : "-";
    perms += (permissions & QFile::ReadGroup) ? "r" : "-";
    perms += (permissions & QFile::WriteGroup) ? "w" : "-";
    perms += (permissions & QFile::ExeGroup) ? "x" : "-";
    perms += (permissions & QFile::ReadOther) ? "r" : "-";
    perms += (permissions & QFile::WriteOther) ? "w" : "-";
    perms += (permissions & QFile::ExeOther) ? "x" : "-";
    
    return perms;
}

QString QtFileUtils::combinePaths(const QString& path1, const QString& path2) {
    QDir dir(path1);
    return dir.filePath(path2);
}

QString QtFileUtils::normalizePath(const QString& path) {
    QDir dir(path);
    return dir.absolutePath();
}

QString QtFileUtils::getCurrentWorkingDirectory() {
    return QDir::currentPath();
}

QString QtFileUtils::getHomeDirectory() {
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString QtFileUtils::getTempDirectory() {
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

QStringList QtFileUtils::getSupportedAudioFormats() {
    return {"mp3", "wav", "ogg", "flac", "aac", "m4a", "wma"};
}

QStringList QtFileUtils::getSupportedMidiFormats() {
    return {"mid", "midi"};
}

QStringList QtFileUtils::getSupportedLyricFormats() {
    return {"lystr", "lrc", "txt", "json"};
}

QStringList QtFileUtils::getSupportedImageFormats() {
    return {"png", "jpg", "jpeg", "bmp", "gif", "svg", "webp"};
}

QString QtFileUtils::getLastError() {
    return lastError_;
}

void QtFileUtils::clearLastError() {
    lastError_.clear();
}

void QtFileUtils::setLastError(const QString& error) {
    lastError_ = error;
    qDebug() << "FileUtils Error:" << error;
}

bool QtFileUtils::isValidPath(const QString& path) {
    QFileInfo fileInfo(path);
    return !fileInfo.filePath().isEmpty();
}

QString QtFileUtils::sanitizePath(const QString& path) {
    QString sanitized = path;
    
    // Replace backslashes with forward slashes on Windows
    #ifdef Q_OS_WIN
    sanitized.replace('\\', '/');
    #endif
    
    // Remove double slashes
    sanitized.replace("//", "/");
    
    // Remove trailing slash
    if (sanitized.endsWith('/') && sanitized.length() > 1) {
        sanitized.chop(1);
    }
    
    return sanitized;
}

} // namespace Lyricstator