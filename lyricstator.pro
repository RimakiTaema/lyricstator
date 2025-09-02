QT += core gui widgets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/core/Application.cpp \
    src/core/AssetManager.cpp \
    src/core/AudioEngine.cpp \
    src/core/ConfigManager.cpp \
    src/core/LyricParser.cpp \
    src/core/MidiParser.cpp \
    src/core/SongManager.cpp \
    src/audio/AudioDevice.cpp \
    src/audio/AudioProcessor.cpp \
    src/audio/MidiPlayer.cpp \
    src/audio/PitchDetector.cpp \
    src/gui/KaraokeDisplay.cpp \
    src/gui/ResourcePackGUI.cpp \
    src/gui/UserInterface.cpp \
    src/gui/Window.cpp \
    src/gui/qt/QtKaraokeDisplay.cpp \
    src/gui/qt/QtResourcePackGUI.cpp \
    src/gui/qt/QtSongBrowser.cpp \
    src/gui/qt/QtMainWindow.cpp \
    src/gui/qt/QtEqualizer.cpp \
    src/gui/qt/QtKeybindEditor.cpp \
    src/utils/FileUtils.cpp \
    src/utils/Logger.cpp \
    src/utils/StringUtils.cpp \
    src/ai/PitchAnalyzer.cpp \
    src/export/LyricExporter.cpp \
    src/export/AudioExporter.cpp \
    src/scripting/ScriptEngine.cpp \
    src/sync/LyricSynchronizer.cpp

HEADERS += \
    src/core/Application.h \
    src/core/AssetManager.h \
    src/core/AudioEngine.h \
    src/core/ConfigManager.h \
    src/core/LyricParser.h \
    src/core/MidiParser.h \
    src/core/SongManager.h \
    src/audio/AudioDevice.h \
    src/audio/AudioProcessor.h \
    src/audio/MidiPlayer.h \
    src/audio/PitchDetector.h \
    src/gui/KaraokeDisplay.h \
    src/gui/ResourcePackGUI.h \
    src/gui/UserInterface.h \
    src/gui/Window.h \
    src/gui/qt/QtKaraokeDisplay.h \
    src/gui/qt/QtResourcePackGUI.h \
    src/gui/qt/QtSongBrowser.h \
    src/gui/qt/QtMainWindow.h \
    src/gui/qt/QtEqualizer.h \
    src/gui/qt/QtKeybindEditor.h \
    src/utils/FileUtils.h \
    src/utils/Logger.h \
    src/utils/StringUtils.h \
    src/ai/PitchAnalyzer.h \
    src/export/LyricExporter.h \
    src/export/AudioExporter.h \
    src/scripting/ScriptEngine.h \
    src/sync/LyricSynchronizer.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Android specific configuration
android {
    QT += androidextras
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android/app/src/main
    ANDROID_ABIS = armeabi-v7a arm64-v8a x86 x86_64
}

# Desktop specific configuration
!android {
    # Use Qt Installer Framework for deployment
    # This will be configured in the installer project
}

# Include paths
INCLUDEPATH += \
    src \
    src/core \
    src/audio \
    src/gui \
    src/gui/qt \
    src/utils \
    src/ai \
    src/export \
    src/scripting \
    src/sync

# Library dependencies
unix:!macx: LIBS += -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2_image
macx: LIBS += -framework SDL2 -framework SDL2_mixer -framework SDL2_ttf -framework SDL2_image
win32: LIBS += -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2_image

# Additional compiler flags
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS_DEBUG += -g -O0 -DDEBUG
QMAKE_CXXFLAGS_RELEASE += -O3 -DNDEBUG

# Resources
RESOURCES += \
    resources/resources.qrc

# Desktop file
desktop.path = /usr/share/applications
desktop.files = lyricstator.desktop
INSTALLS += desktop