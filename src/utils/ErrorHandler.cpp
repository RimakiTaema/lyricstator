#include "utils/ErrorHandler.h"
#include <iostream>
#include <SDL2/SDL.h>

namespace Lyricstator {

ErrorHandler::ErrorHandler() {
}

ErrorHandler::~ErrorHandler() {
}

void ErrorHandler::ShowError(const std::string& message, ErrorType type) {
    std::string typeStr;
    switch (type) {
        case ErrorType::FILE_NOT_FOUND: typeStr = "File Not Found"; break;
        case ErrorType::UNSUPPORTED_FORMAT: typeStr = "Unsupported Format"; break;
        case ErrorType::PARSING_ERROR: typeStr = "Parsing Error"; break;
        case ErrorType::AUDIO_ERROR: typeStr = "Audio Error"; break;
        case ErrorType::SDL_ERROR: typeStr = "SDL Error"; break;
        case ErrorType::MEMORY_ERROR: typeStr = "Memory Error"; break;
        default: typeStr = "Error"; break;
    }
    
    std::cerr << "[" << typeStr << "] " << message << std::endl;
    
    // For GUI applications, we could show SDL message box
    // ShowSDLErrorDialog(typeStr + ": " + message);
}

void ErrorHandler::LogError(const std::string& message) {
    std::cerr << "[LOG] " << message << std::endl;
}

void ErrorHandler::ShowSDLErrorDialog(const std::string& message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Lyricstator Error", message.c_str(), nullptr);
}

} // namespace Lyricstator