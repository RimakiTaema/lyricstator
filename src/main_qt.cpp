#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <iostream>
#include <exception>

#include "core/QtApplication.h"
#include "gui/qt/QtMainWindow.h"

int main(int argc, char* argv[]) {
    try {
        std::cout << "=== Lyricstator v1.0.0 (Qt6) ===" << std::endl;
        std::cout << "Karaoke and Lyric Visualization System" << std::endl;
        std::cout << "=================================" << std::endl;
        
        // Create and initialize Qt application
        Lyricstator::QtApplication app(argc, argv);
        
        if (!app.Initialize()) {
            std::cerr << "Failed to initialize Qt application!" << std::endl;
            return -1;
        }
        
        // Load files from command line arguments if provided
        for (int i = 1; i < argc; ++i) {
            QString filepath = QString::fromLocal8Bit(argv[i]);
            QString extension = filepath.mid(filepath.lastIndexOf('.')).toLower();
            
            if (extension == ".mid" || extension == ".midi") {
                std::cout << "Loading MIDI file from command line: " << filepath.toStdString() << std::endl;
                app.LoadMidiFile(filepath);
            } else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
                std::cout << "Loading audio file from command line: " << filepath.toStdString() << std::endl;
                app.LoadAudioFile(filepath);
            } else if (extension == ".lystr") {
                std::cout << "Loading lyric script from command line: " << filepath.toStdString() << std::endl;
                app.LoadLyricScript(filepath);
            } else {
                std::cout << "Ignoring unsupported file: " << filepath.toStdString() << std::endl;
            }
        }
        
        std::cout << "\nQt6 GUI Controls:" << std::endl;
        std::cout << "  Menu Bar     - File operations and tools" << std::endl;
        std::cout << "  Toolbar      - Quick access buttons" << std::endl;
        std::cout << "  Space        - Play/Pause" << std::endl;
        std::cout << "  Escape       - Stop" << std::endl;
        std::cout << "  F1           - Resource Pack GUI" << std::endl;
        std::cout << "  F2           - Equalizer" << std::endl;
        std::cout << "  F3           - Keybind Editor" << std::endl;
        std::cout << "  Ctrl+Q       - Quit" << std::endl;
        std::cout << "  Drag & Drop  - Load files" << std::endl;
        std::cout << "\nSupported formats:" << std::endl;
        std::cout << "  Audio: .wav, .mp3, .ogg" << std::endl;
        std::cout << "  MIDI: .mid, .midi" << std::endl;
        std::cout << "  Lyrics: .lystr" << std::endl;
        std::cout << "\n" << std::endl;
        
        // Run the Qt application
        int result = app.Run();
        
        std::cout << "Qt Application exited normally" << std::endl;
        return result;
        
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception occurred!" << std::endl;
        return -1;
    }
}