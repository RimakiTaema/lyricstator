#include "core/Application.h"
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        std::cout << "=== Lyricstator v1.0.0 ===" << std::endl;
        std::cout << "Karaoke and Lyric Visualization System" << std::endl;
        std::cout << "=================================" << std::endl;
        
        // Create and initialize application
        Lyricstator::Application app;
        
        if (!app.Initialize()) {
            std::cerr << "Failed to initialize application!" << std::endl;
            return -1;
        }
        
        // Load files from command line arguments if provided
        for (int i = 1; i < argc; ++i) {
            std::string filepath = argv[i];
            std::string extension = filepath.substr(filepath.find_last_of('.'));
            
            if (extension == ".mid" || extension == ".midi") {
                std::cout << "Loading MIDI file from command line: " << filepath << std::endl;
                app.LoadMidiFile(filepath);
            } else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
                std::cout << "Loading audio file from command line: " << filepath << std::endl;
                app.LoadAudioFile(filepath);
            } else if (extension == ".lystr") {
                std::cout << "Loading lyric script from command line: " << filepath << std::endl;
                app.LoadLyricScript(filepath);
            } else {
                std::cout << "Ignoring unsupported file: " << filepath << std::endl;
            }
        }
        
        std::cout << "\nControls:" << std::endl;
        std::cout << "  Space     - Play/Pause" << std::endl;
        std::cout << "  Escape    - Stop" << std::endl;
        std::cout << "  Ctrl+Q    - Quit" << std::endl;
        std::cout << "  Drag & Drop files onto window to load them" << std::endl;
        std::cout << "\nSupported formats:" << std::endl;
        std::cout << "  Audio: .wav, .mp3, .ogg" << std::endl;
        std::cout << "  MIDI: .mid, .midi" << std::endl;
        std::cout << "  Lyrics: .lystr" << std::endl;
        std::cout << "\n" << std::endl;
        
        // Run the application
        app.Run();
        
        std::cout << "Application exited normally" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception occurred!" << std::endl;
        return -1;
    }
}
