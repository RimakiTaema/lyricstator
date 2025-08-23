#include "QtSongBrowser.h"
#include <QApplication>
#include <QScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QSettings>
#include <QMediaMetaData>
#include <QEasingCurve>
#include <QGraphicsDropShadowEffect>
#include <QStyleOption>
#include <QPainter>
#include <algorithm>

QtSongBrowser::QtSongBrowser(QWidget *parent)
    : QWidget(parent)
    , application_(nullptr)
    , slideAnimation_(nullptr)
    , animationTimer_(nullptr)
    , searchTimer_(nullptr)
    , animationTime_(0.0f)
    , isVisible_(false)
    , isAnimating_(false)
    , isScanning_(false)
{
    setAcceptDrops(true);
    setMinimumSize(800, 600);
    
    setupUI();
    setupStyling();
    setupCallbacks();
    loadDirectoriesFromSettings();
    
    // Setup animation timer
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &QtSongBrowser::updateAnimation);
    animationTimer_->start(16); // ~60 FPS
    
    // Setup search delay timer
    searchTimer_ = new QTimer(this);
    searchTimer_->setSingleShot(true);
    searchTimer_->setInterval(300); // 300ms delay
    connect(searchTimer_, &QTimer::timeout, this, &QtSongBrowser::updateSearchResults);
    
    refreshSongList();
}

QtSongBrowser::~QtSongBrowser()
{
    saveDirectoriesToSettings();
    if (animationTimer_) {
        animationTimer_->stop();
    }
    if (searchTimer_) {
        searchTimer_->stop();
    }
}

void QtSongBrowser::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtSongBrowser::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    // Create main splitter
    mainSplitter_ = new QSplitter(Qt::Horizontal);
    mainLayout_->addWidget(mainSplitter_);
    
    // Create left splitter for directory tree and search
    leftSplitter_ = new QSplitter(Qt::Vertical);
    
    createDirectoryTree();
    createSearchArea();
    createFilterArea();
    
    leftSplitter_->addWidget(directoryGroup_);
    leftSplitter_->addWidget(searchGroup_);
    leftSplitter_->setSizes({300, 200});
    
    // Create song list
    createSongList();
    
    // Create control panel
    createControlPanel();
    
    // Add to main splitter
    mainSplitter_->addWidget(leftSplitter_);
    mainSplitter_->addWidget(songListGroup_);
    mainSplitter_->addWidget(controlGroup_);
    mainSplitter_->setSizes({250, 400, 150});
}

void QtSongBrowser::createSongList()
{
    songListGroup_ = new QGroupBox("Songs");
    songListLayout_ = new QVBoxLayout(songListGroup_);
    
    songList_ = new QTreeWidget();
    songList_->setHeaderLabels({"Title", "Artist", "Album", "Duration", "Path"});
    songList_->setRootIsDecorated(false);
    songList_->setAlternatingRowColors(true);
    songList_->setSortingEnabled(true);
    songList_->sortByColumn(0, Qt::AscendingOrder);
    
    // Set column widths
    songList_->header()->setStretchLastSection(true);
    songList_->setColumnWidth(0, 200); // Title
    songList_->setColumnWidth(1, 150); // Artist
    songList_->setColumnWidth(2, 150); // Album
    songList_->setColumnWidth(3, 80);  // Duration
    
    songListLayout_->addWidget(songList_);
}

void QtSongBrowser::createDirectoryTree()
{
    directoryGroup_ = new QGroupBox("Song Directories");
    directoryLayout_ = new QVBoxLayout(directoryGroup_);
    
    directoryTree_ = new QTreeWidget();
    directoryTree_->setHeaderLabels({"Directory", "Songs"});
    directoryTree_->setRootIsDecorated(true);
    directoryLayout_->addWidget(directoryTree_);
    
    // Directory control buttons
    directoryButtonLayout_ = new QHBoxLayout();
    
    addDirectoryButton_ = new QPushButton("Add Directory");
    removeDirectoryButton_ = new QPushButton("Remove");
    refreshButton_ = new QPushButton("Refresh");
    
    directoryButtonLayout_->addWidget(addDirectoryButton_);
    directoryButtonLayout_->addWidget(removeDirectoryButton_);
    directoryButtonLayout_->addWidget(refreshButton_);
    
    directoryLayout_->addLayout(directoryButtonLayout_);
}

void QtSongBrowser::createSearchArea()
{
    searchGroup_ = new QGroupBox("Search");
    searchLayout_ = new QVBoxLayout(searchGroup_);
    
    // Search box
    QHBoxLayout* searchBoxLayout = new QHBoxLayout();
    searchBox_ = new QLineEdit();
    searchBox_->setPlaceholderText("Search songs...");
    clearSearchButton_ = new QPushButton("Clear");
    clearSearchButton_->setMaximumWidth(60);
    
    searchBoxLayout->addWidget(searchBox_);
    searchBoxLayout->addWidget(clearSearchButton_);
    searchLayout_->addLayout(searchBoxLayout);
}

void QtSongBrowser::createFilterArea()
{
    // Filter combo
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel("Filter:");
    filterCombo_ = new QComboBox();
    filterCombo_->addItems({"All Files", "Audio Only", "MIDI Only", "With Lyrics", "Without Lyrics"});
    
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filterCombo_);
    filterLayout->addStretch();
    searchLayout_->addLayout(filterLayout);
}

void QtSongBrowser::createControlPanel()
{
    controlGroup_ = new QGroupBox("Controls");
    controlLayout_ = new QVBoxLayout(controlGroup_);
    
    playSelectedButton_ = new QPushButton("Play Selected");
    addToPlaylistButton_ = new QPushButton("Add to Playlist");
    
    controlLayout_->addWidget(playSelectedButton_);
    controlLayout_->addWidget(addToPlaylistButton_);
    controlLayout_->addStretch();
    
    // Status and progress
    statusLabel_ = new QLabel("Ready");
    statusLabel_->setWordWrap(true);
    scanProgressBar_ = new QProgressBar();
    scanProgressBar_->setVisible(false);
    
    controlLayout_->addWidget(statusLabel_);
    controlLayout_->addWidget(scanProgressBar_);
}

void QtSongBrowser::setupStyling()
{
    applyStyling();
    
    // Add shadow effects
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    shadowEffect->setOffset(3, 3);
    setGraphicsEffect(shadowEffect);
}

void QtSongBrowser::applyStyling()
{
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid rgb(80, 100, 140);
            border-radius: 8px;
            margin: 5px;
            padding-top: 10px;
            background-color: rgb(25, 30, 45);
            color: rgb(200, 220, 255);
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: rgb(200, 220, 255);
        }
        QTreeWidget, QListWidget {
            background-color: rgb(20, 25, 40);
            border: 1px solid rgb(60, 80, 120);
            border-radius: 5px;
            color: white;
            alternate-background-color: rgb(30, 35, 50);
            selection-background-color: rgb(100, 150, 200);
        }
        QTreeWidget::item:hover, QListWidget::item:hover {
            background-color: rgb(40, 50, 70);
        }
        QTreeWidget::item:selected, QListWidget::item:selected {
            background-color: rgb(100, 150, 200);
        }
        QHeaderView::section {
            background-color: rgb(40, 50, 70);
            color: white;
            border: 1px solid rgb(60, 80, 120);
            padding: 5px;
        }
        QPushButton {
            background-color: rgb(60, 80, 120);
            border: 1px solid rgb(100, 120, 160);
            border-radius: 5px;
            color: white;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(80, 100, 140);
        }
        QPushButton:pressed {
            background-color: rgb(40, 60, 100);
        }
        QLineEdit {
            background-color: rgb(30, 35, 50);
            border: 1px solid rgb(60, 80, 120);
            border-radius: 4px;
            color: white;
            padding: 6px;
        }
        QLineEdit:focus {
            border-color: rgb(100, 150, 200);
        }
        QComboBox {
            background-color: rgb(30, 35, 50);
            border: 1px solid rgb(60, 80, 120);
            border-radius: 4px;
            color: white;
            padding: 6px;
        }
        QComboBox:drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
        QLabel {
            color: rgb(200, 220, 255);
        }
        QProgressBar {
            background-color: rgb(30, 35, 50);
            border: 1px solid rgb(60, 80, 120);
            border-radius: 4px;
            text-align: center;
            color: white;
        }
        QProgressBar::chunk {
            background-color: rgb(100, 150, 200);
            border-radius: 3px;
        }
    )");
}

void QtSongBrowser::setupCallbacks()
{
    // Song list connections
    connect(songList_, &QTreeWidget::itemClicked, this, &QtSongBrowser::onSongItemClicked);
    connect(songList_, &QTreeWidget::itemDoubleClicked, this, &QtSongBrowser::onSongItemDoubleClicked);
    
    // Directory tree connections
    connect(directoryTree_, &QTreeWidget::itemClicked, this, &QtSongBrowser::onDirectoryItemClicked);
    connect(addDirectoryButton_, &QPushButton::clicked, this, &QtSongBrowser::onAddDirectoryClicked);
    connect(removeDirectoryButton_, &QPushButton::clicked, this, &QtSongBrowser::onRemoveDirectoryClicked);
    connect(refreshButton_, &QPushButton::clicked, this, &QtSongBrowser::onRefreshClicked);
    
    // Search connections
    connect(searchBox_, &QLineEdit::textChanged, this, &QtSongBrowser::onSearchTextChanged);
    connect(clearSearchButton_, &QPushButton::clicked, [this]() { searchBox_->clear(); });
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtSongBrowser::onFilterChanged);
    
    // Control connections
    connect(playSelectedButton_, &QPushButton::clicked, this, &QtSongBrowser::onPlaySelectedClicked);
    connect(addToPlaylistButton_, &QPushButton::clicked, this, &QtSongBrowser::onAddToPlaylistClicked);
}

void QtSongBrowser::loadDirectoriesFromSettings()
{
    QSettings settings;
    settings.beginGroup("SongBrowser");
    songDirectories_ = settings.value("directories").toStringList();
    settings.endGroup();
    
    // Add default music directories if none exist
    if (songDirectories_.isEmpty()) {
        QStringList musicDirs = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
        if (!musicDirs.isEmpty()) {
            songDirectories_.append(musicDirs.first());
        }
    }
    
    updateDirectoryList();
}

void QtSongBrowser::saveDirectoriesToSettings()
{
    QSettings settings;
    settings.beginGroup("SongBrowser");
    settings.setValue("directories", songDirectories_);
    settings.endGroup();
}

void QtSongBrowser::addSongDirectory(const QString& directory)
{
    if (!songDirectories_.contains(directory)) {
        songDirectories_.append(directory);
        updateDirectoryList();
        saveDirectoriesToSettings();
    }
}

void QtSongBrowser::removeSongDirectory(const QString& directory)
{
    songDirectories_.removeAll(directory);
    updateDirectoryList();
    saveDirectoriesToSettings();
}

void QtSongBrowser::updateDirectoryList()
{
    directoryTree_->clear();
    
    for (const QString& directory : songDirectories_) {
        QTreeWidgetItem* item = new QTreeWidgetItem(directoryTree_);
        item->setText(0, QFileInfo(directory).fileName());
        item->setText(1, "Scanning...");
        item->setData(0, Qt::UserRole, directory);
        item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    }
}

void QtSongBrowser::refreshSongList()
{
    if (isScanning_) return;
    
    allSongs_.clear();
    songList_->clear();
    
    statusLabel_->setText("Scanning directories...");
    scanProgressBar_->setVisible(true);
    scanProgressBar_->setRange(0, songDirectories_.size());
    scanProgressBar_->setValue(0);
    isScanning_ = true;
    
    // Scan directories (in a real implementation, this should be done in a separate thread)
    for (int i = 0; i < songDirectories_.size(); ++i) {
        scanDirectory(songDirectories_[i]);
        scanProgressBar_->setValue(i + 1);
        QApplication::processEvents(); // Keep UI responsive
    }
    
    scanProgressBar_->setVisible(false);
    isScanning_ = false;
    
    filterSongs();
    statusLabel_->setText(QString("Found %1 songs").arg(allSongs_.size()));
}

void QtSongBrowser::scanDirectory(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists()) return;
    
    QStringList audioExtensions = {"*.mp3", "*.wav", "*.ogg", "*.flac", "*.m4a", "*.aac"};
    QStringList midiExtensions = {"*.mid", "*.midi"};
    QStringList lyricExtensions = {"*.lystr", "*.lrc", "*.txt"};
    
    QStringList allExtensions = audioExtensions + midiExtensions + lyricExtensions;
    
    QDirIterator iterator(directory, allExtensions, QDir::Files, QDirIterator::Subdirectories);
    
    while (iterator.hasNext()) {
        QString filePath = iterator.next();
        QFileInfo fileInfo(filePath);
        
        if (audioExtensions.contains("*." + fileInfo.suffix().toLower()) ||
            midiExtensions.contains("*." + fileInfo.suffix().toLower())) {
            
            SongInfo songInfo;
            songInfo.filePath = filePath;
            songInfo.extension = fileInfo.suffix().toLower();
            songInfo.fileSize = fileInfo.size();
            
            loadSongMetadata(filePath, songInfo);
            allSongs_.append(songInfo);
        }
    }
    
    // Update directory tree item count
    for (int i = 0; i < directoryTree_->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = directoryTree_->topLevelItem(i);
        QString itemDir = item->data(0, Qt::UserRole).toString();
        if (itemDir == directory) {
            int songCount = std::count_if(allSongs_.begin(), allSongs_.end(),
                [directory](const SongInfo& song) {
                    return song.filePath.startsWith(directory);
                });
            item->setText(1, QString("%1 songs").arg(songCount));
            break;
        }
    }
}

void QtSongBrowser::loadSongMetadata(const QString& filePath, SongInfo& songInfo)
{
    QFileInfo fileInfo(filePath);
    
    // Default values
    songInfo.title = fileInfo.baseName();
    songInfo.artist = "Unknown Artist";
    songInfo.album = "Unknown Album";
    songInfo.duration = 0;
    songInfo.hasLyrics = false;
    songInfo.hasMidi = false;
    
    // Check for companion files
    QString baseName = fileInfo.completeBaseName();
    QString dirPath = fileInfo.absolutePath();
    
    // Check for lyrics file
    QStringList lyricExtensions = {"lystr", "lrc", "txt"};
    for (const QString& ext : lyricExtensions) {
        QString lyricPath = dirPath + "/" + baseName + "." + ext;
        if (QFileInfo::exists(lyricPath)) {
            songInfo.hasLyrics = true;
            break;
        }
    }
    
    // Check for MIDI file
    QStringList midiExtensions = {"mid", "midi"};
    for (const QString& ext : midiExtensions) {
        QString midiPath = dirPath + "/" + baseName + "." + ext;
        if (QFileInfo::exists(midiPath)) {
            songInfo.hasMidi = true;
            break;
        }
    }
    
    // TODO: Load actual metadata using QMediaMetaData for audio files
    // This would require additional Qt Multimedia components
}

void QtSongBrowser::addSongToList(const SongInfo& songInfo)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(songList_);
    item->setText(0, songInfo.title);
    item->setText(1, songInfo.artist);
    item->setText(2, songInfo.album);
    item->setText(3, QString("%1:%2").arg(songInfo.duration / 60).arg(songInfo.duration % 60, 2, 10, QChar('0')));
    item->setText(4, songInfo.filePath);
    
    // Set icons based on file type and features
    if (songInfo.extension == "mid" || songInfo.extension == "midi") {
        item->setIcon(0, style()->standardIcon(QStyle::SP_MediaPlay));
    } else {
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
    }
    
    // Store song info in item data
    item->setData(0, Qt::UserRole, QVariant::fromValue(songInfo));
}

void QtSongBrowser::filterSongs()
{
    songList_->clear();
    filteredSongs_.clear();
    
    QString searchText = searchBox_->text().toLower();
    QString filter = filterCombo_->currentText();
    
    for (const SongInfo& song : allSongs_) {
        bool passesSearch = searchText.isEmpty() ||
                           song.title.toLower().contains(searchText) ||
                           song.artist.toLower().contains(searchText) ||
                           song.album.toLower().contains(searchText);
        
        bool passesFilter = true;
        if (filter == "Audio Only") {
            passesFilter = (song.extension != "mid" && song.extension != "midi");
        } else if (filter == "MIDI Only") {
            passesFilter = (song.extension == "mid" || song.extension == "midi");
        } else if (filter == "With Lyrics") {
            passesFilter = song.hasLyrics;
        } else if (filter == "Without Lyrics") {
            passesFilter = !song.hasLyrics;
        }
        
        if (passesSearch && passesFilter) {
            filteredSongs_.append(song);
            addSongToList(song);
        }
    }
    
    statusLabel_->setText(QString("Showing %1 of %2 songs").arg(filteredSongs_.size()).arg(allSongs_.size()));
}

void QtSongBrowser::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void QtSongBrowser::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        for (const QUrl& url : mimeData->urls()) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);
                
                if (fileInfo.isDir()) {
                    addSongDirectory(filePath);
                } else {
                    // Handle individual file drops
                    emit songSelected(filePath);
                }
            }
        }
        event->acceptProposedAction();
    }
}

// Slot implementations
void QtSongBrowser::onSearchTextChanged()
{
    searchTimer_->start(); // Restart the timer on each text change
}

void QtSongBrowser::onSongItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    if (!item) return;
    
    SongInfo songInfo = item->data(0, Qt::UserRole).value<SongInfo>();
    statusLabel_->setText(QString("Selected: %1 - %2").arg(songInfo.artist).arg(songInfo.title));
}

void QtSongBrowser::onSongItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    if (!item) return;
    
    SongInfo songInfo = item->data(0, Qt::UserRole).value<SongInfo>();
    emit songSelected(songInfo.filePath);
}

void QtSongBrowser::onDirectoryItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    if (!item) return;
    
    QString directory = item->data(0, Qt::UserRole).toString();
    statusLabel_->setText(QString("Directory: %1").arg(directory));
}

void QtSongBrowser::onAddDirectoryClicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Select Song Directory");
    if (!directory.isEmpty()) {
        addSongDirectory(directory);
        refreshSongList();
    }
}

void QtSongBrowser::onRemoveDirectoryClicked()
{
    QTreeWidgetItem* currentItem = directoryTree_->currentItem();
    if (!currentItem) return;
    
    QString directory = currentItem->data(0, Qt::UserRole).toString();
    int ret = QMessageBox::question(this, "Remove Directory",
                                   QString("Remove directory '%1' from the list?").arg(directory),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        removeSongDirectory(directory);
        refreshSongList();
    }
}

void QtSongBrowser::onRefreshClicked()
{
    refreshSongList();
}

void QtSongBrowser::onPlaySelectedClicked()
{
    QTreeWidgetItem* currentItem = songList_->currentItem();
    if (!currentItem) return;
    
    SongInfo songInfo = currentItem->data(0, Qt::UserRole).value<SongInfo>();
    emit songSelected(songInfo.filePath);
}

void QtSongBrowser::onAddToPlaylistClicked()
{
    QTreeWidgetItem* currentItem = songList_->currentItem();
    if (!currentItem) return;
    
    SongInfo songInfo = currentItem->data(0, Qt::UserRole).value<SongInfo>();
    // TODO: Implement playlist functionality
    statusLabel_->setText(QString("Added to playlist: %1").arg(songInfo.title));
    emit playlistChanged();
}

void QtSongBrowser::onFilterChanged()
{
    filterSongs();
}

void QtSongBrowser::updateAnimation()
{
    animationTime_ += 0.016f; // Assume 60 FPS
    
    // Subtle animations could be added here
    if (isVisible_ && !isAnimating_) {
        // Could add subtle effects like progress bar animations, etc.
    }
}

void QtSongBrowser::updateSearchResults()
{
    filterSongs();
}

void QtSongBrowser::show()
{
    isVisible_ = true;
    QWidget::show();
    showWithAnimation();
}

void QtSongBrowser::hide()
{
    hideWithAnimation();
}

void QtSongBrowser::toggle()
{
    if (isVisible_) {
        hide();
    } else {
        show();
    }
}

void QtSongBrowser::setVisible(bool visible)
{
    if (visible) {
        show();
    } else {
        hide();
    }
}

void QtSongBrowser::showWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    slideAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    slideAnimation_->setDuration(300);
    slideAnimation_->setStartValue(0.0);
    slideAnimation_->setEndValue(1.0);
    slideAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(slideAnimation_, &QPropertyAnimation::finished, [this]() {
        isAnimating_ = false;
        slideAnimation_->deleteLater();
        slideAnimation_ = nullptr;
    });
    
    slideAnimation_->start();
}

void QtSongBrowser::hideWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    slideAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    slideAnimation_->setDuration(200);
    slideAnimation_->setStartValue(1.0);
    slideAnimation_->setEndValue(0.0);
    slideAnimation_->setEasingCurve(QEasingCurve::InCubic);
    
    connect(slideAnimation_, &QPropertyAnimation::finished, [this]() {
        isVisible_ = false;
        isAnimating_ = false;
        QWidget::hide();
        slideAnimation_->deleteLater();
        slideAnimation_ = nullptr;
    });
    
    slideAnimation_->start();
}

// Register SongInfo as a metatype so it can be stored in QVariant
Q_DECLARE_METATYPE(SongInfo)