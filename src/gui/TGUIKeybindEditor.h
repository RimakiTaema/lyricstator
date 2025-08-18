#pragma once
#include <TGUI/TGUI.hpp>
#include <vector>
#include <string>
#include <functional>
#include <SDL.h>

namespace Lyricstator {

struct KeyBinding;

class TGUIKeybindEditor {
public:
    TGUIKeybindEditor();
    ~TGUIKeybindEditor();
    
    // Initialization
    bool Initialize(tgui::Gui* gui);
    void Shutdown();
    
    // Visibility control
    void Show();
    void Hide();
    void Toggle();
    bool IsVisible() const { return isVisible_; }
    
    // Keybind management
    void RefreshKeybindList();
    void ResetToDefaults();
    void SaveKeybinds();
    
    // Callbacks
    void SetKeybindChangedCallback(std::function<void(const std::string&, SDL_Scancode, bool, bool, bool)> callback) { 
        keybindChangedCallback_ = callback; 
    }
    
    // Update
    void Update(float deltaTime);
    
    // Event handling
    void HandleKeyPress(const tgui::Event& event);
    
private:
    tgui::Gui* gui_;
    bool initialized_;
    bool isVisible_;
    bool isCapturingKey_;
    std::string capturingAction_;
    
    // UI Components
    tgui::Panel::Ptr mainPanel_;
    tgui::Label::Ptr titleLabel_;
    tgui::Button::Ptr closeButton_;
    tgui::Button::Ptr resetButton_;
    tgui::Button::Ptr saveButton_;
    tgui::ListView::Ptr keybindListView_;
    tgui::Label::Ptr instructionLabel_;
    tgui::Panel::Ptr capturePanel_;
    tgui::Label::Ptr captureLabel_;
    tgui::Button::Ptr cancelCaptureButton_;
    
    // Search and filter
    tgui::EditBox::Ptr searchBox_;
    tgui::ComboBox::Ptr categoryFilter_;
    
    // Data
    std::vector<KeyBinding> currentKeybinds_;
    std::vector<KeyBinding> filteredKeybinds_;
    std::string currentSearchQuery_;
    std::string currentCategory_;
    
    // Animation
    float slideAnimation_;
    bool isAnimating_;
    
    // Callbacks
    std::function<void(const std::string&, SDL_Scancode, bool, bool, bool)> keybindChangedCallback_;
    
    // Internal methods
    void CreateUI();
    void SetupStyling();
    void SetupCallbacks();
    void PopulateKeybindList();
    void FilterKeybinds();
    void OnKeybindSelected(int index);
    void OnKeybindDoubleClicked(int index);
    void OnSearchTextChanged();
    void OnCategoryFilterChanged();
    void OnResetClicked();
    void OnSaveClicked();
    void OnCloseClicked();
    
    // Key capture
    void StartKeyCapture(const std::string& action);
    void StopKeyCapture();
    void OnKeyCaptured(SDL_Scancode key, bool ctrl, bool shift, bool alt);
    void OnCancelCapture();
    
    // Utility functions
    std::string GetKeybindString(const KeyBinding& binding) const;
    std::string GetActionCategory(const std::string& action) const;
    std::string GetActionDisplayName(const std::string& action) const;
    bool MatchesSearch(const KeyBinding& binding, const std::string& query) const;
    bool MatchesCategory(const KeyBinding& binding, const std::string& category) const;
    tgui::Color GetCategoryColor(const std::string& category) const;
    
    // Animation helpers
    void StartSlideAnimation(bool show);
    void UpdateAnimation(float deltaTime);
    
    // Validation
    bool IsKeybindConflict(SDL_Scancode key, bool ctrl, bool shift, bool alt, const std::string& excludeAction) const;
    void ShowConflictWarning(const std::string& conflictingAction);
};

} // namespace Lyricstator
