#pragma once
#include <TGUI/TGUI.hpp>
#include <vector>
#include <string>
#include <functional>

namespace Lyricstator {

struct EqualizerBand;

class TGUIEqualizer {
public:
    TGUIEqualizer();
    ~TGUIEqualizer();
    
    // Initialization
    bool Initialize(tgui::Gui* gui);
    void Shutdown();
    
    // Visibility control
    void Show();
    void Hide();
    void Toggle();
    bool IsVisible() const { return isVisible_; }
    
    // Equalizer control
    void SetBandCount(int count);
    void SetBandGain(int bandIndex, float gain);
    void SetBandFrequency(int bandIndex, float frequency);
    void ResetAllBands();
    void EnableEqualizer(bool enable);
    bool IsEqualizerEnabled() const { return equalizerEnabled_; }
    
    // Presets
    void LoadPreset(const std::string& presetName);
    void SavePreset(const std::string& presetName);
    std::vector<std::string> GetAvailablePresets() const;
    
    // Spectrum visualization
    void UpdateSpectrum(const std::vector<float>& spectrumData);
    void SetSpectrumVisible(bool visible) { showSpectrum_ = visible; }
    
    // Callbacks
    void SetBandChangedCallback(std::function<void(int, float)> callback) { bandChangedCallback_ = callback; }
    void SetEqualizerToggleCallback(std::function<void(bool)> callback) { equalizerToggleCallback_ = callback; }
    
    // Update
    void Update(float deltaTime);
    
    // Event handling
    void HandleKeyPress(const tgui::Event& event);
    
private:
    tgui::Gui* gui_;
    bool initialized_;
    bool isVisible_;
    bool equalizerEnabled_;
    bool showSpectrum_;
    
    // UI Components
    tgui::Panel::Ptr mainPanel_;
    tgui::Label::Ptr titleLabel_;
    tgui::Button::Ptr closeButton_;
    tgui::Button::Ptr enableButton_;
    tgui::Button::Ptr resetButton_;
    tgui::ComboBox::Ptr presetComboBox_;
    tgui::Button::Ptr savePresetButton_;
    
    // Equalizer bands
    struct EqualizerBandUI {
        tgui::Slider::Ptr gainSlider;
        tgui::Label::Ptr frequencyLabel;
        tgui::Label::Ptr gainLabel;
        tgui::Panel::Ptr bandPanel;
        float frequency;
        float gain;
        int index;
    };
    
    std::vector<EqualizerBandUI> bandControls_;
    tgui::Panel::Ptr bandsContainer_;
    
    // Spectrum visualization
    tgui::Canvas::Ptr spectrumCanvas_;
    std::vector<float> spectrumData_;
    std::vector<float> smoothedSpectrum_;
    
    // Animation and styling
    float slideAnimation_;
    bool isAnimating_;
    
    // Callbacks
    std::function<void(int, float)> bandChangedCallback_;
    std::function<void(bool)> equalizerToggleCallback_;
    
    // Internal methods
    void CreateUI();
    void SetupStyling();
    void SetupCallbacks();
    void CreateBandControls(int bandCount);
    void UpdateBandLayout();
    void OnBandGainChanged(int bandIndex, float gain);
    void OnEnableToggled();
    void OnResetClicked();
    void OnPresetSelected();
    void OnSavePresetClicked();
    void OnCloseClicked();
    
    // Spectrum rendering
    void RenderSpectrum();
    void SmoothSpectrum(float deltaTime);
    
    // Preset management
    void InitializeDefaultPresets();
    void ApplyPreset(const std::vector<float>& gains);
    std::vector<float> GetCurrentGains() const;
    
    // Animation helpers
    void StartSlideAnimation(bool show);
    void UpdateAnimation(float deltaTime);
    
    // Utility functions
    std::string FormatFrequency(float frequency) const;
    std::string FormatGain(float gain) const;
    tgui::Color GetBandColor(int bandIndex, int totalBands) const;
    float GetFrequencyForBand(int bandIndex, int totalBands) const;
};

} // namespace Lyricstator
