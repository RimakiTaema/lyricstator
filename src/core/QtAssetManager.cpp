#include "QtAssetManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

QtAssetManager& QtAssetManager::getInstance() {
    static QtAssetManager instance;
    return instance;
}

QtAssetManager::QtAssetManager(QObject* parent)
    : QObject(parent)
    , assetBasePath_()
    , activeResourcePack_("default")
{
    initializeAssetPath();
    loadDefaultResourcePack();
}

QtAssetManager::~QtAssetManager() {
    cleanup();
}

bool QtAssetManager::loadResourcePack(const QString& packPath) {
    QFileInfo fileInfo(packPath);
    if (!fileInfo.exists() || !fileInfo.isDir()) {
        qDebug() << "Resource pack path does not exist:" << packPath;
        return false;
    }
    
    try {
        QtResourcePack pack = parseResourcePackConfig(packPath + "/config.json");
        if (pack.name.isEmpty()) {
            pack.name = fileInfo.fileName();
        }
        pack.path = packPath;
        
        resourcePacks_[pack.name] = pack;
        emit resourcePackLoaded(pack.name);
        
        qDebug() << "Resource pack loaded:" << pack.name;
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Failed to load resource pack:" << e.what();
        return false;
    }
}

void QtAssetManager::setActiveResourcePack(const QString& packName) {
    if (resourcePacks_.contains(packName)) {
        activeResourcePack_ = packName;
        emit resourcePackChanged(packName);
        qDebug() << "Active resource pack changed to:" << packName;
    } else {
        qDebug() << "Resource pack not found:" << packName;
    }
}

QVector<QtResourcePack> QtAssetManager::getAvailableResourcePacks() {
    QVector<QtResourcePack> packs;
    for (auto it = resourcePacks_.begin(); it != resourcePacks_.end(); ++it) {
        packs.append(it.value());
    }
    return packs;
}

const QtResourcePack& QtAssetManager::getActiveResourcePack() const {
    static QtResourcePack defaultPack;
    auto it = resourcePacks_.find(activeResourcePack_);
    if (it != resourcePacks_.end()) {
        return it.value();
    }
    return defaultPack;
}

QFont QtAssetManager::loadFont(const QString& name, const QString& path, int size) {
    QString fontPath = getAssetPath(path);
    QFont font;
    
    if (QFile::exists(fontPath)) {
        int fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1) {
            QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            if (!fontFamilies.isEmpty()) {
                font = QFont(fontFamilies.first(), size);
                fonts_[name] = font;
                qDebug() << "Font loaded:" << name << "from" << fontPath;
            }
        }
    } else {
        qDebug() << "Font file not found:" << fontPath;
    }
    
    return font;
}

QFont QtAssetManager::getFont(const QString& name) {
    if (fonts_.contains(name)) {
        return fonts_[name];
    }
    
    // Return default font if not found
    QFont defaultFont;
    defaultFont.setPointSize(12);
    return defaultFont;
}

QFont QtAssetManager::getThemeFont(const QString& fontType) {
    const QtResourcePack& pack = getActiveResourcePack();
    QString fontPath;
    int fontSize = 12;
    
    if (fontType == "lyrics") {
        fontPath = pack.fonts.lyrics;
        fontSize = pack.fonts.lyricsSize;
    } else if (fontType == "ui") {
        fontPath = pack.fonts.ui;
        fontSize = pack.fonts.uiSize;
    } else if (fontType == "bold") {
        fontPath = pack.fonts.bold;
        fontSize = pack.fonts.boldSize;
    }
    
    if (!fontPath.isEmpty()) {
        return loadFont(fontType, fontPath, fontSize);
    }
    
    return getFont(fontType);
}

QPixmap QtAssetManager::loadImage(const QString& name, const QString& path) {
    QString imagePath = getAssetPath(path);
    QPixmap pixmap;
    
    if (pixmap.load(imagePath)) {
        images_[name] = pixmap;
        qDebug() << "Image loaded:" << name << "from" << imagePath;
    } else {
        qDebug() << "Failed to load image:" << imagePath;
    }
    
    return pixmap;
}

QPixmap QtAssetManager::getImage(const QString& name) {
    if (images_.contains(name)) {
        return images_[name];
    }
    
    // Return empty pixmap if not found
    return QPixmap();
}

QPixmap QtAssetManager::getThemeImage(const QString& imageType) {
    const QtResourcePack& pack = getActiveResourcePack();
    QString imagePath;
    
    if (imageType == "background") {
        imagePath = pack.images.background;
    } else if (imageType == "noteIndicator") {
        imagePath = pack.images.noteIndicator;
    } else if (imageType == "progressBar") {
        imagePath = pack.images.progressBar;
    }
    
    if (!imagePath.isEmpty()) {
        return loadImage(imageType, imagePath);
    }
    
    return getImage(imageType);
}

QColor QtAssetManager::getThemeColor(const QString& colorType) {
    const QtResourcePack& pack = getActiveResourcePack();
    
    if (colorType == "background") {
        return pack.colors.background;
    } else if (colorType == "text") {
        return pack.colors.text;
    } else if (colorType == "accent") {
        return pack.colors.accent;
    } else if (colorType == "highlight") {
        return pack.colors.highlight;
    } else if (colorType == "progress") {
        return pack.colors.progress;
    }
    
    // Return default color if not found
    return QColor(255, 255, 255, 255);
}

QString QtAssetManager::getAssetPath(const QString& relativePath) {
    if (QDir::isAbsolutePath(relativePath)) {
        return relativePath;
    }
    
    QString appDir = QApplication::applicationDirPath();
    QString assetPath = appDir + "/assets/" + relativePath;
    
    if (QFile::exists(assetPath)) {
        return assetPath;
    }
    
    // Try resource path
    QString resourcePath = ":/assets/" + relativePath;
    if (QFile::exists(resourcePath)) {
        return resourcePath;
    }
    
    return relativePath;
}

QString QtAssetManager::getResourcePackPath(const QString& packName, const QString& relativePath) {
    auto it = resourcePacks_.find(packName);
    if (it != resourcePacks_.end()) {
        return it->path + "/" + relativePath;
    }
    return getAssetPath(relativePath);
}

void QtAssetManager::cleanup() {
    fonts_.clear();
    images_.clear();
    qDebug() << "AssetManager cleanup completed";
}

void QtAssetManager::initializeAssetPath() {
    QString appDir = QApplication::applicationDirPath();
    assetBasePath_ = appDir + "/assets";
    
    QDir assetDir(assetBasePath_);
    if (!assetDir.exists()) {
        assetDir.mkpath(".");
    }
    
    qDebug() << "Asset base path:" << assetBasePath_;
}

void QtAssetManager::loadDefaultResourcePack() {
    // Create default resource pack
    QtResourcePack defaultPack;
    defaultPack.name = "default";
    defaultPack.displayName = "Default Theme";
    defaultPack.author = "Lyricstator";
    defaultPack.version = "1.0.0";
    defaultPack.description = "Default resource pack with basic styling";
    
    resourcePacks_["default"] = defaultPack;
    qDebug() << "Default resource pack loaded";
}

QtResourcePack QtAssetManager::parseResourcePackConfig(const QString& configPath) {
    QtResourcePack pack;
    
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open resource pack config:" << configPath;
        return pack;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Failed to parse resource pack config:" << error.errorString();
        return pack;
    }
    
    QJsonObject root = doc.object();
    
    // Parse basic info
    pack.name = root["name"].toString();
    pack.displayName = root["displayName"].toString();
    pack.author = root["author"].toString();
    pack.version = root["version"].toString();
    pack.description = root["description"].toString();
    
    // Parse colors
    if (root.contains("colors")) {
        QJsonObject colors = root["colors"].toObject();
        pack.colors.background = QColor(colors["background"].toString());
        pack.colors.text = QColor(colors["text"].toString());
        pack.colors.accent = QColor(colors["accent"].toString());
        pack.colors.highlight = QColor(colors["highlight"].toString());
        pack.colors.progress = QColor(colors["progress"].toString());
    }
    
    // Parse fonts
    if (root.contains("fonts")) {
        QJsonObject fonts = root["fonts"].toObject();
        pack.fonts.lyrics = fonts["lyrics"].toString();
        pack.fonts.ui = fonts["ui"].toString();
        pack.fonts.bold = fonts["bold"].toString();
        pack.fonts.lyricsSize = fonts["lyricsSize"].toInt(32);
        pack.fonts.uiSize = fonts["uiSize"].toInt(16);
        pack.fonts.boldSize = fonts["boldSize"].toInt(20);
    }
    
    // Parse images
    if (root.contains("images")) {
        QJsonObject images = root["images"].toObject();
        pack.images.background = images["background"].toString();
        pack.images.noteIndicator = images["noteIndicator"].toString();
        pack.images.progressBar = images["progressBar"].toString();
    }
    
    return pack;
}