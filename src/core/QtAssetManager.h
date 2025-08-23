#pragma once
#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <QFont>
#include <QPixmap>
#include <QColor>
#include <memory>

struct QtResourcePack {
    QString name;
    QString path;
    QString displayName;
    QString author;
    QString version;
    QString description;
    
    // Theme configuration
    struct Colors {
        QColor background = QColor(20, 20, 30, 255);
        QColor text = QColor(255, 255, 255, 255);
        QColor accent = QColor(100, 200, 255, 255);
        QColor highlight = QColor(255, 100, 100, 255);
        QColor progress = QColor(50, 255, 50, 255);
    } colors;
    
    struct Fonts {
        QString lyrics = "lyrics.ttf";
        QString ui = "default.ttf";
        QString bold = "bold.ttf";
        int lyricsSize = 32;
        int uiSize = 16;
        int boldSize = 20;
    } fonts;
    
    struct Images {
        QString background = "background.png";
        QString noteIndicator = "note_indicator.png";
        QString progressBar = "progress_bar.png";
    } images;
};

class QtAssetManager : public QObject {
    Q_OBJECT

public:
    static QtAssetManager& getInstance();
    
    // Resource pack management
    bool loadResourcePack(const QString& packPath);
    void setActiveResourcePack(const QString& packName);
    QVector<QtResourcePack> getAvailableResourcePacks();
    const QtResourcePack& getActiveResourcePack() const;
    
    // Font management
    QFont loadFont(const QString& name, const QString& path, int size);
    QFont getFont(const QString& name);
    QFont getThemeFont(const QString& fontType); // lyrics, ui, bold
    
    // Image management
    QPixmap loadImage(const QString& name, const QString& path);
    QPixmap getImage(const QString& name);
    QPixmap getThemeImage(const QString& imageType);
    
    // Color management
    QColor getThemeColor(const QString& colorType);
    
    // Asset path helpers
    QString getAssetPath(const QString& relativePath);
    QString getResourcePackPath(const QString& packName, const QString& relativePath);
    
    void cleanup();

signals:
    void resourcePackLoaded(const QString& packName);
    void resourcePackChanged(const QString& packName);

private:
    QtAssetManager(QObject* parent = nullptr);
    ~QtAssetManager();
    
    QMap<QString, QFont> fonts_;
    QMap<QString, QPixmap> images_;
    QMap<QString, QtResourcePack> resourcePacks_;
    
    QString assetBasePath_;
    QString activeResourcePack_ = "default";
    
    void initializeAssetPath();
    void loadDefaultResourcePack();
    QtResourcePack parseResourcePackConfig(const QString& configPath);
};

// Global instance
#define AssetManager QtAssetManager::getInstance()