#pragma once
#include "common/Types.h"

namespace Lyricstator {

class SynchronizationManager {
public:
    SynchronizationManager();
    ~SynchronizationManager();
    
    bool Initialize();
    void Shutdown();
    
    void Update(uint32_t currentTimeMs);
    void Seek(uint32_t timeMs);
    
private:
    bool initialized_;
};

} // namespace Lyricstator