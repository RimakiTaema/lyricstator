#pragma once

#include "common/Types.h"
#include "core/AssetManager.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>

namespace Lyricstator {

class ResourcePackGUI {
public:
    ResourcePackGUI();
    ~ResourcePackGUI();
    
    bool Initialize(SDL_Renderer* renderer, AssetManager* assetManager);
    void Shutdown();
    
    void Update(float deltaTime);
    void Render();
    void HandleEvent(const SDL_Event& event);
    
    // Toggle visibility
    void Show();
    void Hide();
    void Toggle();
    bool IsVisible() const { return visible_; }
    
    // Resource pack management
    void RefreshResourcePacks();
    void SelectResourcePack(const std::string& packName);
    
private:
    struct ResourcePackInfo {
        std::string name;
        std::string displayName;
        std::string description;
        std::string author;
        std::string version;
        bool isActive;
    };
    
    void RenderBackground();
    void RenderTitle();
    void RenderResourcePackList();
    void RenderPackPreview();
    void RenderButtons();
    void RenderScrollbar();
    
    void UpdateScrolling();
    void HandleMouseClick(int x, int y);
    void HandleMouseWheel(int direction);
    
    void DrawRoundedRect(const SDL_Rect& rect, int radius, const Color& color);
    void DrawButton(const SDL_Rect& rect, const std::string& text, bool highlighted);
    void RenderText(const std::string& text, int x, int y, const Color& color, int fontSize = 16);
    
    SDL_Renderer* renderer_;
    AssetManager* assetManager_;
    TTF_Font* font_;
    TTF_Font* titleFont_;
    
    bool visible_;
    bool initialized_;
    
    // Animation
    float animationTime_;
    float slideOffset_;
    
    // Resource pack data
    std::vector<ResourcePackInfo> resourcePacks_;
    int selectedPackIndex_;
    int hoveredPackIndex_;
    
    // Scrolling
    int scrollOffset_;
    int maxScroll_;
    bool scrolling_;
    
    // Layout
    SDL_Rect guiRect_;
    SDL_Rect listRect_;
    SDL_Rect previewRect_;
    
    // Colors
    Color backgroundColor_;
    Color panelColor_;
    Color textColor_;
    Color highlightColor_;
    Color buttonColor_;
    Color buttonHoverColor_;
};

} // namespace Lyricstator
