#include "TGUIEqualizer.h"
#include "core/SettingsManager.h"
#include "utils/ErrorHandler.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace Lyricstator {

TGUIEqualizer::TGUIEqualizer()
    : gui_(nullptr)
    , initialized_(false)
    , isVisible_(false)
    , equalizerEnabled_(true)
    , showSpectrum_(true)
    , slideAnimation_(0.0f)
    , isAnimating_(false)
{
    spectrumData_.resize(64, 0.0f);
    smoothedSpectrum_.resize(64, 0.0f);
}

TGUIEqualizer::~TGUIEqualizer() {
    Shutdown();
}

bool TGUIEqualizer::Initialize(tgui::Gui* gui) {
    if (initialized_ || !gui) {
        return false;
    }
    
    gui_ = gui;
    
    CreateUI();
    SetupStyling();
    SetupCallbacks();
    
    // Load equalizer settings
    auto& settings = SettingsManager::getInstance();
    const auto& audioSettings = settings.getAudioSettings();
    equalizerEnabled_ = audioSettings.enableEqualizer;
    
    // Create band controls based on settings
    SetBandCount(audioSettings.equalizerBandCount);
    
    // Apply saved band gains
    for (size_t i = 0; i < audioSettings.equalizerBands.size() && i < bandControls_.size(); ++i) {
        SetBandGain(i, audioSettings.equalizerBands[i].gain);
    }
    
    InitializeDefaultPresets();
    
    initialized_ = true;
    ErrorHandler::getInstance().logInfo("TGUIEqualizer initialized successfully");
    return true;
}

void TGUIEqualizer::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    Hide();
    
    if (gui_ && mainPanel_) {
        gui_->remove(mainPanel_);
    }
    
    initialized_ = false;
}

void TGUIEqualizer::CreateUI() {
    // Main panel - positioned on the right side
    mainPanel_ = tgui::Panel::create();
    mainPanel_->setSize("35%", "90%");
    mainPanel_->setPosition("100%", "5%"); // Start off-screen
    mainPanel_->getRenderer()->setBackgroundColor(tgui::Color(20, 25, 35, 245));
    mainPanel_->getRenderer()->setBorderColor(tgui::Color(100, 200, 255));
    mainPanel_->getRenderer()->setBorders(2);
    mainPanel_->getRenderer()->setRoundedBorderRadius(15);
    mainPanel_->setVisible(false);
    
    // Title bar
    titleLabel_ = tgui::Label::create("Audio Equalizer");
    titleLabel_->setSize("70%", "8%");
    titleLabel_->setPosition("3%", "2%");
    titleLabel_->getRenderer()->setTextColor(tgui::Color::White);
    titleLabel_->setTextSize(22);
    mainPanel_->add(titleLabel_);
    
    // Close button
    closeButton_ = tgui::Button::create("×");
    closeButton_->setSize("8%", "8%");
    closeButton_->setPosition("89%", "2%");
    closeButton_->getRenderer()->setBackgroundColor(tgui::Color(220, 60, 60));
    closeButton_->getRenderer()->setBackgroundColorHover(tgui::Color(255, 80, 80));
    closeButton_->getRenderer()->setTextColor(tgui::Color::White);
    closeButton_->getRenderer()->setRoundedBorderRadius(8);
    closeButton_->setTextSize(18);
    mainPanel_->add(closeButton_);
    
    // Enable/Disable button
    enableButton_ = tgui::Button::create("ON");
    enableButton_->setSize("15%", "6%");
    enableButton_->setPosition("3%", "12%");
    enableButton_->getRenderer()->setBackgroundColor(tgui::Color(50, 200, 50));
    enableButton_->getRenderer()->setBackgroundColorHover(tgui::Color(70, 220, 70));
    enableButton_->getRenderer()->setTextColor(tgui::Color::White);
    enableButton_->getRenderer()->setRoundedBorderRadius(6);
    enableButton_->setTextSize(14);
    mainPanel_->add(enableButton_);
    
    // Reset button
    resetButton_ = tgui::Button::create("Reset");
    resetButton_->setSize("15%", "6%");
    resetButton_->setPosition("20%", "12%");
    resetButton_->getRenderer()->setBackgroundColor(tgui::Color(200, 150, 50));
    resetButton_->getRenderer()->setBackgroundColorHover(tgui::Color(220, 170, 70));
    resetButton_->getRenderer()->setTextColor(tgui::Color::White);
    resetButton_->getRenderer()->setRoundedBorderRadius(6);
    resetButton_->setTextSize(14);
    mainPanel_->add(resetButton_);
    
    // Preset combo box
    presetComboBox_ = tgui::ComboBox::create();
    presetComboBox_->setSize("35%", "6%");
    presetComboBox_->setPosition("37%", "12%");
    presetComboBox_->getRenderer()->setBackgroundColor(tgui::Color(40, 45, 55));
    presetComboBox_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    presetComboBox_->getRenderer()->setTextColor(tgui::Color::White);
    presetComboBox_->getRenderer()->setRoundedBorderRadius(6);
    presetComboBox_->setTextSize(12);
    mainPanel_->add(presetComboBox_);
    
    // Save preset button
    savePresetButton_ = tgui::Button::create("Save");
    savePresetButton_->setSize("15%", "6%");
    savePresetButton_->setPosition("74%", "12%");
    savePresetButton_->getRenderer()->setBackgroundColor(tgui::Color(100, 100, 200));
    savePresetButton_->getRenderer()->setBackgroundColorHover(tgui::Color(120, 120, 220));
    savePresetButton_->getRenderer()->setTextColor(tgui::Color::White);
    savePresetButton_->getRenderer()->setRoundedBorderRadius(6);
    savePresetButton_->setTextSize(12);
    mainPanel_->add(savePresetButton_);
    
    // Spectrum visualization canvas
    spectrumCanvas_ = tgui::Canvas::create();
    spectrumCanvas_->setSize("94%", "15%");
    spectrumCanvas_->setPosition("3%", "20%");
    spectrumCanvas_->getRenderer()->setBackgroundColor(tgui::Color(15, 20, 30));
    spectrumCanvas_->getRenderer()->setBorderColor(tgui::Color(80, 120, 200));
    spectrumCanvas_->getRenderer()->setBorders(1);
    spectrumCanvas_->getRenderer()->setRoundedBorderRadius(8);
    mainPanel_->add(spectrumCanvas_);
    
    // Bands container
    bandsContainer_ = tgui::Panel::create();
    bandsContainer_->setSize("94%", "60%");
    bandsContainer_->setPosition("3%", "37%");
    bandsContainer_->getRenderer()->setBackgroundColor(tgui::Color(25, 30, 40, 100));
    bandsContainer_->getRenderer()->setRoundedBorderRadius(10);
    mainPanel_->add(bandsContainer_);
    
    gui_->add(mainPanel_);
}

void TGUIEqualizer::SetupStyling() {
    // Apply modern gradient-like styling
    auto applyGradientStyle = [](tgui::Widget::Ptr widget, tgui::Color baseColor) {
        if (auto button = std::dynamic_pointer_cast<tgui::Button>(widget)) {
            button->getRenderer()->setBackgroundColor(baseColor);
            tgui::Color hoverColor = baseColor;
            hoverColor.setRed(std::min(255, (int)hoverColor.getRed() + 30));
            hoverColor.setGreen(std::min(255, (int)hoverColor.getGreen() + 30));
            hoverColor.setBlue(std::min(255, (int)hoverColor.getBlue() + 30));
            button->getRenderer()->setBackgroundColorHover(hoverColor);
        }
    };
    
    applyGradientStyle(enableButton_, tgui::Color(50, 200, 50));
    applyGradientStyle(resetButton_, tgui::Color(200, 150, 50));
    applyGradientStyle(savePresetButton_, tgui::Color(100, 100, 200));
    applyGradientStyle(closeButton_, tgui::Color(220, 60, 60));
}

void TGUIEqualizer::SetupCallbacks() {
    // Enable/Disable button
    enableButton_->onPress([this]() {
        OnEnableToggled();
    });
    
    // Reset button
    resetButton_->onPress([this]() {
        OnResetClicked();
    });
    
    // Preset selection
    presetComboBox_->onItemSelect([this]() {
        OnPresetSelected();
    });
    
    // Save preset button
    savePresetButton_->onPress([this]() {
        OnSavePresetClicked();
    });
    
    // Close button
    closeButton_->onPress([this]() {
        OnCloseClicked();
    });
}

void TGUIEqualizer::CreateBandControls(int bandCount) {
    // Clear existing controls
    bandsContainer_->removeAllWidgets();
    bandControls_.clear();
    
    if (bandCount < 3) bandCount = 3;
    if (bandCount > 48) bandCount = 48;
    
    float bandWidth = 94.0f / bandCount;
    float bandSpacing = 1.0f;
    
    for (int i = 0; i < bandCount; ++i) {
        EqualizerBandUI band;
        band.index = i;
        band.frequency = GetFrequencyForBand(i, bandCount);
        band.gain = 0.0f;
        
        // Band panel
        band.bandPanel = tgui::Panel::create();
        band.bandPanel->setSize(tgui::String(std::to_string(bandWidth - bandSpacing) + "%"), "95%");
        band.bandPanel->setPosition(tgui::String(std::to_string(i * bandWidth + bandSpacing/2) + "%"), "2.5%");
        band.bandPanel->getRenderer()->setBackgroundColor(tgui::Color(30, 35, 45, 150));
        band.bandPanel->getRenderer()->setBorderColor(GetBandColor(i, bandCount));
        band.bandPanel->getRenderer()->setBorders(1);
        band.bandPanel->getRenderer()->setRoundedBorderRadius(6);
        
        // Frequency label
        band.frequencyLabel = tgui::Label::create(FormatFrequency(band.frequency));
        band.frequencyLabel->setSize("100%", "15%");
        band.frequencyLabel->setPosition("0%", "5%");
        band.frequencyLabel->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
        band.frequencyLabel->setTextSize(10);
        band.frequencyLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
        band.bandPanel->add(band.frequencyLabel);
        
        // Gain slider (vertical)
        band.gainSlider = tgui::Slider::create();
        band.gainSlider->setSize("60%", "65%");
        band.gainSlider->setPosition("20%", "20%");
        band.gainSlider->setMinimum(-20.0f);
        band.gainSlider->setMaximum(20.0f);
        band.gainSlider->setValue(0.0f);
        band.gainSlider->setVerticalScroll(true);
        band.gainSlider->getRenderer()->setTrackColor(tgui::Color(50, 55, 65));
        band.gainSlider->getRenderer()->setThumbColor(GetBandColor(i, bandCount));
        band.gainSlider->getRenderer()->setThumbColorHover(tgui::Color::White);
        band.gainSlider->getRenderer()->setBorderColor(tgui::Color(80, 90, 100));
        band.gainSlider->getRenderer()->setBorders(1);
        band.gainSlider->getRenderer()->setRoundedBorderRadius(4);
        band.bandPanel->add(band.gainSlider);
        
        // Gain label
        band.gainLabel = tgui::Label::create("0.0dB");
        band.gainLabel->setSize("100%", "15%");
        band.gainLabel->setPosition("0%", "87%");
        band.gainLabel->getRenderer()->setTextColor(tgui::Color::White);
        band.gainLabel->setTextSize(9);
        band.gainLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
        band.bandPanel->add(band.gainLabel);
        
        // Setup slider callback
        band.gainSlider->onValueChange([this, i](float value) {
            OnBandGainChanged(i, value);
        });
        
        bandsContainer_->add(band.bandPanel);
        bandControls_.push_back(band);
    }
}

void TGUIEqualizer::Show() {
    if (!initialized_ || isVisible_) {
        return;
    }
    
    isVisible_ = true;
    StartSlideAnimation(true);
}

void TGUIEqualizer::Hide() {
    if (!initialized_ || !isVisible_) {
        return;
    }
    
    isVisible_ = false;
    StartSlideAnimation(false);
}

void TGUIEqualizer::Toggle() {
    if (isVisible_) {
        Hide();
    } else {
        Show();
    }
}

void TGUIEqualizer::SetBandCount(int count) {
    CreateBandControls(count);
    
    // Update settings
    auto& settings = SettingsManager::getInstance();
    settings.setEqualizerBandCount(count);
}

void TGUIEqualizer::SetBandGain(int bandIndex, float gain) {
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bandControls_.size())) {
        bandControls_[bandIndex].gain = gain;
        bandControls_[bandIndex].gainSlider->setValue(gain);
        bandControls_[bandIndex].gainLabel->setText(FormatGain(gain));
        
        // Update settings
        auto& settings = SettingsManager::getInstance();
        settings.setEqualizerBand(bandIndex, bandControls_[bandIndex].frequency, gain);
    }
}

void TGUIEqualizer::ResetAllBands() {
    for (auto& band : bandControls_) {
        band.gain = 0.0f;
        band.gainSlider->setValue(0.0f);
        band.gainLabel->setText("0.0dB");
    }
    
    // Update settings
    auto& settings = SettingsManager::getInstance();
    for (size_t i = 0; i < bandControls_.size(); ++i) {
        settings.setEqualizerBand(i, bandControls_[i].frequency, 0.0f);
    }
}

void TGUIEqualizer::EnableEqualizer(bool enable) {
    equalizerEnabled_ = enable;
    enableButton_->setText(enable ? "ON" : "OFF");
    enableButton_->getRenderer()->setBackgroundColor(enable ? 
        tgui::Color(50, 200, 50) : tgui::Color(200, 50, 50));
    
    // Update settings
    auto& settings = SettingsManager::getInstance();
    settings.enableEqualizer(enable);
    
    if (equalizerToggleCallback_) {
        equalizerToggleCallback_(enable);
    }
}

void TGUIEqualizer::UpdateSpectrum(const std::vector<float>& spectrumData) {
    if (spectrumData.size() != spectrumData_.size()) {
        spectrumData_.resize(spectrumData.size());
        smoothedSpectrum_.resize(spectrumData.size());
    }
    
    spectrumData_ = spectrumData;
}

void TGUIEqualizer::Update(float deltaTime) {
    if (!initialized_) {
        return;
    }
    
    UpdateAnimation(deltaTime);
    
    if (isVisible_ && showSpectrum_) {
        SmoothSpectrum(deltaTime);
        RenderSpectrum();
    }
}

void TGUIEqualizer::OnBandGainChanged(int bandIndex, float gain) {
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bandControls_.size())) {
        bandControls_[bandIndex].gain = gain;
        bandControls_[bandIndex].gainLabel->setText(FormatGain(gain));
        
        // Update settings
        auto& settings = SettingsManager::getInstance();
        settings.setEqualizerBand(bandIndex, bandControls_[bandIndex].frequency, gain);
        
        if (bandChangedCallback_) {
            bandChangedCallback_(bandIndex, gain);
        }
    }
}

void TGUIEqualizer::OnEnableToggled() {
    EnableEqualizer(!equalizerEnabled_);
}

void TGUIEqualizer::OnResetClicked() {
    ResetAllBands();
}

void TGUIEqualizer::OnPresetSelected() {
    std::string selectedPreset = presetComboBox_->getSelectedItem().toStdString();
    if (!selectedPreset.empty()) {
        LoadPreset(selectedPreset);
    }
}

void TGUIEqualizer::OnSavePresetClicked() {
    // This would typically open a dialog to enter preset name
    // For now, save as "Custom"
    SavePreset("Custom");
}

void TGUIEqualizer::OnCloseClicked() {
    Hide();
}

void TGUIEqualizer::StartSlideAnimation(bool show) {
    isAnimating_ = true;
    slideAnimation_ = show ? 0.0f : 1.0f;
    
    if (show) {
        mainPanel_->setVisible(true);
    }
}

void TGUIEqualizer::UpdateAnimation(float deltaTime) {
    if (!isAnimating_) {
        return;
    }
    
    const float animationSpeed = 3.5f;
    
    if (isVisible_) {
        slideAnimation_ += deltaTime * animationSpeed;
        if (slideAnimation_ >= 1.0f) {
            slideAnimation_ = 1.0f;
            isAnimating_ = false;
        }
    } else {
        slideAnimation_ -= deltaTime * animationSpeed;
        if (slideAnimation_ <= 0.0f) {
            slideAnimation_ = 0.0f;
            isAnimating_ = false;
            mainPanel_->setVisible(false);
        }
    }
    
    // Apply smooth easing animation
    float easedProgress = slideAnimation_ * slideAnimation_ * (3.0f - 2.0f * slideAnimation_);
    
    // Slide in from the right
    float targetX = 63.0f; // 100% - 35% - 2% margin
    float startX = 100.0f;
    float currentX = startX - (startX - targetX) * easedProgress;
    
    mainPanel_->setPosition(tgui::String(std::to_string(currentX) + "%"), "5%");
    
    // Fade in
    tgui::Color bgColor = mainPanel_->getRenderer()->getBackgroundColor();
    bgColor.setAlpha(static_cast<tgui::Uint8>(245 * easedProgress));
    mainPanel_->getRenderer()->setBackgroundColor(bgColor);
}

void TGUIEqualizer::RenderSpectrum() {
    if (!spectrumCanvas_ || smoothedSpectrum_.empty()) {
        return;
    }
    
    spectrumCanvas_->clear(tgui::Color(15, 20, 30));
    
    float width = spectrumCanvas_->getSize().x;
    float height = spectrumCanvas_->getSize().y;
    float barWidth = width / smoothedSpectrum_.size();
    
    for (size_t i = 0; i < smoothedSpectrum_.size(); ++i) {
        float barHeight = smoothedSpectrum_[i] * height * 0.8f;
        float x = i * barWidth;
        float y = height - barHeight;
        
        // Create gradient color based on frequency and amplitude
        tgui::Color barColor;
        float hue = (float)i / smoothedSpectrum_.size();
        float intensity = smoothedSpectrum_[i];
        
        if (hue < 0.33f) { // Low frequencies - red to yellow
            barColor = tgui::Color(255, static_cast<tgui::Uint8>(hue * 3 * 255), 0, static_cast<tgui::Uint8>(intensity * 255));
        } else if (hue < 0.66f) { // Mid frequencies - yellow to green
            barColor = tgui::Color(static_cast<tgui::Uint8>((1.0f - (hue - 0.33f) * 3) * 255), 255, 0, static_cast<tgui::Uint8>(intensity * 255));
        } else { // High frequencies - green to blue
            barColor = tgui::Color(0, static_cast<tgui::Uint8>((1.0f - (hue - 0.66f) * 3) * 255), static_cast<tgui::Uint8>((hue - 0.66f) * 3 * 255), static_cast<tgui::Uint8>(intensity * 255));
        }
        
        // Draw spectrum bar
        sf::RectangleShape bar;
        bar.setPosition(x, y);
        bar.setSize(sf::Vector2f(barWidth - 1, barHeight));
        bar.setFillColor(sf::Color(barColor.getRed(), barColor.getGreen(), barColor.getBlue(), barColor.getAlpha()));
        spectrumCanvas_->draw(bar);
    }
    
    spectrumCanvas_->display();
}

void TGUIEqualizer::SmoothSpectrum(float deltaTime) {
    const float smoothingFactor = 8.0f * deltaTime;
    
    for (size_t i = 0; i < spectrumData_.size(); ++i) {
        float target = spectrumData_[i];
        smoothedSpectrum_[i] += (target - smoothedSpectrum_[i]) * smoothingFactor;
        
        // Ensure values stay in valid range
        smoothedSpectrum_[i] = std::max(0.0f, std::min(1.0f, smoothedSpectrum_[i]));
    }
}

void TGUIEqualizer::InitializeDefaultPresets() {
    presetComboBox_->addItem("Flat");
    presetComboBox_->addItem("Rock");
    presetComboBox_->addItem("Pop");
    presetComboBox_->addItem("Jazz");
    presetComboBox_->addItem("Classical");
    presetComboBox_->addItem("Electronic");
    presetComboBox_->addItem("Vocal");
    presetComboBox_->addItem("Bass Boost");
    presetComboBox_->addItem("Treble Boost");
    presetComboBox_->addItem("Custom");
}

void TGUIEqualizer::LoadPreset(const std::string& presetName) {
    std::vector<float> gains;
    
    if (presetName == "Flat") {
        gains.assign(bandControls_.size(), 0.0f);
    } else if (presetName == "Rock") {
        gains = {3.0f, 2.0f, 1.0f, 0.0f, -1.0f, -2.0f, 0.0f, 2.0f, 4.0f, 5.0f, 4.0f, 3.0f};
    } else if (presetName == "Pop") {
        gains = {1.0f, 2.0f, 3.0f, 2.0f, 0.0f, -1.0f, -1.0f, 0.0f, 2.0f, 3.0f, 2.0f, 1.0f};
    } else if (presetName == "Jazz") {
        gains = {2.0f, 1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f, -1.0f};
    } else if (presetName == "Classical") {
        gains = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f};
    } else if (presetName == "Electronic") {
        gains = {4.0f, 3.0f, 1.0f, 0.0f, -2.0f, -1.0f, 1.0f, 2.0f, 3.0f, 4.0f, 3.0f, 2.0f};
    } else if (presetName == "Vocal") {
        gains = {-2.0f, -1.0f, 0.0f, 2.0f, 4.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f, -1.0f, -2.0f};
    } else if (presetName == "Bass Boost") {
        gains = {6.0f, 5.0f, 4.0f, 2.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    } else if (presetName == "Treble Boost") {
        gains = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 6.0f};
    }
    
    ApplyPreset(gains);
}

void TGUIEqualizer::ApplyPreset(const std::vector<float>& gains) {
    for (size_t i = 0; i < gains.size() && i < bandControls_.size(); ++i) {
        SetBandGain(i, gains[i]);
    }
}

std::string TGUIEqualizer::FormatFrequency(float frequency) const {
    if (frequency >= 1000.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << (frequency / 1000.0f) << "k";
        return oss.str();
    } else {
        return std::to_string(static_cast<int>(frequency));
    }
}

std::string TGUIEqualizer::FormatGain(float gain) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    if (gain >= 0) oss << "+";
    oss << gain << "dB";
    return oss.str();
}

tgui::Color TGUIEqualizer::GetBandColor(int bandIndex, int totalBands) const {
    float hue = static_cast<float>(bandIndex) / totalBands;
    
    if (hue < 0.33f) { // Low frequencies - red to orange
        return tgui::Color(255, static_cast<tgui::Uint8>(hue * 3 * 128 + 127), 0);
    } else if (hue < 0.66f) { // Mid frequencies - orange to green
        return tgui::Color(static_cast<tgui::Uint8>((1.0f - (hue - 0.33f) * 3) * 255), 255, 0);
    } else { // High frequencies - green to blue
        return tgui::Color(0, static_cast<tgui::Uint8>((1.0f - (hue - 0.66f) * 3) * 255), static_cast<tgui::Uint8>((hue - 0.66f) * 3 * 255));
    }
}

float TGUIEqualizer::GetFrequencyForBand(int bandIndex, int totalBands) const {
    // Logarithmic frequency distribution from 31.25 Hz to 16 kHz
    float minFreq = 31.25f;
    float maxFreq = 16000.0f;
    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    
    float logFreq = logMin + (logMax - logMin) * bandIndex / (totalBands - 1);
    return std::pow(10.0f, logFreq);
}

void TGUIEqualizer::HandleKeyPress(const tgui::Event& event) {
    if (event.type == tgui::Event::Type::KeyPressed) {
        if (event.key.code == tgui::Event::KeyboardKey::Escape) {
            Hide();
        } else if (event.key.code == tgui::Event::KeyboardKey::R && event.key.control) {
            ResetAllBands();
        } else if (event.key.code == tgui::Event::KeyboardKey::E && event.key.control) {
            OnEnableToggled();
        }
    }
}

std::vector<float> TGUIEqualizer::GetCurrentGains() const {
    std::vector<float> gains;
    for (const auto& band : bandControls_) {
        gains.push_back(band.gain);
    }
    return gains;
}

std::vector<std::string> TGUIEqualizer::GetAvailablePresets() const {
    std::vector<std::string> presets;
    for (size_t i = 0; i < presetComboBox_->getItemCount(); ++i) {
        presets.push_back(presetComboBox_->getItemByIndex(i).toStdString());
    }
    return presets;
}

void TGUIEqualizer::SavePreset(const std::string& presetName) {
    // This would typically save to settings or a preset file
    // For now, just log the action
    ErrorHandler::getInstance().logInfo("Saved equalizer preset: " + presetName);
}

} // namespace Lyricstator
