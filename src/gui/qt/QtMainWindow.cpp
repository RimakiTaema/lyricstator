#include "QtMainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QString>
#include <QCloseEvent>
#include <QApplication>
#include <QScreen>
#include <QDir>
#include <QFileInfo>

QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , application_(nullptr)
    , karaokeDisplay_(nullptr)
    , songBrowser_(nullptr)
    , resourcePackGUI_(nullptr)
    , equalizer_(nullptr)
    , keybindEditor_(nullptr)
    , audioSettings_(nullptr)
    , midiEditor_(nullptr)
    , lyricEditor_(nullptr)
    , settings_(nullptr)
    , helpSystem_(nullptr)
    , openMidiAction_(nullptr)
    , openAudioAction_(nullptr)
    , openLyricsAction_(nullptr)
    , resourcePackAction_(nullptr)
    , equalizerAction_(nullptr)
    , keybindEditorAction_(nullptr)
    , audioSettingsAction_(nullptr)
    , midiEditorAction_(nullptr)
    , lyricEditorAction_(nullptr)
    , settingsAction_(nullptr)
    , helpAction_(nullptr)
    , aboutAction_(nullptr)
    , exitAction_(nullptr)
{
    setWindowTitle("Lyricstator - Qt6");
    setWindowIcon(QIcon(":/assets/icon.png"));
    
    // Set window size and center on screen
    resize(1200, 800);
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Enable drag and drop
    setAcceptDrops(true);
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
    setupCallbacks();
}

QtMainWindow::~QtMainWindow()
{
    // Clean up dialog instances
    if (resourcePackGUI_) delete resourcePackGUI_;
    if (equalizer_) delete equalizer_;
    if (keybindEditor_) delete keybindEditor_;
    if (audioSettings_) delete audioSettings_;
    if (midiEditor_) delete midiEditor_;
    if (lyricEditor_) delete lyricEditor_;
    if (settings_) delete settings_;
    if (helpSystem_) delete helpSystem_;
}

void QtMainWindow::setApplication(Lyricstator::Application* app)
{
    application_ = app;
    
    // Set application for all components
    if (karaokeDisplay_) {
        karaokeDisplay_->setApplication(app);
    }
    if (songBrowser_) {
        songBrowser_->setApplication(app);
    }
    if (resourcePackGUI_) {
        resourcePackGUI_->setApplication(app);
    }
    if (equalizer_) {
        equalizer_->setApplication(app);
    }
    if (keybindEditor_) {
        keybindEditor_->setApplication(app);
    }
    if (audioSettings_) {
        audioSettings_->setApplication(app);
    }
    if (midiEditor_) {
        midiEditor_->setApplication(app);
    }
    if (lyricEditor_) {
        lyricEditor_->setApplication(app);
    }
    if (settings_) {
        settings_->setApplication(app);
    }
    if (helpSystem_) {
        helpSystem_->setApplication(app);
    }
}

void QtMainWindow::setupUI()
{
    // Set window styling
    setStyleSheet(R"(
        QMainWindow {
            background-color: rgb(30, 35, 45);
            color: white;
        }
        QMenuBar {
            background-color: rgb(40, 45, 55);
            color: white;
            border-bottom: 1px solid rgb(60, 70, 90);
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 12px;
        }
        QMenuBar::item:selected {
            background-color: rgb(60, 70, 90);
        }
        QMenu {
            background-color: rgb(40, 45, 55);
            color: white;
            border: 1px solid rgb(60, 70, 90);
        }
        QMenu::item:selected {
            background-color: rgb(60, 70, 90);
        }
        QToolBar {
            background-color: rgb(35, 40, 50);
            border: none;
            spacing: 5px;
            padding: 5px;
        }
        QStatusBar {
            background-color: rgb(35, 40, 50);
            color: white;
            border-top: 1px solid rgb(60, 70, 90);
        }
    )");
}

void QtMainWindow::setupMenuBar()
{
    QMenuBar* menuBar = this->menuBar();
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("&File");
    
    openMidiAction_ = new QAction("Open &MIDI File...", this);
    openMidiAction_->setShortcut(QKeySequence("Ctrl+M"));
    openMidiAction_->setStatusTip("Open a MIDI file for karaoke");
    fileMenu->addAction(openMidiAction_);
    
    openAudioAction_ = new QAction("Open &Audio File...", this);
    openAudioAction_->setShortcut(QKeySequence("Ctrl+A"));
    openAudioAction_->setStatusTip("Open an audio file for karaoke");
    fileMenu->addAction(openAudioAction_);
    
    openLyricsAction_ = new QAction("Open &Lyrics File...", this);
    openLyricsAction_->setShortcut(QKeySequence("Ctrl+L"));
    openLyricsAction_->setStatusTip("Open a lyrics file");
    fileMenu->addAction(openLyricsAction_);
    
    fileMenu->addSeparator();
    
    exitAction_ = new QAction("E&xit", this);
    exitAction_->setShortcut(QKeySequence("Ctrl+Q"));
    exitAction_->setStatusTip("Exit the application");
    fileMenu->addAction(exitAction_);
    
    // Tools menu
    QMenu* toolsMenu = menuBar->addMenu("&Tools");
    
    resourcePackAction_ = new QAction("&Resource Pack Manager", this);
    resourcePackAction_->setShortcut(QKeySequence("F1"));
    resourcePackAction_->setStatusTip("Manage resource packs and themes");
    toolsMenu->addAction(resourcePackAction_);
    
    equalizerAction_ = new QAction("&Equalizer", this);
    equalizerAction_->setShortcut(QKeySequence("F2"));
    equalizerAction_->setStatusTip("Configure audio equalizer");
    toolsMenu->addAction(equalizerAction_);
    
    keybindEditorAction_ = new QAction("&Keyboard Shortcuts", this);
    keybindEditorAction_->setShortcut(QKeySequence("F3"));
    keybindEditorAction_->setStatusTip("Configure keyboard shortcuts");
    toolsMenu->addAction(keybindEditorAction_);
    
    audioSettingsAction_ = new QAction("&Audio Settings", this);
    audioSettingsAction_->setShortcut(QKeySequence("F4"));
    audioSettingsAction_->setStatusTip("Configure audio settings");
    toolsMenu->addAction(audioSettingsAction_);
    
    midiEditorAction_ = new QAction("&MIDI Editor", this);
    midiEditorAction_->setShortcut(QKeySequence("F5"));
    midiEditorAction_->setStatusTip("Edit MIDI files");
    toolsMenu->addAction(midiEditorAction_);
    
    lyricEditorAction_ = new QAction("&Lyric Editor", this);
    lyricEditorAction_->setShortcut(QKeySequence("F6"));
    lyricEditorAction_->setStatusTip("Edit lyrics and timing");
    toolsMenu->addAction(lyricEditorAction_);
    
    // Settings menu
    QMenu* settingsMenu = menuBar->addMenu("&Settings");
    
    settingsAction_ = new QAction("&Preferences", this);
    settingsAction_->setShortcut(QKeySequence("Ctrl+,"));
    settingsAction_->setStatusTip("Configure application preferences");
    settingsMenu->addAction(settingsAction_);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("&Help");
    
    helpAction_ = new QAction("&User Manual", this);
    helpAction_->setShortcut(QKeySequence("F1"));
    helpAction_->setStatusTip("Show user manual and help");
    helpMenu->addAction(helpAction_);
    
    aboutAction_ = new QAction("&About", this);
    aboutAction_->setStatusTip("About Lyricstator");
    helpMenu->addAction(aboutAction_);
}

void QtMainWindow::setupToolBar()
{
    QToolBar* toolBar = addToolBar("Main Toolbar");
    toolBar->setMovable(false);
    
    toolBar->addAction(openMidiAction_);
    toolBar->addAction(openAudioAction_);
    toolBar->addAction(openLyricsAction_);
    toolBar->addSeparator();
    toolBar->addAction(resourcePackAction_);
    toolBar->addAction(equalizerAction_);
    toolBar->addAction(keybindEditorAction_);
    toolBar->addSeparator();
    toolBar->addAction(settingsAction_);
    toolBar->addAction(helpAction_);
}

void QtMainWindow::setupStatusBar()
{
    QStatusBar* statusBar = this->statusBar();
    statusBar->showMessage("Ready - Drag and drop files to load them");
}

void QtMainWindow::setupCentralWidget()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create main components
    karaokeDisplay_ = new QtKaraokeDisplay(this);
    mainLayout->addWidget(karaokeDisplay_);
    
    songBrowser_ = new QtSongBrowser(this);
    mainLayout->addWidget(songBrowser_);
    
    // Set application for components
    if (application_) {
        karaokeDisplay_->setApplication(application_);
        songBrowser_->setApplication(application_);
    }
}

void QtMainWindow::setupCallbacks()
{
    // Connect menu actions
    connect(openMidiAction_, &QAction::triggered, this, &QtMainWindow::onOpenMidiFile);
    connect(openAudioAction_, &QAction::triggered, this, &QtMainWindow::onOpenAudioFile);
    connect(openLyricsAction_, &QAction::triggered, this, &QtMainWindow::onOpenLyricsFile);
    connect(resourcePackAction_, &QAction::triggered, this, &QtMainWindow::onShowResourcePackGUI);
    connect(equalizerAction_, &QAction::triggered, this, &QtMainWindow::onShowEqualizer);
    connect(keybindEditorAction_, &QAction::triggered, this, &QtMainWindow::onShowKeybindEditor);
    connect(audioSettingsAction_, &QAction::triggered, this, &QtMainWindow::onShowAudioSettings);
    connect(midiEditorAction_, &QAction::triggered, this, &QtMainWindow::onShowMidiEditor);
    connect(lyricEditorAction_, &QAction::triggered, this, &QtMainWindow::onShowLyricEditor);
    connect(settingsAction_, &QAction::triggered, this, &QtMainWindow::onShowSettings);
    connect(helpAction_, &QAction::triggered, this, &QtMainWindow::onShowHelp);
    connect(aboutAction_, &QAction::triggered, this, &QtMainWindow::onAbout);
    connect(exitAction_, &QAction::triggered, this, &QtMainWindow::onExit);
}

void QtMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        statusBar()->showMessage("Drop files to load them");
    }
}

void QtMainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                handleFileDrop(filePath);
            }
        }
        
        event->acceptProposedAction();
        statusBar()->showMessage("Files loaded successfully");
    }
}

void QtMainWindow::closeEvent(QCloseEvent* event)
{
    // Save any unsaved changes
    if (application_) {
        // TODO: Check for unsaved changes
        // if (hasUnsavedChanges()) {
        //     QMessageBox::StandardButton reply = QMessageBox::question(this, "Save Changes", 
        //         "Do you want to save your changes before exiting?",
        //         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        //     
        //     if (reply == QMessageBox::Cancel) {
        //         event->ignore();
        //         return;
        //     } else if (reply == QMessageBox::Yes) {
        //         // Save changes
        //     }
        // }
    }
    
    event->accept();
}

void QtMainWindow::loadFile(const QString& filePath)
{
    if (!application_) {
        QMessageBox::warning(this, "Error", "Application not initialized");
        return;
    }
    
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "mid" || extension == "midi") {
        // Load MIDI file
        if (application_->LoadMidiFile(filePath.toStdString())) {
            statusBar()->showMessage(QString("Loaded MIDI file: %1").arg(fileInfo.fileName()));
        } else {
            QMessageBox::warning(this, "Error", "Failed to load MIDI file");
        }
    } else if (extension == "mp3" || extension == "wav" || extension == "ogg" || 
               extension == "flac" || extension == "m4a" || extension == "aac") {
        // Load audio file
        if (application_->LoadAudioFile(filePath.toStdString())) {
            statusBar()->showMessage(QString("Loaded audio file: %1").arg(fileInfo.fileName()));
        } else {
            QMessageBox::warning(this, "Error", "Failed to load audio file");
        }
    } else if (extension == "lystr" || extension == "lrc" || extension == "txt") {
        // Load lyrics file
        if (application_->LoadLyricScript(filePath.toStdString())) {
            statusBar()->showMessage(QString("Loaded lyrics file: %1").arg(fileInfo.fileName()));
        } else {
            QMessageBox::warning(this, "Error", "Failed to load lyrics file");
        }
    } else {
        QMessageBox::warning(this, "Unsupported File", 
                           QString("Unsupported file type: %1").arg(extension));
    }
}

void QtMainWindow::handleFileDrop(const QString& filePath)
{
    loadFile(filePath);
}

// Slot implementations
void QtMainWindow::onOpenMidiFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open MIDI File", 
                                                   "", "MIDI Files (*.mid *.midi)");
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void QtMainWindow::onOpenAudioFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Audio File", 
                                                   "", "Audio Files (*.mp3 *.wav *.ogg *.flac *.m4a *.aac)");
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void QtMainWindow::onOpenLyricsFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Lyrics File", 
                                                   "", "Lyrics Files (*.lystr *.lrc *.txt)");
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void QtMainWindow::onShowResourcePackGUI()
{
    if (!resourcePackGUI_) {
        resourcePackGUI_ = new QtResourcePackGUI(this);
        if (application_) {
            resourcePackGUI_->setApplication(application_);
        }
    }
    resourcePackGUI_->show();
}

void QtMainWindow::onShowEqualizer()
{
    if (!equalizer_) {
        equalizer_ = new QtEqualizer(this);
        if (application_) {
            equalizer_->setApplication(application_);
        }
    }
    equalizer_->show();
}

void QtMainWindow::onShowKeybindEditor()
{
    if (!keybindEditor_) {
        keybindEditor_ = new QtKeybindEditor(this);
        if (application_) {
            keybindEditor_->setApplication(application_);
        }
    }
    keybindEditor_->show();
}

void QtMainWindow::onShowAudioSettings()
{
    if (!audioSettings_) {
        audioSettings_ = new QtAudioSettings(this);
        if (application_) {
            audioSettings_->setApplication(application_);
        }
    }
    audioSettings_->show();
}

void QtMainWindow::onShowMidiEditor()
{
    if (!midiEditor_) {
        midiEditor_ = new QtMidiEditor(this);
        if (application_) {
            midiEditor_->setApplication(application_);
        }
    }
    midiEditor_->show();
}

void QtMainWindow::onShowLyricEditor()
{
    if (!lyricEditor_) {
        lyricEditor_ = new QtLyricEditor(this);
        if (application_) {
            lyricEditor_->setApplication(application_);
        }
    }
    lyricEditor_->show();
}

void QtMainWindow::onShowSettings()
{
    if (!settings_) {
        settings_ = new QtSettings(this);
        if (application_) {
            settings_->setApplication(application_);
        }
    }
    settings_->show();
}

void QtMainWindow::onShowHelp()
{
    if (!helpSystem_) {
        helpSystem_ = new QtHelpSystem(this);
        if (application_) {
            helpSystem_->setApplication(application_);
        }
    }
    helpSystem_->show();
}

void QtMainWindow::onAbout()
{
    QMessageBox::about(this, "About Lyricstator", 
                       "<h2>Lyricstator</h2>"
                       "<p><b>Version:</b> 1.0.0</p>"
                       "<p><b>Description:</b> Advanced karaoke application with Qt6 interface</p>"
                       "<p><b>Features:</b></p>"
                       "<ul>"
                       "<li>MIDI and audio file support</li>"
                       "<li>Real-time pitch detection</li>"
                       "<li>Resource pack theming</li>"
                       "<li>Advanced audio equalizer</li>"
                       "<li>Customizable keyboard shortcuts</li>"
                       "<li>Professional audio settings</li>"
                       "<li>MIDI and lyric editing</li>"
                       "<li>Comprehensive preferences</li>"
                       "<li>Built-in help system</li>"
                       "</ul>"
                       "<p><b>Built with:</b> Qt6, SDL2, C++17</p>");
}

void QtMainWindow::onExit()
{
    QApplication::quit();
}