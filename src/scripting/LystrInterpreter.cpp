#include "scripting/LystrInterpreter.h"
#include <iostream>

namespace Lyricstator {

LystrInterpreter::LystrInterpreter() : currentCommandIndex_(0) {
}

LystrInterpreter::~LystrInterpreter() {
}

void LystrInterpreter::LoadScript(const std::vector<LystrCommand>& commands) {
    commands_ = commands;
    currentCommandIndex_ = 0;
    std::cout << "Loaded script with " << commands_.size() << " commands" << std::endl;
}

void LystrInterpreter::Update(uint32_t currentTimeMs) {
    // Execute commands at their scheduled time
    while (currentCommandIndex_ < commands_.size()) {
        const auto& command = commands_[currentCommandIndex_];
        
        if (command.timestamp <= currentTimeMs) {
            // Execute command
            if (command.type == LystrCommandType::DISPLAY_LYRIC && lyricCallback_) {
                auto it = command.parameters.find("text");
                if (it != command.parameters.end()) {
                    lyricCallback_(it->second);
                }
            }
            currentCommandIndex_++;
        } else {
            break;
        }
    }
}

void LystrInterpreter::Seek(uint32_t timeMs) {
    // Reset to beginning and fast-forward to the correct position
    currentCommandIndex_ = 0;
    
    for (size_t i = 0; i < commands_.size(); ++i) {
        if (commands_[i].timestamp <= timeMs) {
            currentCommandIndex_ = i + 1;
        } else {
            break;
        }
    }
}

void LystrInterpreter::Reset() {
    currentCommandIndex_ = 0;
}

void LystrInterpreter::SetLyricCallback(std::function<void(const std::string&)> callback) {
    lyricCallback_ = callback;
}

} // namespace Lyricstator