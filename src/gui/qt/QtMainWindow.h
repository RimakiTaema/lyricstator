#ifndef QTMAINWINDOW_H
#define QTMAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QString>

#include "core/Application.h"
#include "QtKaraokeDisplay.h"
#include "QtResourcePackGUI.h"
#include "QtSongBrowser.h"
#include "QtEqualizer.h"
#include "QtKeybindEditor.h"

class QtMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtMainWindow(QWidget *parent = nullptr);
    ~QtMainWindow();

    void setApplication(Lyricstator::Application* app);
    void loadFile(const QString& filepath);
    void loadMidiFile(const QString& filepath);
    void loadAudioFile(const QString& filepath);
    void loadLyricScript(const QString& filepath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void openFile();
    void openMidiFile();
    void openAudioFile();
    void openLyricScript();
    void showResourcePackGUI();
    void showEqualizer();
    void showKeybindEditor();
    void showAbout();
    void quitApplication();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void createActions();
    void connectSignals();
    void processDroppedFiles(const QList<QUrl>& urls);

    Lyricstator::Application* application_;
    
    // UI Components
    QtKaraokeDisplay* karaokeDisplay_;
    QtResourcePackGUI* resourcePackGUI_;
    QtSongBrowser* songBrowser_;
    QtEqualizer* equalizer_;
    QtKeybindEditor* keybindEditor_;
    
    // Actions
    QAction* openFileAction_;
    QAction* openMidiAction_;
    QAction* openAudioAction_;
    QAction* openLyricAction_;
    QAction* resourcePackAction_;
    QAction* equalizerAction_;
    QAction* keybindAction_;
    QAction* quitAction_;
    
    // Menus
    QMenu* fileMenu_;
    QMenu* toolsMenu_;
    QMenu* helpMenu_;
    
    // Toolbar
    QToolBar* mainToolBar_;
    
    // Status bar
    QStatusBar* statusBar_;
};

#endif // QTMAINWINDOW_H