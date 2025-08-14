#pragma once
#include "common/Types.h"
#include <vector>
#include <functional>

namespace Lyricstator {

class LystrInterpreter {
public:
    LystrInterpreter();
    ~LystrInterpreter();
    
    void LoadScript(const std::vector<LystrCommand>& commands);
    void Update(uint32_t currentTimeMs);
    void Seek(uint32_t timeMs);
    void Reset();
    
    void SetLyricCallback(std::function<void(const std::string&)> callback);
    
private:
    std::vector<LystrCommand> commands_;
    size_t currentCommandIndex_;
    std::function<void(const std::string&)> lyricCallback_;
};

} // namespace Lyricstator
