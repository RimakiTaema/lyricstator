#pragma once
#include "common/Types.h"
#include <string>

struct SDL_Renderer;

namespace Lyricstator {

class KaraokeDisplay {
public:
    KaraokeDisplay();
    ~KaraokeDisplay();
    
    bool Initialize(SDL_Renderer* renderer);
    void Shutdown();
    
    void Update(float deltaTime);
    void Render();
    
    void HighlightLyric(const std::string& text);
    void UpdatePitchDisplay(float frequency, float confidence);
    
private:
    SDL_Renderer* renderer_;
    std::string currentLyric_;
    float currentPitch_;
    float currentConfidence_;
    bool initialized_;
};

} // namespace Lyricstator