#ifndef QTSONGBROWSER_H
#define QTSONGBROWSER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSplitter>
#include <QGroupBox>
#include <QComboBox>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QTimer>
#include <QStringList>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "core/Application.h"

struct SongInfo {
    QString title;
    QString artist;
    QString album;
    QString filePath;
    QString extension;
    qint64 fileSize;
    int duration; // in seconds
    bool hasLyrics;
    bool hasMidi;
};

class QtSongBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit QtSongBrowser(QWidget *parent = nullptr);
    ~QtSongBrowser();

    void setApplication(Lyricstator::Application* app);
    void addSongDirectory(const QString& directory);
    void removeSongDirectory(const QString& directory);
    void refreshSongList();
    void setVisible(bool visible);

signals:
    void songSelected(const QString& filePath);
    void playlistChanged();

public slots:
    void show();
    void hide();
    void toggle();

private slots:
    void onSearchTextChanged();
    void onSongItemClicked(QTreeWidgetItem* item, int column);
    void onSongItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onDirectoryItemClicked(QTreeWidgetItem* item, int column);
    void onAddDirectoryClicked();
    void onRemoveDirectoryClicked();
    void onRefreshClicked();
    void onPlaySelectedClicked();
    void onAddToPlaylistClicked();
    void onFilterChanged();
    void updateAnimation();
    void updateSearchResults();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void setupUI();
    void createSongList();
    void createDirectoryTree();
    void createControlPanel();
    void createSearchArea();
    void createFilterArea();
    void setupStyling();
    void setupCallbacks();
    void loadDirectoriesFromSettings();
    void saveDirectoriesToSettings();
    void scanDirectory(const QString& directory);
    void addSongToList(const SongInfo& songInfo);
    void updateDirectoryList();
    void filterSongs();
    void loadSongMetadata(const QString& filePath, SongInfo& songInfo);
    void showWithAnimation();
    void hideWithAnimation();
    void applyStyling();

    Lyricstator::Application* application_;
    
    // UI Components
    QVBoxLayout* mainLayout_;
    QHBoxLayout* topLayout_;
    QSplitter* mainSplitter_;
    QSplitter* leftSplitter_;
    
    // Song list area
    QGroupBox* songListGroup_;
    QTreeWidget* songList_;
    QVBoxLayout* songListLayout_;
    
    // Directory tree area
    QGroupBox* directoryGroup_;
    QTreeWidget* directoryTree_;
    QVBoxLayout* directoryLayout_;
    QHBoxLayout* directoryButtonLayout_;
    QPushButton* addDirectoryButton_;
    QPushButton* removeDirectoryButton_;
    QPushButton* refreshButton_;
    
    // Search and filter area
    QGroupBox* searchGroup_;
    QVBoxLayout* searchLayout_;
    QLineEdit* searchBox_;
    QComboBox* filterCombo_;
    QPushButton* clearSearchButton_;
    
    // Control panel
    QGroupBox* controlGroup_;
    QVBoxLayout* controlLayout_;
    QPushButton* playSelectedButton_;
    QPushButton* addToPlaylistButton_;
    QLabel* statusLabel_;
    QProgressBar* scanProgressBar_;
    
    // Data
    QStringList songDirectories_;
    QList<SongInfo> allSongs_;
    QList<SongInfo> filteredSongs_;
    QString currentFilter_;
    
    // Animation and state
    QPropertyAnimation* slideAnimation_;
    QTimer* animationTimer_;
    QTimer* searchTimer_;
    float animationTime_;
    bool isVisible_;
    bool isAnimating_;
    bool isScanning_;
};

#endif // QTSONGBROWSER_H