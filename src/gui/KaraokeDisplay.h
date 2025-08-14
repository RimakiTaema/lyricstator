#pragma once
#include "common/Types.h"
#include <string>

struct SDL_Renderer;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

namespace Lyricstator {

enum class TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

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
    
    void SetColorScheme(const Color& primary, const Color& accent, const Color& highlight);
    
private:
    // Core rendering
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    
    // Font sizes
    int largeFontSize_;
    int mediumFontSize_;
    int smallFontSize_;
    
    // Current state
    std::string currentLyric_;
    float currentPitch_;
    float currentConfidence_;
    bool initialized_;
    
    float animationTime_;
    float highlightProgress_;
    float backgroundGradientOffset_;
    float targetPitchHeight_;
    float smoothPitchHeight_;
    
    Color backgroundColor_;
    Color primaryTextColor_;
    Color highlightColor_;
    Color accentColor_;
    Color pitchBarColor_;
    Color shadowColor_;
    
    void RenderAnimatedBackground(int width, int height);
    void RenderLyricArea(int width, int height);
    void RenderPitchVisualization(int width, int height);
    void RenderUIElements(int width, int height);
    void RenderText(const std::string& text, int x, int y, const Color& color, 
                   TextAlign align = TextAlign::LEFT, int fontSize = 24);
    void RenderHighlightEffect(int x, int y);
    void RenderDecorations(int width, int height);
    void DrawCircle(int centerX, int centerY, int radius);
};

} // namespace Lyricstator
