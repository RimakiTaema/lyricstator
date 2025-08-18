#include "TGUIKeybindEditor.h"
#include "core/SettingsManager.h"
#include "utils/ErrorHandler.h"
#include <algorithm>
#include <sstream>

namespace Lyricstator {

TGUIKeybindEditor::TGUIKeybindEditor()
    : gui_(nullptr)
    , initialized_(false)
    , isVisible_(false)
    , isCapturingKey_(false)
    , slideAnimation_(0.0f)
    , isAnimating_(false)
{
}

TGUIKeybindEditor::~TGUIKeybindEditor() {
    Shutdown();
}

bool TGUIKeybindEditor::Initialize(tgui::Gui* gui) {
    if (initialized_ || !gui) {
        return false;
    }
    
    gui_ = gui;
    
    CreateUI();
    SetupStyling();
    SetupCallbacks();
    RefreshKeybindList();
    
    initialized_ = true;
    ErrorHandler::getInstance().logInfo("TGUIKeybindEditor initialized successfully");
    return true;
}

void TGUIKeybindEditor::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    Hide();
    
    if (gui_ && mainPanel_) {
        gui_->remove(mainPanel_);
    }
    
    initialized_ = false;
}

void TGUIKeybindEditor::CreateUI() {
    // Main panel - centered on screen
    mainPanel_ = tgui::Panel::create();
    mainPanel_->setSize("70%", "80%");
    mainPanel_->setPosition("15%", "10%");
    mainPanel_->getRenderer()->setBackgroundColor(tgui::Color(25, 30, 40, 250));
    mainPanel_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    mainPanel_->getRenderer()->setBorders(2);
    mainPanel_->getRenderer()->setRoundedBorderRadius(12);
    mainPanel_->setVisible(false);
    
    // Title bar
    titleLabel_ = tgui::Label::create("Keyboard Shortcuts");
    titleLabel_->setSize("60%", "8%");
    titleLabel_->setPosition("3%", "2%");
    titleLabel_->getRenderer()->setTextColor(tgui::Color::White);
    titleLabel_->setTextSize(24);
    mainPanel_->add(titleLabel_);
    
    // Close button
    closeButton_ = tgui::Button::create("×");
    closeButton_->setSize("6%", "8%");
    closeButton_->setPosition("91%", "2%");
    closeButton_->getRenderer()->setBackgroundColor(tgui::Color(220, 60, 60));
    closeButton_->getRenderer()->setBackgroundColorHover(tgui::Color(255, 80, 80));
    closeButton_->getRenderer()->setTextColor(tgui::Color::White);
    closeButton_->getRenderer()->setRoundedBorderRadius(8);
    closeButton_->setTextSize(18);
    mainPanel_->add(closeButton_);
    
    // Search box
    searchBox_ = tgui::EditBox::create();
    searchBox_->setSize("40%", "6%");
    searchBox_->setPosition("3%", "12%");
    searchBox_->setDefaultText("Search shortcuts...");
    searchBox_->getRenderer()->setBackgroundColor(tgui::Color(40, 45, 55));
    searchBox_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    searchBox_->getRenderer()->setTextColor(tgui::Color::White);
    searchBox_->getRenderer()->setRoundedBorderRadius(6);
    searchBox_->setTextSize(14);
    mainPanel_->add(searchBox_);
    
    // Category filter
    categoryFilter_ = tgui::ComboBox::create();
    categoryFilter_->setSize("25%", "6%");
    categoryFilter_->setPosition("45%", "12%");
    categoryFilter_->addItem("All Categories");
    categoryFilter_->addItem("Playback");
    categoryFilter_->addItem("Navigation");
    categoryFilter_->addItem("Interface");
    categoryFilter_->addItem("Audio");
    categoryFilter_->addItem("File");
    categoryFilter_->setSelectedItem("All Categories");
    categoryFilter_->getRenderer()->setBackgroundColor(tgui::Color(40, 45, 55));
    categoryFilter_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    categoryFilter_->getRenderer()->setTextColor(tgui::Color::White);
    categoryFilter_->getRenderer()->setRoundedBorderRadius(6);
    categoryFilter_->setTextSize(14);
    mainPanel_->add(categoryFilter_);
    
    // Reset button
    resetButton_ = tgui::Button::create("Reset to Defaults");
    resetButton_->setSize("15%", "6%");
    resetButton_->setPosition("72%", "12%");
    resetButton_->getRenderer()->setBackgroundColor(tgui::Color(200, 150, 50));
    resetButton_->getRenderer()->setBackgroundColorHover(tgui::Color(220, 170, 70));
    resetButton_->getRenderer()->setTextColor(tgui::Color::White);
    resetButton_->getRenderer()->setRoundedBorderRadius(6);
    resetButton_->setTextSize(12);
    mainPanel_->add(resetButton_);
    
    // Keybind list view
    keybindListView_ = tgui::ListView::create();
    keybindListView_->setSize("94%", "65%");
    keybindListView_->setPosition("3%", "20%");
    keybindListView_->addColumn("Action", 300);
    keybindListView_->addColumn("Shortcut", 200);
    keybindListView_->addColumn("Category", 150);
    keybindListView_->addColumn("Description", 350);
    keybindListView_->getRenderer()->setBackgroundColor(tgui::Color(30, 35, 45));
    keybindListView_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    keybindListView_->getRenderer()->setTextColor(tgui::Color::White);
    keybindListView_->getRenderer()->setHeaderBackgroundColor(tgui::Color(50, 60, 80));
    keybindListView_->getRenderer()->setHeaderTextColor(tgui::Color::White);
    keybindListView_->getRenderer()->setSelectedBackgroundColor(tgui::Color(100, 150, 255, 100));
    keybindListView_->getRenderer()->setRoundedBorderRadius(8);
    keybindListView_->setTextSize(13);
    mainPanel_->add(keybindListView_);
    
    // Instruction label
    instructionLabel_ = tgui::Label::create("Double-click a shortcut to change it, or select and press Enter");
    instructionLabel_->setSize("60%", "5%");
    instructionLabel_->setPosition("3%", "87%");
    instructionLabel_->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
    instructionLabel_->setTextSize(12);
    mainPanel_->add(instructionLabel_);
    
    // Save button
    saveButton_ = tgui::Button::create("Save Changes");
    saveButton_->setSize("15%", "5%");
    saveButton_->setPosition("82%", "87%");
    saveButton_->getRenderer()->setBackgroundColor(tgui::Color(50, 200, 50));
    saveButton_->getRenderer()->setBackgroundColorHover(tgui::Color(70, 220, 70));
    saveButton_->getRenderer()->setTextColor(tgui::Color::White);
    saveButton_->getRenderer()->setRoundedBorderRadius(6);
    saveButton_->setTextSize(12);
    mainPanel_->add(saveButton_);
    
    // Key capture panel (initially hidden)
    capturePanel_ = tgui::Panel::create();
    capturePanel_->setSize("50%", "30%");
    capturePanel_->setPosition("25%", "35%");
    capturePanel_->getRenderer()->setBackgroundColor(tgui::Color(40, 50, 70, 240));
    capturePanel_->getRenderer()->setBorderColor(tgui::Color(255, 200, 100));
    capturePanel_->getRenderer()->setBorders(3);
    capturePanel_->getRenderer()->setRoundedBorderRadius(15);
    capturePanel_->setVisible(false);
    
    // Capture instruction label
    captureLabel_ = tgui::Label::create("Press the new key combination\nfor this action...");
    captureLabel_->setSize("80%", "50%");
    captureLabel_->setPosition("10%", "20%");
    captureLabel_->getRenderer()->setTextColor(tgui::Color::White);
    captureLabel_->setTextSize(16);
    captureLabel_->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
    captureLabel_->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
    capturePanel_->add(captureLabel_);
    
    // Cancel capture button
    cancelCaptureButton_ = tgui::Button::create("Cancel");
    cancelCaptureButton_->setSize("30%", "20%");
    cancelCaptureButton_->setPosition("35%", "75%");
    cancelCaptureButton_->getRenderer()->setBackgroundColor(tgui::Color(200, 100, 100));
    cancelCaptureButton_->getRenderer()->setBackgroundColorHover(tgui::Color(220, 120, 120));
    cancelCaptureButton_->getRenderer()->setTextColor(tgui::Color::White);
    cancelCaptureButton_->getRenderer()->setRoundedBorderRadius(8);
    cancelCaptureButton_->setTextSize(14);
    capturePanel_->add(cancelCaptureButton_);
    
    mainPanel_->add(capturePanel_);
    gui_->add(mainPanel_);
}

void TGUIKeybindEditor::SetupStyling() {
    // Apply consistent modern styling
    auto applyModernStyle = [](tgui::Widget::Ptr widget) {
        if (auto button = std::dynamic_pointer_cast<tgui::Button>(widget)) {
            button->getRenderer()->setRoundedBorderRadius(8);
        } else if (auto editBox = std::dynamic_pointer_cast<tgui::EditBox>(widget)) {
            editBox->getRenderer()->setRoundedBorderRadius(6);
        } else if (auto comboBox = std::dynamic_pointer_cast<tgui::ComboBox>(widget)) {
            comboBox->getRenderer()->setRoundedBorderRadius(6);
        }
    };
    
    applyModernStyle(searchBox_);
    applyModernStyle(categoryFilter_);
    applyModernStyle(resetButton_);
    applyModernStyle(saveButton_);
    applyModernStyle(closeButton_);
    applyModernStyle(cancelCaptureButton_);
}

void TGUIKeybindEditor::SetupCallbacks() {
    // Search box
    searchBox_->onTextChange([this]() {
        OnSearchTextChanged();
    });
    
    // Category filter
    categoryFilter_->onItemSelect([this]() {
        OnCategoryFilterChanged();
    });
    
    // Buttons
    resetButton_->onPress([this]() {
        OnResetClicked();
    });
    
    saveButton_->onPress([this]() {
        OnSaveClicked();
    });
    
    closeButton_->onPress([this]() {
        OnCloseClicked();
    });
    
    cancelCaptureButton_->onPress([this]() {
        OnCancelCapture();
    });
    
    // Keybind list
    keybindListView_->onItemSelect([this](int index) {
        OnKeybindSelected(index);
    });
    
    keybindListView_->onDoubleClick([this](int index) {
        OnKeybindDoubleClicked(index);
    });
}

void TGUIKeybindEditor::Show() {
    if (!initialized_ || isVisible_) {
        return;
    }
    
    isVisible_ = true;
    RefreshKeybindList();
    StartSlideAnimation(true);
}

void TGUIKeybindEditor::Hide() {
    if (!initialized_ || !isVisible_) {
        return;
    }
    
    if (isCapturingKey_) {
        StopKeyCapture();
    }
    
    isVisible_ = false;
    StartSlideAnimation(false);
}

void TGUIKeybindEditor::Toggle() {
    if (isVisible_) {
        Hide();
    } else {
        Show();
    }
}

void TGUIKeybindEditor::RefreshKeybindList() {
    if (!initialized_) {
        return;
    }
    
    auto& settings = SettingsManager::getInstance();
    currentKeybinds_ = settings.getKeyBindings();
    FilterKeybinds();
}

void TGUIKeybindEditor::FilterKeybinds() {
    filteredKeybinds_.clear();
    
    for (const auto& binding : currentKeybinds_) {
        bool matchesSearch = currentSearchQuery_.empty() || MatchesSearch(binding, currentSearchQuery_);
        bool matchesCategory = currentCategory_.empty() || currentCategory_ == "All Categories" || 
                              MatchesCategory(binding, currentCategory_);
        
        if (matchesSearch && matchesCategory) {
            filteredKeybinds_.push_back(binding);
        }
    }
    
    PopulateKeybindList();
}

void TGUIKeybindEditor::PopulateKeybindList() {
    keybindListView_->removeAllItems();
    
    for (const auto& binding : filteredKeybinds_) {
        std::vector<tgui::String> columns = {
            GetActionDisplayName(binding.action),
            GetKeybindString(binding),
            GetActionCategory(binding.action),
            binding.description
        };
        
        keybindListView_->addItem(columns);
        
        // Color code by category
        int itemIndex = keybindListView_->getItemCount() - 1;
        tgui::Color categoryColor = GetCategoryColor(GetActionCategory(binding.action));
        keybindListView_->setItemColor(itemIndex, categoryColor);
    }
}

void TGUIKeybindEditor::OnKeybindSelected(int index) {
    if (index >= 0 && index < static_cast<int>(filteredKeybinds_.size())) {
        const auto& binding = filteredKeybinds_[index];
        instructionLabel_->setText("Selected: " + GetActionDisplayName(binding.action) + 
                                 " - Double-click to change shortcut");
    }
}

void TGUIKeybindEditor::OnKeybindDoubleClicked(int index) {
    if (index >= 0 && index < static_cast<int>(filteredKeybinds_.size())) {
        const auto& binding = filteredKeybinds_[index];
        StartKeyCapture(binding.action);
    }
}

void TGUIKeybindEditor::OnSearchTextChanged() {
    currentSearchQuery_ = searchBox_->getText().toStdString();
    FilterKeybinds();
}

void TGUIKeybindEditor::OnCategoryFilterChanged() {
    currentCategory_ = categoryFilter_->getSelectedItem().toStdString();
    FilterKeybinds();
}

void TGUIKeybindEditor::OnResetClicked() {
    ResetToDefaults();
}

void TGUIKeybindEditor::OnSaveClicked() {
    SaveKeybinds();
}

void TGUIKeybindEditor::OnCloseClicked() {
    Hide();
}

void TGUIKeybindEditor::StartKeyCapture(const std::string& action) {
    isCapturingKey_ = true;
    capturingAction_ = action;
    
    captureLabel_->setText("Press the new key combination for:\n" + GetActionDisplayName(action));
    capturePanel_->setVisible(true);
    
    // Disable main panel interaction
    keybindListView_->setEnabled(false);
    searchBox_->setEnabled(false);
    categoryFilter_->setEnabled(false);
    resetButton_->setEnabled(false);
    saveButton_->setEnabled(false);
}

void TGUIKeybindEditor::StopKeyCapture() {
    isCapturingKey_ = false;
    capturingAction_.clear();
    capturePanel_->setVisible(false);
    
    // Re-enable main panel interaction
    keybindListView_->setEnabled(true);
    searchBox_->setEnabled(true);
    categoryFilter_->setEnabled(true);
    resetButton_->setEnabled(true);
    saveButton_->setEnabled(true);
}

void TGUIKeybindEditor::OnKeyCaptured(SDL_Scancode key, bool ctrl, bool shift, bool alt) {
    if (!isCapturingKey_ || capturingAction_.empty()) {
        return;
    }
    
    // Check for conflicts
    if (IsKeybindConflict(key, ctrl, shift, alt, capturingAction_)) {
        // Find conflicting action
        auto& settings = SettingsManager::getInstance();
        auto* conflictBinding = settings.findKeyBinding(key, ctrl, shift, alt);
        if (conflictBinding) {
            ShowConflictWarning(conflictBinding->action);
            return;
        }
    }
    
    // Apply the new keybinding
    auto& settings = SettingsManager::getInstance();
    settings.setKeyBinding(capturingAction_, key, ctrl, shift, alt);
    
    if (keybindChangedCallback_) {
        keybindChangedCallback_(capturingAction_, key, ctrl, shift, alt);
    }
    
    StopKeyCapture();
    RefreshKeybindList();
    
    ErrorHandler::getInstance().logInfo("Keybinding changed: " + capturingAction_ + " -> " + 
                                       settings.getKeyBindingString({key, ctrl, shift, alt, capturingAction_, ""}));
}

void TGUIKeybindEditor::OnCancelCapture() {
    StopKeyCapture();
}

void TGUIKeybindEditor::ResetToDefaults() {
    auto& settings = SettingsManager::getInstance();
    
    // Clear current keybindings and reinitialize defaults
    settings.getKeyBindings().clear();
    settings.resetToDefaults();
    
    RefreshKeybindList();
    ErrorHandler::getInstance().logInfo("Keybindings reset to defaults");
}

void TGUIKeybindEditor::SaveKeybinds() {
    auto& settings = SettingsManager::getInstance();
    if (settings.saveSettings()) {
        instructionLabel_->setText("Keybindings saved successfully!");
        ErrorHandler::getInstance().logInfo("Keybindings saved to settings file");
    } else {
        instructionLabel_->setText("Failed to save keybindings!");
        ErrorHandler::getInstance().logError("Failed to save keybindings to settings file");
    }
}

void TGUIKeybindEditor::Update(float deltaTime) {
    if (!initialized_) {
        return;
    }
    
    UpdateAnimation(deltaTime);
}

void TGUIKeybindEditor::HandleKeyPress(const tgui::Event& event) {
    if (event.type == tgui::Event::Type::KeyPressed) {
        if (isCapturingKey_) {
            // Capture the key combination
            SDL_Scancode scancode = static_cast<SDL_Scancode>(event.key.code);
            bool ctrl = event.key.control;
            bool shift = event.key.shift;
            bool alt = event.key.alt;
            
            // Don't capture escape (used for canceling)
            if (scancode != SDL_SCANCODE_ESCAPE) {
                OnKeyCaptured(scancode, ctrl, shift, alt);
            } else {
                OnCancelCapture();
            }
        } else {
            // Normal key handling
            if (event.key.code == tgui::Event::KeyboardKey::Escape) {
                Hide();
            } else if (event.key.code == tgui::Event::KeyboardKey::Enter) {
                int selectedIndex = keybindListView_->getSelectedItemIndex();
                if (selectedIndex >= 0) {
                    OnKeybindDoubleClicked(selectedIndex);
                }
            }
        }
    }
}

std::string TGUIKeybindEditor::GetKeybindString(const KeyBinding& binding) const {
    auto& settings = SettingsManager::getInstance();
    return settings.getKeyBindingString(binding);
}

std::string TGUIKeybindEditor::GetActionCategory(const std::string& action) const {
    if (action.find("play") != std::string::npos || action.find("pause") != std::string::npos || 
        action.find("stop") != std::string::npos || action.find("seek") != std::string::npos) {
        return "Playback";
    } else if (action.find("volume") != std::string::npos || action.find("equalizer") != std::string::npos) {
        return "Audio";
    } else if (action.find("toggle") != std::string::npos || action.find("gui") != std::string::npos ||
               action.find("browser") != std::string::npos || action.find("fullscreen") != std::string::npos) {
        return "Interface";
    } else if (action.find("open") != std::string::npos || action.find("search") != std::string::npos) {
        return "File";
    } else {
        return "Navigation";
    }
}

std::string TGUIKeybindEditor::GetActionDisplayName(const std::string& action) const {
    // Convert action names to user-friendly display names
    if (action == "play_pause") return "Play/Pause";
    if (action == "stop") return "Stop";
    if (action == "toggle_resource_pack_gui") return "Toggle Resource Packs";
    if (action == "toggle_song_browser") return "Toggle Song Browser";
    if (action == "toggle_equalizer") return "Toggle Equalizer";
    if (action == "open_settings") return "Open Settings";
    if (action == "search_songs") return "Search Songs";
    if (action == "open_file") return "Open File";
    if (action == "seek_backward") return "Seek Backward";
    if (action == "seek_forward") return "Seek Forward";
    if (action == "volume_up") return "Volume Up";
    if (action == "volume_down") return "Volume Down";
    if (action == "toggle_fullscreen") return "Toggle Fullscreen";
    
    // Default: capitalize and replace underscores
    std::string displayName = action;
    std::replace(displayName.begin(), displayName.end(), '_', ' ');
    if (!displayName.empty()) {
        displayName[0] = std::toupper(displayName[0]);
    }
    return displayName;
}

bool TGUIKeybindEditor::MatchesSearch(const KeyBinding& binding, const std::string& query) const {
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    auto toLower = [](const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    };
    
    return toLower(GetActionDisplayName(binding.action)).find(lowerQuery) != std::string::npos ||
           toLower(binding.description).find(lowerQuery) != std::string::npos ||
           toLower(GetKeybindString(binding)).find(lowerQuery) != std::string::npos;
}

bool TGUIKeybindEditor::MatchesCategory(const KeyBinding& binding, const std::string& category) const {
    return GetActionCategory(binding.action) == category;
}

tgui::Color TGUIKeybindEditor::GetCategoryColor(const std::string& category) const {
    if (category == "Playback") return tgui::Color(100, 200, 100, 50);
    if (category == "Audio") return tgui::Color(200, 150, 100, 50);
    if (category == "Interface") return tgui::Color(100, 150, 200, 50);
    if (category == "File") return tgui::Color(200, 100, 150, 50);
    if (category == "Navigation") return tgui::Color(150, 100, 200, 50);
    return tgui::Color(100, 100, 100, 50);
}

void TGUIKeybindEditor::StartSlideAnimation(bool show) {
    isAnimating_ = true;
    slideAnimation_ = show ? 0.0f : 1.0f;
    
    if (show) {
        mainPanel_->setVisible(true);
    }
}

void TGUIKeybindEditor::UpdateAnimation(float deltaTime) {
    if (!isAnimating_) {
        return;
    }
    
    const float animationSpeed = 3.0f;
    
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
    
    // Scale and fade animation
    float scale = 0.8f + 0.2f * easedProgress;
    mainPanel_->setScale(scale, scale);
    
    // Fade in
    tgui::Color bgColor = mainPanel_->getRenderer()->getBackgroundColor();
    bgColor.setAlpha(static_cast<tgui::Uint8>(250 * easedProgress));
    mainPanel_->getRenderer()->setBackgroundColor(bgColor);
}

bool TGUIKeybindEditor::IsKeybindConflict(SDL_Scancode key, bool ctrl, bool shift, bool alt, const std::string& excludeAction) const {
    auto& settings = SettingsManager::getInstance();
    auto* existingBinding = settings.findKeyBinding(key, ctrl, shift, alt);
    return existingBinding && existingBinding->action != excludeAction;
}

void TGUIKeybindEditor::ShowConflictWarning(const std::string& conflictingAction) {
    std::string message = "This key combination is already used by: " + GetActionDisplayName(conflictingAction);
    captureLabel_->setText(message + "\n\nPress a different key combination or Cancel");
    ErrorHandler::getInstance().logWarning("Keybind conflict detected: " + conflictingAction);
}

} // namespace Lyricstator
