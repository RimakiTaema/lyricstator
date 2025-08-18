#include "gui/TGUIKaraokeDisplay.h"
#include <cmath>

namespace Lyricstator {

TGUIKaraokeDisplay::TGUIKaraokeDisplay()
    : currentLyric_("")
    , pitchValue_(0.0f)
    , confidence_(0.0f)
    , highlightProgress_(0.0f)
    , animationTime_(0.0f)
    , primaryColor_(255, 255, 255)
    , accentColor_(100, 200, 255)
    , highlightColor_(255, 215, 0)
{
}

bool TGUIKaraokeDisplay::Initialize(tgui::Gui& gui) {
    try {
        CreateWidgets();
        SetupLayout();
        
        // Add to GUI - simple and clean
        gui.add(mainPanel_);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void TGUIKaraokeDisplay::CreateWidgets() {
    mainPanel_ = tgui::Panel::create();
    mainPanel_->setSize("100%", "100%");
    mainPanel_->getRenderer()->setBackgroundColor(tgui::Color(20, 25, 40, 200));
    
    lyricLabel_ = tgui::Label::create();
    lyricLabel_->setTextSize(48);
    lyricLabel_->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
    lyricLabel_->getRenderer()->setTextColor(primaryColor_);
    lyricLabel_->setText("Ready to sing!");
    
    pitchBar_ = tgui::ProgressBar::create();
    pitchBar_->setMinimum(0);
    pitchBar_->setMaximum(100);
    pitchBar_->getRenderer()->setFillColor(accentColor_);
    pitchBar_->getRenderer()->setBackgroundColor(tgui::Color(50, 50, 50));
    
    visualizerPanel_ = tgui::Panel::create();
    visualizerPanel_->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
    
    // Add widgets to main panel - clean hierarchy
    mainPanel_->add(lyricLabel_);
    mainPanel_->add(pitchBar_);
    mainPanel_->add(visualizerPanel_);
}

void TGUIKaraokeDisplay::SetupLayout() {
    lyricLabel_->setPosition("10%", "40%");
    lyricLabel_->setSize("80%", "20%");
    
    pitchBar_->setPosition("20%", "70%");
    pitchBar_->setSize("60%", "30");
    
    visualizerPanel_->setPosition("0%", "0%");
    visualizerPanel_->setSize("100%", "100%");
}

void TGUIKaraokeDisplay::Update(float deltaTime) {
    animationTime_ += deltaTime;
    UpdateAnimations(deltaTime);
    UpdatePitchVisualization();
}

void TGUIKaraokeDisplay::SetLyric(const std::string& text) {
    currentLyric_ = text;
    lyricLabel_->setText(text);
    
    lyricLabel_->getRenderer()->setOpacity(0.0f);
    // TGUI animation would go here in a real implementation
}

void TGUIKaraokeDisplay::HighlightLyric(float progress) {
    highlightProgress_ = std::clamp(progress, 0.0f, 1.0f);
    
    float intensity = 0.5f + 0.5f * std::sin(animationTime_ * 3.0f);
    tgui::Color highlightedColor = tgui::Color(
        static_cast<tgui::Uint8>(highlightColor_.getRed() * intensity),
        static_cast<tgui::Uint8>(highlightColor_.getGreen() * intensity),
        static_cast<tgui::Uint8>(highlightColor_.getBlue() * intensity)
    );
    
    lyricLabel_->getRenderer()->setTextColor(highlightedColor);
}

void TGUIKaraokeDisplay::UpdatePitch(float frequency, float confidence) {
    pitchValue_ = frequency;
    confidence_ = confidence;
    
    float targetValue = (confidence * 100.0f);
    pitchBar_->setValue(static_cast<unsigned int>(targetValue));
}

void TGUIKaraokeDisplay::UpdateAnimations(float deltaTime) {
    float pulseIntensity = 0.8f + 0.2f * std::sin(animationTime_ * 2.0f);
    tgui::Color bgColor = tgui::Color(
        static_cast<tgui::Uint8>(20 * pulseIntensity),
        static_cast<tgui::Uint8>(25 * pulseIntensity),
        static_cast<tgui::Uint8>(40 * pulseIntensity),
        200
    );
    mainPanel_->getRenderer()->setBackgroundColor(bgColor);
}

void TGUIKaraokeDisplay::UpdatePitchVisualization() {
    if (confidence_ > 0.8f) {
        pitchBar_->getRenderer()->setFillColor(tgui::Color::Green);
    } else if (confidence_ > 0.5f) {
        pitchBar_->getRenderer()->setFillColor(tgui::Color::Yellow);
    } else {
        pitchBar_->getRenderer()->setFillColor(tgui::Color::Red);
    }
}

void TGUIKaraokeDisplay::ApplyTheme(const std::string& themeName) {
    if (themeName == "neon") {
        primaryColor_ = tgui::Color(0, 255, 255);
        accentColor_ = tgui::Color(255, 0, 255);
        highlightColor_ = tgui::Color(255, 255, 0);
    } else if (themeName == "retro") {
        primaryColor_ = tgui::Color(255, 100, 150);
        accentColor_ = tgui::Color(100, 255, 200);
        highlightColor_ = tgui::Color(255, 200, 100);
    } else {
        // Default theme
        primaryColor_ = tgui::Color(255, 255, 255);
        accentColor_ = tgui::Color(100, 200, 255);
        highlightColor_ = tgui::Color(255, 215, 0);
    }
    
    // Apply colors to widgets
    lyricLabel_->getRenderer()->setTextColor(primaryColor_);
    pitchBar_->getRenderer()->setFillColor(accentColor_);
}

void TGUIKaraokeDisplay::Shutdown() {
    if (mainPanel_) {
        mainPanel_->removeAllWidgets();
        mainPanel_ = nullptr;
    }
}

} // namespace Lyricstator
