#pragma once
#include "common/Types.h"
#include <string>
#include <fstream>
#include <memory>

namespace Lyricstator {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class ErrorHandler {
public:
    ErrorHandler();
    ~ErrorHandler();
    
    void ShowError(const std::string& message, ErrorType type);
    void LogError(const std::string& message);
    
    void Log(const std::string& message, LogLevel level = LogLevel::INFO);
    void LogDebug(const std::string& message);
    void LogWarning(const std::string& message);
    void LogCritical(const std::string& message);
    
    bool InitializeFileLogging(const std::string& logFilePath);
    void SetConsoleLogging(bool enabled);
    void SetLogLevel(LogLevel minLevel);
    
    std::string GetErrorSolution(ErrorType type) const;
    
private:
    void ShowSDLErrorDialog(const std::string& message);
    
    void WriteToLog(const std::string& message, LogLevel level);
    std::string GetLogLevelString(LogLevel level) const;
    std::string GetCurrentTimestamp() const;
    
    std::unique_ptr<std::ofstream> logFile_;
    bool consoleLoggingEnabled_;
    bool fileLoggingEnabled_;
    LogLevel minLogLevel_;
    std::string logFilePath_;
};

} // namespace Lyricstator
