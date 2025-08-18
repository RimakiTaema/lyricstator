#pragma once

#include <TGUI/TGUI.hpp>
#include <memory>
#include <vector>
#include <string>

class AssetManager;

/**
 * Simple and beautiful TGUI-based resource pack selector
 * Easy to use: just call toggle() to show/hide
 */
class TGUIResourcePackGUI {
public:
    TGUIResourcePackGUI(AssetManager* assetManager);
    ~TGUIResourcePackGUI();

    // Simple interface - easy to understand
    void initialize(tgui::Gui& gui);
    void show();
    void hide();
    void toggle();
    bool isVisible() const { return m_visible; }

    void update();

private:
    // Clean internal methods
    void createInterface(tgui::Gui& gui);
    void loadResourcePacks();
    void onPackSelected(const std::string& packName);
    void createPackPreview(const std::string& packName);
    void setupStyling();  // Added styling method for beautiful appearance

    // Core components - easy to maintain
    AssetManager* m_assetManager;
    tgui::Panel::Ptr m_mainPanel;
    tgui::ScrollablePanel::Ptr m_packList;
    tgui::Panel::Ptr m_previewPanel;
    tgui::Label::Ptr m_titleLabel;
    tgui::Button::Ptr m_closeButton;  // Added close button
    
    // Simple state management
    bool m_visible;
    std::vector<std::string> m_availablePacks;
    std::string m_currentPack;
    
    // Animation support - smooth and beautiful
    float m_animationTime;  // Added animation timing
};
