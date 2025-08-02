#pragma once
#include "common/Types.h"
#include <string>

namespace Lyricstator {

class ErrorHandler {
public:
    ErrorHandler();
    ~ErrorHandler();
    
    void ShowError(const std::string& message, ErrorType type);
    void LogError(const std::string& message);
    
private:
    void ShowSDLErrorDialog(const std::string& message);
};

} // namespace Lyricstator