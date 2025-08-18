#pragma once

#include "common/Types.h"
#include <TGUI/TGUI.hpp>
#include <string>
#include <memory>

namespace Lyricstator {

/**
 * Modern TGUI-based karaoke display with smooth animations and beautiful visuals
 * Easy to maintain and extend with new features
 */
class TGUIKaraokeDisplay {
public:
    TGUIKaraokeDisplay();
    ~TGUIKaraokeDisplay() = default;
    
    // Simple initialization - just pass the TGUI gui reference
    bool Initialize(tgui::Gui& gui);
    void Shutdown();
    
    // Main update and event handling
    void Update(float deltaTime);
    void HandleEvent(const sf::Event& event);
    
    // Karaoke functionality - simple interface
    void SetLyric(const std::string& text);
    void HighlightLyric(float progress); // 0.0 to 1.0
    void UpdatePitch(float frequency, float confidence);
    
    // Theme support - easy to customize
    void ApplyTheme(const std::string& themeName);
    void SetColors(const Color& primary, const Color& accent, const Color& highlight);
    
private:
    // TGUI widgets - easy to understand
    tgui::Panel::Ptr mainPanel_;
    tgui::Label::Ptr lyricLabel_;
    tgui::ProgressBar::Ptr pitchBar_;
    tgui::Panel::Ptr visualizerPanel_;
    
    // Simple state management
    std::string currentLyric_;
    float pitchValue_;
    float confidence_;
    float highlightProgress_;
    
    // Animation helpers - smooth and simple
    float animationTime_;
    tgui::Color primaryColor_;
    tgui::Color accentColor_;
    tgui::Color highlightColor_;
    
    // Easy setup methods
    void CreateWidgets();
    void SetupLayout();
    void UpdateAnimations(float deltaTime);
    void UpdatePitchVisualization();
};

} // namespace Lyricstator
