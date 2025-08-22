#include "QtMainWindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QStyle>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , application_(nullptr)
    , karaokeDisplay_(nullptr)
    , resourcePackGUI_(nullptr)
    , songBrowser_(nullptr)
    , equalizer_(nullptr)
    , keybindEditor_(nullptr)
{
    setWindowTitle("Lyricstator v1.0.0 - Karaoke and Lyric Visualization System");
    setAcceptDrops(true);
    resize(1200, 800);
    
    setupUI();
    createActions();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    connectSignals();
    
    // Center window on screen
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            qApp->desktop()->availableGeometry()
        )
    );
}

QtMainWindow::~QtMainWindow()
{
    // Qt will handle cleanup of child widgets
}

void QtMainWindow::setApplication(Lyricstator::Application* app)
{
    application_ = app;
    if (karaokeDisplay_) {
        karaokeDisplay_->setApplication(app);
    }
}

void QtMainWindow::loadFile(const QString& filepath)
{
    if (filepath.isEmpty()) return;
    
    QString extension = filepath.mid(filepath.lastIndexOf('.')).toLower();
    
    if (extension == ".mid" || extension == ".midi") {
        loadMidiFile(filepath);
    } else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
        loadAudioFile(filepath);
    } else if (extension == ".lystr") {
        loadLyricScript(filepath);
    } else {
        QMessageBox::warning(this, "Unsupported File", 
                           "File type not supported: " + extension);
    }
}

void QtMainWindow::loadMidiFile(const QString& filepath)
{
    if (!application_) return;
    
    try {
        application_->LoadMidiFile(filepath.toStdString());
        statusBar()->showMessage("Loaded MIDI file: " + filepath, 3000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
                           "Failed to load MIDI file: " + QString(e.what()));
    }
}

void QtMainWindow::loadAudioFile(const QString& filepath)
{
    if (!application_) return;
    
    try {
        application_->LoadAudioFile(filepath.toStdString());
        statusBar()->showMessage("Loaded audio file: " + filepath, 3000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
                           "Failed to load audio file: " + QString(e.what()));
    }
}

void QtMainWindow::loadLyricScript(const QString& filepath)
{
    if (!application_) return;
    
    try {
        application_->LoadLyricScript(filepath.toStdString());
        statusBar()->showMessage("Loaded lyric script: " + filepath, 3000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
                           "Failed to load lyric script: " + QString(e.what()));
    }
}

void QtMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void QtMainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        processDroppedFiles(mimeData->urls());
        event->acceptProposedAction();
    }
}

void QtMainWindow::closeEvent(QCloseEvent* event)
{
    if (application_) {
        application_->Shutdown();
    }
    event->accept();
}

void QtMainWindow::setupUI()
{
    setupCentralWidget();
}

void QtMainWindow::setupCentralWidget()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Create main karaoke display
    karaokeDisplay_ = new QtKaraokeDisplay(this);
    mainLayout->addWidget(karaokeDisplay_);
    
    // Create song browser
    songBrowser_ = new QtSongBrowser(this);
    mainLayout->addWidget(songBrowser_);
    
    // Set layout properties
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
}

void QtMainWindow::createActions()
{
    // File actions
    openFileAction_ = new QAction(QIcon::fromTheme("document-open"), "Open File...", this);
    openFileAction_->setShortcut(QKeySequence::Open);
    openFileAction_->setStatusTip("Open a file");
    
    openMidiAction_ = new QAction(QIcon::fromTheme("audio-x-midi"), "Open MIDI...", this);
    openMidiAction_->setShortcut(QKeySequence("Ctrl+M"));
    openMidiAction_->setStatusTip("Open a MIDI file");
    
    openAudioAction_ = new QAction(QIcon::fromTheme("audio-x-generic"), "Open Audio...", this);
    openAudioAction_->setShortcut(QKeySequence("Ctrl+A"));
    openAudioAction_->setStatusTip("Open an audio file");
    
    openLyricAction_ = new QAction(QIcon::fromTheme("text-x-generic"), "Open Lyrics...", this);
    openLyricAction_->setShortcut(QKeySequence("Ctrl+L"));
    openLyricAction_->setStatusTip("Open a lyric script file");
    
    quitAction_ = new QAction(QIcon::fromTheme("application-exit"), "Quit", this);
    quitAction_->setShortcut(QKeySequence::Quit);
    quitAction_->setStatusTip("Quit the application");
    
    // Tools actions
    resourcePackAction_ = new QAction(QIcon::fromTheme("applications-graphics"), "Resource Pack", this);
    resourcePackAction_->setShortcut(QKeySequence("F1"));
    resourcePackAction_->setStatusTip("Open resource pack manager");
    
    equalizerAction_ = new QAction(QIcon::fromTheme("audio-input-microphone"), "Equalizer", this);
    equalizerAction_->setShortcut(QKeySequence("F2"));
    equalizerAction_->setStatusTip("Open audio equalizer");
    
    keybindAction_ = new QAction(QIcon::fromTheme("preferences-desktop-keyboard"), "Keybinds", this);
    keybindAction_->setShortcut(QKeySequence("F3"));
    keybindAction_->setStatusTip("Configure keybindings");
}

void QtMainWindow::setupMenuBar()
{
    // File menu
    fileMenu_ = menuBar()->addMenu("&File");
    fileMenu_->addAction(openFileAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(openMidiAction_);
    fileMenu_->addAction(openAudioAction_);
    fileMenu_->addAction(openLyricAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(quitAction_);
    
    // Tools menu
    toolsMenu_ = menuBar()->addMenu("&Tools");
    toolsMenu_->addAction(resourcePackAction_);
    toolsMenu_->addAction(equalizerAction_);
    toolsMenu_->addAction(keybindAction_);
    
    // Help menu
    helpMenu_ = menuBar()->addMenu("&Help");
    helpMenu_->addAction("&About", this, &QtMainWindow::showAbout);
}

void QtMainWindow::setupToolBar()
{
    mainToolBar_ = addToolBar("Main Toolbar");
    mainToolBar_->setMovable(false);
    
    mainToolBar_->addAction(openFileAction_);
    mainToolBar_->addSeparator();
    mainToolBar_->addAction(openMidiAction_);
    mainToolBar_->addAction(openAudioAction_);
    mainToolBar_->addAction(openLyricAction_);
    mainToolBar_->addSeparator();
    mainToolBar_->addAction(resourcePackAction_);
    mainToolBar_->addAction(equalizerAction_);
    mainToolBar_->addAction(keybindAction_);
}

void QtMainWindow::setupStatusBar()
{
    statusBar_ = statusBar();
    statusBar_->showMessage("Ready");
}

void QtMainWindow::connectSignals()
{
    // File actions
    connect(openFileAction_, &QAction::triggered, this, &QtMainWindow::openFile);
    connect(openMidiAction_, &QAction::triggered, this, &QtMainWindow::openMidiFile);
    connect(openAudioAction_, &QAction::triggered, this, &QtMainWindow::openAudioFile);
    connect(openLyricAction_, &QAction::triggered, this, &QtMainWindow::openLyricScript);
    connect(quitAction_, &QAction::triggered, this, &QtMainWindow::quitApplication);
    
    // Tools actions
    connect(resourcePackAction_, &QAction::triggered, this, &QtMainWindow::showResourcePackGUI);
    connect(equalizerAction_, &QAction::triggered, this, &QtMainWindow::showEqualizer);
    connect(keybindAction_, &QAction::triggered, this, &QtMainWindow::showKeybindEditor);
}

void QtMainWindow::processDroppedFiles(const QList<QUrl>& urls)
{
    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            loadFile(url.toLocalFile());
        }
    }
}

void QtMainWindow::openFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Open File", "",
        "All Files (*);;MIDI Files (*.mid *.midi);;Audio Files (*.wav *.mp3 *.ogg);;Lyric Scripts (*.lystr)");
    
    if (!filepath.isEmpty()) {
        loadFile(filepath);
    }
}

void QtMainWindow::openMidiFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Open MIDI File", "",
        "MIDI Files (*.mid *.midi);;All Files (*)");
    
    if (!filepath.isEmpty()) {
        loadMidiFile(filepath);
    }
}

void QtMainWindow::openAudioFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Open Audio File", "",
        "Audio Files (*.wav *.mp3 *.ogg);;All Files (*)");
    
    if (!filepath.isEmpty()) {
        loadAudioFile(filepath);
    }
}

void QtMainWindow::openLyricScript()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Open Lyric Script", "",
        "Lyric Scripts (*.lystr);;All Files (*)");
    
    if (!filepath.isEmpty()) {
        loadLyricScript(filepath);
    }
}

void QtMainWindow::showResourcePackGUI()
{
    if (!resourcePackGUI_) {
        resourcePackGUI_ = new QtResourcePackGUI(this);
        resourcePackGUI_->setApplication(application_);
    }
    
    resourcePackGUI_->show();
    resourcePackGUI_->raise();
    resourcePackGUI_->activateWindow();
}

void QtMainWindow::showEqualizer()
{
    if (!equalizer_) {
        equalizer_ = new QtEqualizer(this);
        equalizer_->setApplication(application_);
    }
    
    equalizer_->show();
    equalizer_->raise();
    equalizer_->activateWindow();
}

void QtMainWindow::showKeybindEditor()
{
    if (!keybindEditor_) {
        keybindEditor_ = new QtKeybindEditor(this);
        keybindEditor_->setApplication(application_);
    }
    
    keybindEditor_->show();
    keybindEditor_->raise();
    keybindEditor_->activateWindow();
}

void QtMainWindow::showAbout()
{
    QMessageBox::about(this, "About Lyricstator",
        "<h3>Lyricstator v1.0.0</h3>"
        "<p>Karaoke and Lyric Visualization System</p>"
        "<p>A modern application for creating and displaying synchronized lyrics "
        "with audio and MIDI support.</p>"
        "<p>Built with Qt6 and modern C++</p>");
}

void QtMainWindow::quitApplication()
{
    QApplication::quit();
}