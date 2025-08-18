#include "TGUIResourcePackGUI.h"
#include "../core/AssetManager.h"
#include <filesystem>
#include <cmath>

TGUIResourcePackGUI::TGUIResourcePackGUI(AssetManager* assetManager)
    : m_assetManager(assetManager), m_visible(false), m_animationTime(0.0f) {
}

TGUIResourcePackGUI::~TGUIResourcePackGUI() = default;

void TGUIResourcePackGUI::initialize(tgui::Gui& gui) {
    createInterface(gui);
    setupStyling();
    loadResourcePacks();
}

void TGUIResourcePackGUI::createInterface(tgui::Gui& gui) {
    m_mainPanel = tgui::Panel::create();
    m_mainPanel->setSize("70%", "80%");
    m_mainPanel->setPosition("15%", "10%");
    m_mainPanel->getRenderer()->setBackgroundColor(tgui::Color(25, 30, 45, 250));
    m_mainPanel->getRenderer()->setBorderColor(tgui::Color(100, 150, 200));
    m_mainPanel->getRenderer()->setBorders(3);
    m_mainPanel->getRenderer()->setRoundedBorderRadius(15);
    m_mainPanel->setVisible(false);

    m_titleLabel = tgui::Label::create("🎨 Resource Packs");
    m_titleLabel->setPosition("5%", "3%");
    m_titleLabel->setTextSize(28);
    m_titleLabel->getRenderer()->setTextColor(tgui::Color(200, 220, 255));
    m_mainPanel->add(m_titleLabel);

    m_closeButton = tgui::Button::create("✕");
    m_closeButton->setSize("40", "40");
    m_closeButton->setPosition("92%", "3%");
    m_closeButton->getRenderer()->setBackgroundColor(tgui::Color(180, 50, 50));
    m_closeButton->getRenderer()->setBackgroundColorHover(tgui::Color(220, 70, 70));
    m_closeButton->getRenderer()->setTextColor(tgui::Color::White);
    m_closeButton->getRenderer()->setRoundedBorderRadius(20);
    m_closeButton->onPress([this] { hide(); });
    m_mainPanel->add(m_closeButton);

    m_packList = tgui::ScrollablePanel::create();
    m_packList->setSize("45%", "75%");
    m_packList->setPosition("5%", "18%");
    m_packList->getRenderer()->setBackgroundColor(tgui::Color(15, 20, 35));
    m_packList->getRenderer()->setBorderColor(tgui::Color(80, 100, 140));
    m_packList->getRenderer()->setBorders(2);
    m_packList->getRenderer()->setRoundedBorderRadius(10);
    m_mainPanel->add(m_packList);

    m_previewPanel = tgui::Panel::create();
    m_previewPanel->setSize("45%", "75%");
    m_previewPanel->setPosition("52%", "18%");
    m_previewPanel->getRenderer()->setBackgroundColor(tgui::Color(20, 25, 40));
    m_previewPanel->getRenderer()->setBorderColor(tgui::Color(80, 100, 140));
    m_previewPanel->getRenderer()->setBorders(2);
    m_previewPanel->getRenderer()->setRoundedBorderRadius(10);
    m_mainPanel->add(m_previewPanel);

    gui.add(m_mainPanel);
}

void TGUIResourcePackGUI::setupStyling() {
    // This method ensures all widgets have a cohesive, modern look
}

void TGUIResourcePackGUI::loadResourcePacks() {
    m_availablePacks.clear();
    m_packList->removeAllWidgets();

    std::string packsDir = "assets/resource_packs";
    if (std::filesystem::exists(packsDir)) {
        float yPos = 10;
        for (const auto& entry : std::filesystem::directory_iterator(packsDir)) {
            if (entry.is_directory()) {
                std::string packName = entry.path().filename().string();
                m_availablePacks.push_back(packName);

                auto button = tgui::Button::create(packName);
                button->setSize("90%", "55");
                button->setPosition("5%", yPos);
                button->getRenderer()->setBackgroundColor(tgui::Color(60, 80, 120));
                button->getRenderer()->setBackgroundColorHover(tgui::Color(80, 100, 140));
                button->getRenderer()->setBackgroundColorDown(tgui::Color(40, 60, 100));
                button->getRenderer()->setTextColor(tgui::Color::White);
                button->getRenderer()->setRoundedBorderRadius(8);
                button->getRenderer()->setBorders(1);
                button->getRenderer()->setBorderColor(tgui::Color(100, 120, 160));
                
                button->onPress([this, packName] { onPackSelected(packName); });
                m_packList->add(button);
                
                yPos += 65;
            }
        }
    }
}

void TGUIResourcePackGUI::onPackSelected(const std::string& packName) {
    m_currentPack = packName;
    m_assetManager->loadResourcePack(packName);
    createPackPreview(packName);
}

void TGUIResourcePackGUI::createPackPreview(const std::string& packName) {
    m_previewPanel->removeAllWidgets();
    
    auto previewLabel = tgui::Label::create("🎭 " + packName);
    previewLabel->setPosition("5%", "5%");
    previewLabel->setTextSize(20);
    previewLabel->getRenderer()->setTextColor(tgui::Color(200, 220, 255));
    m_previewPanel->add(previewLabel);

    auto descLabel = tgui::Label::create("Theme Preview");
    descLabel->setPosition("5%", "15%");
    descLabel->setTextSize(14);
    descLabel->getRenderer()->setTextColor(tgui::Color(150, 170, 200));
    m_previewPanel->add(descLabel);

    auto theme = m_assetManager->getCurrentTheme();
    if (theme.colors.find("primary") != theme.colors.end()) {
        float yOffset = 25;
        
        // Primary color swatch
        auto primaryPanel = tgui::Panel::create();
        primaryPanel->setSize("80%", "25");
        primaryPanel->setPosition("10%", std::to_string(yOffset) + "%");
        
        auto& primaryColor = theme.colors.at("primary");
        primaryPanel->getRenderer()->setBackgroundColor(
            tgui::Color(primaryColor.r, primaryColor.g, primaryColor.b)
        );
        primaryPanel->getRenderer()->setRoundedBorderRadius(5);
        m_previewPanel->add(primaryPanel);
        
        auto primaryLabel = tgui::Label::create("Primary Color");
        primaryLabel->setPosition("10%", std::to_string(yOffset + 8) + "%");
        primaryLabel->setTextSize(12);
        primaryLabel->getRenderer()->setTextColor(tgui::Color::White);
        m_previewPanel->add(primaryLabel);
        
        yOffset += 15;
        
        // Accent color if available
        if (theme.colors.find("accent") != theme.colors.end()) {
            auto accentPanel = tgui::Panel::create();
            accentPanel->setSize("80%", "25");
            accentPanel->setPosition("10%", std::to_string(yOffset) + "%");
            
            auto& accentColor = theme.colors.at("accent");
            accentPanel->getRenderer()->setBackgroundColor(
                tgui::Color(accentColor.r, accentColor.g, accentColor.b)
            );
            accentPanel->getRenderer()->setRoundedBorderRadius(5);
            m_previewPanel->add(accentPanel);
            
            auto accentLabel = tgui::Label::create("Accent Color");
            accentLabel->setPosition("10%", std::to_string(yOffset + 8) + "%");
            accentLabel->setTextSize(12);
            accentLabel->getRenderer()->setTextColor(tgui::Color::White);
            m_previewPanel->add(accentLabel);
        }
    }
    
    auto applyButton = tgui::Button::create("Apply Theme");
    applyButton->setSize("80%", "35");
    applyButton->setPosition("10%", "80%");
    applyButton->getRenderer()->setBackgroundColor(tgui::Color(50, 150, 50));
    applyButton->getRenderer()->setBackgroundColorHover(tgui::Color(70, 170, 70));
    applyButton->getRenderer()->setTextColor(tgui::Color::White);
    applyButton->getRenderer()->setRoundedBorderRadius(8);
    applyButton->onPress([this, packName] { 
        m_assetManager->loadResourcePack(packName);
        hide(); // Close GUI after applying
    });
    m_previewPanel->add(applyButton);
}

void TGUIResourcePackGUI::show() {
    m_visible = true;
    if (m_mainPanel) {
        m_mainPanel->setVisible(true);
        loadResourcePacks(); // Refresh packs when showing
    }
}

void TGUIResourcePackGUI::hide() {
    m_visible = false;
    if (m_mainPanel) {
        m_mainPanel->setVisible(false);
    }
}

void TGUIResourcePackGUI::toggle() {
    if (m_visible) {
        hide();
    } else {
        show();
    }
}

void TGUIResourcePackGUI::update() {
    m_animationTime += 0.016f; // Assume 60 FPS
    
    // Subtle pulsing effect for the main panel
    if (m_visible && m_mainPanel) {
        float pulse = 0.95f + 0.05f * std::sin(m_animationTime * 2.0f);
        // Could add subtle scaling or opacity effects here
    }
}
