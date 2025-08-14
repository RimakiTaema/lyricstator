#include "gui/KaraokeDisplay.h"
#include "utils/ErrorHandler.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Lyricstator {

KaraokeDisplay::KaraokeDisplay() 
    : renderer_(nullptr), 
      font_(nullptr),
      largeFontSize_(48),
      mediumFontSize_(32),
      smallFontSize_(24),
      currentPitch_(0.0f), 
      currentConfidence_(0.0f), 
      initialized_(false),
      animationTime_(0.0f),
      highlightProgress_(0.0f),
      backgroundGradientOffset_(0.0f) {
    
    // Initialize color scheme
    backgroundColor_ = {15, 15, 25, 255};
    primaryTextColor_ = {255, 255, 255, 255};
    highlightColor_ = {255, 215, 0, 255};  // Gold
    accentColor_ = {64, 224, 208, 255};    // Turquoise
    pitchBarColor_ = {50, 205, 50, 255};   // Lime green
    shadowColor_ = {0, 0, 0, 128};
}

KaraokeDisplay::~KaraokeDisplay() {
    Shutdown();
}

bool KaraokeDisplay::Initialize(SDL_Renderer* renderer) {
    renderer_ = renderer;
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return false;
    }
    
    // Load fonts - try multiple paths for cross-platform compatibility
    const char* fontPaths[] = {
        "assets/fonts/NotoSans-Bold.ttf",
        "/System/Library/Fonts/Arial.ttf",           // macOS
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", // Linux
        "C:/Windows/Fonts/arial.ttf"                 // Windows
    };
    
    for (const char* fontPath : fontPaths) {
        font_ = TTF_OpenFont(fontPath, largeFontSize_);
        if (font_) {
            std::cout << "Loaded font: " << fontPath << std::endl;
            break;
        }
    }
    
    if (!font_) {
        std::cerr << "Warning: Could not load any font, using default rendering" << std::endl;
        // Continue without font - we'll use basic rendering
    }
    
    initialized_ = true;
    std::cout << "Enhanced KaraokeDisplay initialized with visual improvements" << std::endl;
    return true;
}

void KaraokeDisplay::Shutdown() {
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    TTF_Quit();
    initialized_ = false;
}

void KaraokeDisplay::Update(float deltaTime) {
    if (!initialized_) return;
    
    animationTime_ += deltaTime;
    backgroundGradientOffset_ += deltaTime * 0.1f; // Slow gradient animation
    
    // Smooth highlight animation
    if (highlightProgress_ < 1.0f) {
        highlightProgress_ = std::min(1.0f, highlightProgress_ + deltaTime * 2.0f);
    }
    
    // Update pitch visualization smoothing
    targetPitchHeight_ = currentConfidence_ * 200.0f;
    smoothPitchHeight_ += (targetPitchHeight_ - smoothPitchHeight_) * deltaTime * 5.0f;
}

void KaraokeDisplay::Render() {
    if (!renderer_) return;
    
    // Get window dimensions
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer_, &windowWidth, &windowHeight);
    
    // Render animated background
    RenderAnimatedBackground(windowWidth, windowHeight);
    
    // Render main lyric area
    RenderLyricArea(windowWidth, windowHeight);
    
    // Render pitch visualization
    RenderPitchVisualization(windowWidth, windowHeight);
    
    // Render UI elements
    RenderUIElements(windowWidth, windowHeight);
}

void KaraokeDisplay::RenderAnimatedBackground(int width, int height) {
    // Create animated gradient background
    for (int y = 0; y < height; y += 4) {
        float gradientPos = static_cast<float>(y) / height;
        float wave = std::sin(gradientPos * 3.14159f + backgroundGradientOffset_) * 0.1f + 0.9f;
        
        uint8_t r = static_cast<uint8_t>(backgroundColor_.r * wave);
        uint8_t g = static_cast<uint8_t>(backgroundColor_.g * wave);
        uint8_t b = static_cast<uint8_t>((backgroundColor_.b + 20) * wave);
        
        SDL_SetRenderDrawColor(renderer_, r, g, b, 255);
        SDL_Rect stripe = {0, y, width, 4};
        SDL_RenderFillRect(renderer_, &stripe);
    }
}

void KaraokeDisplay::RenderLyricArea(int width, int height) {
    // Main lyric display area with rounded corners effect
    int lyricAreaY = height * 0.6f;
    int lyricAreaHeight = height * 0.25f;
    
    // Draw shadow
    SDL_SetRenderDrawColor(renderer_, shadowColor_.r, shadowColor_.g, shadowColor_.b, shadowColor_.a);
    SDL_Rect shadowRect = {52, lyricAreaY + 2, width - 104, lyricAreaHeight};
    SDL_RenderFillRect(renderer_, &shadowRect);
    
    // Draw main lyric area
    SDL_SetRenderDrawColor(renderer_, 30, 30, 50, 200);
    SDL_Rect lyricArea = {50, lyricAreaY, width - 100, lyricAreaHeight};
    SDL_RenderFillRect(renderer_, &lyricArea);
    
    // Draw border with accent color
    SDL_SetRenderDrawColor(renderer_, accentColor_.r, accentColor_.g, accentColor_.b, 255);
    SDL_RenderDrawRect(renderer_, &lyricArea);
    
    // Render current lyric text
    if (!currentLyric_.empty()) {
        RenderText(currentLyric_, width / 2, lyricAreaY + lyricAreaHeight / 2, 
                  primaryTextColor_, TextAlign::CENTER, largeFontSize_);
        
        // Render highlight effect
        if (highlightProgress_ > 0.0f) {
            RenderHighlightEffect(width / 2, lyricAreaY + lyricAreaHeight / 2);
        }
    }
}

void KaraokeDisplay::RenderPitchVisualization(int width, int height) {
    if (currentConfidence_ < 0.05f) return;
    
    int vizX = width - 100;
    int vizY = height * 0.2f;
    int vizWidth = 60;
    int vizHeight = height * 0.4f;
    
    // Background for pitch meter
    SDL_SetRenderDrawColor(renderer_, 20, 20, 30, 200);
    SDL_Rect pitchBg = {vizX, vizY, vizWidth, vizHeight};
    SDL_RenderFillRect(renderer_, &pitchBg);
    
    // Border
    SDL_SetRenderDrawColor(renderer_, accentColor_.r, accentColor_.g, accentColor_.b, 255);
    SDL_RenderDrawRect(renderer_, &pitchBg);
    
    // Pitch level indicator
    int pitchBarHeight = static_cast<int>(smoothPitchHeight_);
    pitchBarHeight = std::min(pitchBarHeight, vizHeight - 4);
    
    if (pitchBarHeight > 0) {
        // Color based on pitch accuracy (simplified)
        uint8_t r = static_cast<uint8_t>(255 * (1.0f - currentConfidence_) + pitchBarColor_.r * currentConfidence_);
        uint8_t g = static_cast<uint8_t>(pitchBarColor_.g * currentConfidence_);
        uint8_t b = static_cast<uint8_t>(pitchBarColor_.b * currentConfidence_);
        
        SDL_SetRenderDrawColor(renderer_, r, g, b, 255);
        SDL_Rect pitchBar = {vizX + 2, vizY + vizHeight - pitchBarHeight - 2, vizWidth - 4, pitchBarHeight};
        SDL_RenderFillRect(renderer_, &pitchBar);
        
        // Add glow effect
        SDL_SetRenderDrawColor(renderer_, r, g, b, 100);
        SDL_Rect glowBar = {vizX, vizY + vizHeight - pitchBarHeight - 4, vizWidth, pitchBarHeight + 4};
        SDL_RenderDrawRect(renderer_, &glowBar);
    }
    
    // Frequency text
    if (currentPitch_ > 0) {
        std::string freqText = std::to_string(static_cast<int>(currentPitch_)) + " Hz";
        RenderText(freqText, vizX + vizWidth/2, vizY - 20, primaryTextColor_, TextAlign::CENTER, smallFontSize_);
    }
}

void KaraokeDisplay::RenderUIElements(int width, int height) {
    // Title area
    RenderText("Lyricstator", width / 2, 40, highlightColor_, TextAlign::CENTER, mediumFontSize_);
    
    // Status indicators
    std::string status = "♪ Ready for Karaoke ♪";
    if (currentConfidence_ > 0.1f) {
        status = "♫ Singing Detected! ♫";
    }
    RenderText(status, width / 2, height - 30, accentColor_, TextAlign::CENTER, smallFontSize_);
    
    // Decorative elements
    RenderDecorations(width, height);
}

void KaraokeDisplay::RenderText(const std::string& text, int x, int y, const Color& color, 
                               TextAlign align, int fontSize) {
    if (!font_ || text.empty()) {
        // Fallback to basic rendering if no font available
        return;
    }
    
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    SDL_Surface* textSurface = TTF_RenderText_Blended(font_, text.c_str(), sdlColor);
    
    if (!textSurface) {
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer_, textSurface);
    
    if (textTexture) {
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        
        int renderX = x;
        if (align == TextAlign::CENTER) {
            renderX = x - textWidth / 2;
        } else if (align == TextAlign::RIGHT) {
            renderX = x - textWidth;
        }
        
        SDL_Rect destRect = {renderX, y - textHeight / 2, textWidth, textHeight};
        SDL_RenderCopy(renderer_, textTexture, nullptr, &destRect);
        
        SDL_DestroyTexture(textTexture);
    }
    
    SDL_FreeSurface(textSurface);
}

void KaraokeDisplay::RenderHighlightEffect(int x, int y) {
    // Animated highlight glow
    float glowIntensity = std::sin(animationTime_ * 4.0f) * 0.3f + 0.7f;
    uint8_t alpha = static_cast<uint8_t>(255 * glowIntensity * highlightProgress_);
    
    SDL_SetRenderDrawColor(renderer_, highlightColor_.r, highlightColor_.g, highlightColor_.b, alpha);
    
    // Multiple glow rings
    for (int i = 1; i <= 3; ++i) {
        int radius = static_cast<int>(30 * i * highlightProgress_);
        DrawCircle(x, y, radius);
    }
}

void KaraokeDisplay::RenderDecorations(int width, int height) {
    // Musical note decorations
    SDL_SetRenderDrawColor(renderer_, accentColor_.r, accentColor_.g, accentColor_.b, 100);
    
    // Animated floating notes
    for (int i = 0; i < 5; ++i) {
        float noteTime = animationTime_ + i * 1.2f;
        int noteX = 30 + i * (width / 6);
        int noteY = static_cast<int>(100 + std::sin(noteTime) * 20);
        
        // Simple note representation
        SDL_Rect note = {noteX, noteY, 8, 8};
        SDL_RenderFillRect(renderer_, &note);
    }
}

void KaraokeDisplay::DrawCircle(int centerX, int centerY, int radius) {
    // Simple circle drawing using SDL rectangles (approximation)
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer_, centerX + dx, centerY + dy);
            }
        }
    }
}

void KaraokeDisplay::HighlightLyric(const std::string& text) {
    currentLyric_ = text;
    highlightProgress_ = 0.0f; // Reset animation
    std::cout << "Highlighting lyric with enhanced visuals: " << text << std::endl;
}

void KaraokeDisplay::UpdatePitchDisplay(float frequency, float confidence) {
    currentPitch_ = frequency;
    currentConfidence_ = confidence;
}

void KaraokeDisplay::SetColorScheme(const Color& primary, const Color& accent, const Color& highlight) {
    primaryTextColor_ = primary;
    accentColor_ = accent;
    highlightColor_ = highlight;
}

} // namespace Lyricstator
