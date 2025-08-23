#ifndef QTMAINWINDOW_H
#define QTMAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QString>

#include "core/Application.h"
#include "gui/qt/QtKaraokeDisplay.h"
#include "gui/qt/QtResourcePackGUI.h"
#include "gui/qt/QtSongBrowser.h"
#include "gui/qt/QtEqualizer.h"
#include "gui/qt/QtKeybindEditor.h"
#include "gui/qt/QtAudioSettings.h"
#include "gui/qt/QtMidiEditor.h"
#include "gui/qt/QtLyricEditor.h"
#include "gui/qt/QtSettings.h"
#include "gui/qt/QtHelpSystem.h"

class QtMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtMainWindow(QWidget *parent = nullptr);
    ~QtMainWindow();

    void setApplication(Lyricstator::Application* app);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onOpenMidiFile();
    void onOpenAudioFile();
    void onOpenLyricsFile();
    void onShowResourcePackGUI();
    void onShowEqualizer();
    void onShowKeybindEditor();
    void onShowAudioSettings();
    void onShowMidiEditor();
    void onShowLyricEditor();
    void onShowSettings();
    void onShowHelp();
    void onAbout();
    void onExit();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupCallbacks();
    void loadFile(const QString& filePath);
    void handleFileDrop(const QString& filePath);

    Lyricstator::Application* application_;
    
    // Main UI components
    QtKaraokeDisplay* karaokeDisplay_;
    QtSongBrowser* songBrowser_;
    
    // Menu actions
    QAction* openMidiAction_;
    QAction* openAudioAction_;
    QAction* openLyricsAction_;
    QAction* resourcePackAction_;
    QAction* equalizerAction_;
    QAction* keybindEditorAction_;
    QAction* audioSettingsAction_;
    QAction* midiEditorAction_;
    QAction* lyricEditorAction_;
    QAction* settingsAction_;
    QAction* helpAction_;
    QAction* aboutAction_;
    QAction* exitAction_;
    
    // Dialog instances
    QtResourcePackGUI* resourcePackGUI_;
    QtEqualizer* equalizer_;
    QtKeybindEditor* keybindEditor_;
    QtAudioSettings* audioSettings_;
    QtMidiEditor* midiEditor_;
    QtLyricEditor* lyricEditor_;
    QtSettings* settings_;
    QtHelpSystem* helpSystem_;
};

#endif // QTMAINWINDOW_H