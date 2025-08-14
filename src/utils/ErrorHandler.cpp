#include "utils/ErrorHandler.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <SDL2/SDL.h>

namespace Lyricstator {

ErrorHandler::ErrorHandler() 
    : consoleLoggingEnabled_(true),
      fileLoggingEnabled_(false),
      minLogLevel_(LogLevel::INFO) {
}

ErrorHandler::~ErrorHandler() {
    if (logFile_ && logFile_->is_open()) {
        Log("ErrorHandler shutting down", LogLevel::INFO);
        logFile_->close();
    }
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
    
    std::string fullMessage = "[" + typeStr + "] " + message;
    std::string solution = GetErrorSolution(type);
    if (!solution.empty()) {
        fullMessage += "\nSuggested solution: " + solution;
    }
    
    Log(fullMessage, LogLevel::ERROR);
    
    if (type == ErrorType::MEMORY_ERROR || type == ErrorType::SDL_ERROR) {
        ShowSDLErrorDialog(fullMessage);
    }
}

void ErrorHandler::LogError(const std::string& message) {
    Log(message, LogLevel::ERROR);
}

void ErrorHandler::Log(const std::string& message, LogLevel level) {
    if (level < minLogLevel_) {
        return;
    }
    
    WriteToLog(message, level);
}

void ErrorHandler::LogDebug(const std::string& message) {
    Log(message, LogLevel::DEBUG);
}

void ErrorHandler::LogWarning(const std::string& message) {
    Log(message, LogLevel::WARNING);
}

void ErrorHandler::LogCritical(const std::string& message) {
    Log(message, LogLevel::CRITICAL);
    // Critical errors always show dialog
    ShowSDLErrorDialog("Critical Error: " + message);
}

bool ErrorHandler::InitializeFileLogging(const std::string& logFilePath) {
    logFilePath_ = logFilePath;
    logFile_ = std::make_unique<std::ofstream>(logFilePath, std::ios::app);
    
    if (logFile_->is_open()) {
        fileLoggingEnabled_ = true;
        Log("File logging initialized: " + logFilePath, LogLevel::INFO);
        return true;
    } else {
        std::cerr << "Failed to initialize file logging: " << logFilePath << std::endl;
        return false;
    }
}

void ErrorHandler::SetConsoleLogging(bool enabled) {
    consoleLoggingEnabled_ = enabled;
}

void ErrorHandler::SetLogLevel(LogLevel minLevel) {
    minLogLevel_ = minLevel;
}

std::string ErrorHandler::GetErrorSolution(ErrorType type) const {
    switch (type) {
        case ErrorType::FILE_NOT_FOUND:
            return "Check if the file path is correct and the file exists";
        case ErrorType::UNSUPPORTED_FORMAT:
            return "Use supported formats: WAV, MP3, OGG for audio; MID for MIDI; LYSTR for lyrics";
        case ErrorType::PARSING_ERROR:
            return "Verify the file is not corrupted and follows the expected format";
        case ErrorType::AUDIO_ERROR:
            return "Check audio device availability and SDL_mixer installation";
        case ErrorType::SDL_ERROR:
            return "Ensure SDL2 libraries are properly installed and initialized";
        case ErrorType::MEMORY_ERROR:
            return "Close other applications to free memory or restart the application";
        default:
            return "";
    }
}

void ErrorHandler::ShowSDLErrorDialog(const std::string& message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Lyricstator Error", message.c_str(), nullptr);
}

void ErrorHandler::WriteToLog(const std::string& message, LogLevel level) {
    std::string timestamp = GetCurrentTimestamp();
    std::string levelStr = GetLogLevelString(level);
    std::string formattedMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    // Console logging
    if (consoleLoggingEnabled_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << formattedMessage << std::endl;
        } else {
            std::cout << formattedMessage << std::endl;
        }
    }
    
    // File logging
    if (fileLoggingEnabled_ && logFile_ && logFile_->is_open()) {
        *logFile_ << formattedMessage << std::endl;
        logFile_->flush(); // Ensure immediate write for errors
    }
}

std::string ErrorHandler::GetLogLevelString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string ErrorHandler::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

} // namespace Lyricstator
