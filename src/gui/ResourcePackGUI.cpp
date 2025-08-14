#include "gui/ResourcePackGUI.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Lyricstator {

ResourcePackGUI::ResourcePackGUI()
    : renderer_(nullptr)
    , assetManager_(nullptr)
    , font_(nullptr)
    , titleFont_(nullptr)
    , visible_(false)
    , initialized_(false)
    , animationTime_(0.0f)
    , slideOffset_(0.0f)
    , selectedPackIndex_(-1)
    , hoveredPackIndex_(-1)
    , scrollOffset_(0)
    , maxScroll_(0)
    , scrolling_(false)
{
    backgroundColor_ = {0, 0, 0, 180};
    panelColor_ = {25, 25, 35, 240};
    textColor_ = {255, 255, 255, 255};
    highlightColor_ = {100, 200, 255, 255};
    buttonColor_ = {60, 60, 80, 255};
    buttonHoverColor_ = {80, 80, 120, 255};
}

ResourcePackGUI::~ResourcePackGUI() {
    Shutdown();
}

bool ResourcePackGUI::Initialize(SDL_Renderer* renderer, AssetManager* assetManager) {
    renderer_ = renderer;
    assetManager_ = assetManager;
    
    // Load fonts
    font_ = TTF_OpenFont("assets/fonts/default.ttf", 16);
    titleFont_ = TTF_OpenFont("assets/fonts/bold.ttf", 24);
    
    if (!font_ || !titleFont_) {
        std::cout << "Warning: Could not load GUI fonts, using system defaults" << std::endl;
    }
    
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer_, &windowWidth, &windowHeight);
    
    guiRect_ = {
        windowWidth / 6,
        windowHeight / 8,
        windowWidth * 2 / 3,
        windowHeight * 3 / 4
    };
    
    listRect_ = {
        guiRect_.x + 20,
        guiRect_.y + 60,
        guiRect_.w / 2 - 30,
        guiRect_.h - 120
    };
    
    previewRect_ = {
        guiRect_.x + guiRect_.w / 2 + 10,
        guiRect_.y + 60,
        guiRect_.w / 2 - 30,
        guiRect_.h - 120
    };
    
    RefreshResourcePacks();
    initialized_ = true;
    
    std::cout << "ResourcePackGUI initialized" << std::endl;
    return true;
}

void ResourcePackGUI::Shutdown() {
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (titleFont_) {
        TTF_CloseFont(titleFont_);
        titleFont_ = nullptr;
    }
    initialized_ = false;
}

void ResourcePackGUI::Update(float deltaTime) {
    if (!visible_) return;
    
    animationTime_ += deltaTime;
    
    float targetOffset = visible_ ? 0.0f : -guiRect_.w;
    slideOffset_ += (targetOffset - slideOffset_) * deltaTime * 8.0f;
    
    UpdateScrolling();
}

void ResourcePackGUI::Render() {
    if (!visible_ || !initialized_) return;
    
    SDL_Rect animatedRect = guiRect_;
    animatedRect.x += static_cast<int>(slideOffset_);
    
    RenderBackground();
    RenderTitle();
    RenderResourcePackList();
    RenderPackPreview();
    RenderButtons();
    RenderScrollbar();
}

void ResourcePackGUI::RenderBackground() {
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer_, &windowWidth, &windowHeight);
    
    SDL_SetRenderDrawColor(renderer_, backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, backgroundColor_.a);
    SDL_Rect fullScreen = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer_, &fullScreen);
    
    // Main panel with animated position
    SDL_Rect animatedRect = guiRect_;
    animatedRect.x += static_cast<int>(slideOffset_);
    
    DrawRoundedRect(animatedRect, 10, panelColor_);
    
    // Border
    SDL_SetRenderDrawColor(renderer_, highlightColor_.r, highlightColor_.g, highlightColor_.b, 255);
    SDL_RenderDrawRect(renderer_, &animatedRect);
}

void ResourcePackGUI::RenderTitle() {
    SDL_Rect animatedRect = guiRect_;
    animatedRect.x += static_cast<int>(slideOffset_);
    
    RenderText("Resource Pack Manager", animatedRect.x + animatedRect.w / 2, animatedRect.y + 30, highlightColor_, 24);
    
    // Close button (X)
    SDL_Rect closeButton = {animatedRect.x + animatedRect.w - 40, animatedRect.y + 10, 30, 30};
    DrawButton(closeButton, "X", false);
}

void ResourcePackGUI::RenderResourcePackList() {
    SDL_Rect animatedListRect = listRect_;
    animatedListRect.x += static_cast<int>(slideOffset_);
    
    // List background
    DrawRoundedRect(animatedListRect, 5, {15, 15, 25, 255});
    
    int itemHeight = 60;
    int visibleItems = animatedListRect.h / itemHeight;
    int startIndex = scrollOffset_ / itemHeight;
    
    for (int i = 0; i < visibleItems && (startIndex + i) < resourcePacks_.size(); ++i) {
        int packIndex = startIndex + i;
        const auto& pack = resourcePacks_[packIndex];
        
        SDL_Rect itemRect = {
            animatedListRect.x + 5,
            animatedListRect.y + i * itemHeight - (scrollOffset_ % itemHeight),
            animatedListRect.w - 10,
            itemHeight - 2
        };
        
        // Highlight selection/hover
        Color itemColor = {20, 20, 30, 255};
        if (packIndex == selectedPackIndex_) {
            itemColor = {40, 80, 120, 255};
        } else if (packIndex == hoveredPackIndex_) {
            itemColor = {30, 30, 45, 255};
        }
        
        if (pack.isActive) {
            itemColor = {60, 120, 60, 255}; // Green for active pack
        }
        
        DrawRoundedRect(itemRect, 3, itemColor);
        
        // Pack name and info
        RenderText(pack.displayName, itemRect.x + 10, itemRect.y + 15, textColor_, 16);
        RenderText("by " + pack.author, itemRect.x + 10, itemRect.y + 35, {180, 180, 180, 255}, 12);
        
        // Active indicator
        if (pack.isActive) {
            RenderText("ACTIVE", itemRect.x + itemRect.w - 60, itemRect.y + 20, {100, 255, 100, 255}, 12);
        }
    }
}

void ResourcePackGUI::RenderPackPreview() {
    if (selectedPackIndex_ < 0 || selectedPackIndex_ >= resourcePacks_.size()) return;
    
    SDL_Rect animatedPreviewRect = previewRect_;
    animatedPreviewRect.x += static_cast<int>(slideOffset_);
    
    const auto& pack = resourcePacks_[selectedPackIndex_];
    
    // Preview background
    DrawRoundedRect(animatedPreviewRect, 5, {15, 15, 25, 255});
    
    int yOffset = animatedPreviewRect.y + 20;
    RenderText("Preview: " + pack.displayName, animatedPreviewRect.x + 10, yOffset, highlightColor_, 18);
    yOffset += 40;
    
    RenderText("Description:", animatedPreviewRect.x + 10, yOffset, textColor_, 14);
    yOffset += 25;
    RenderText(pack.description, animatedPreviewRect.x + 10, yOffset, {200, 200, 200, 255}, 12);
    yOffset += 40;
    
    RenderText("Version: " + pack.version, animatedPreviewRect.x + 10, yOffset, {180, 180, 180, 255}, 12);
    yOffset += 25;
    RenderText("Author: " + pack.author, animatedPreviewRect.x + 10, yOffset, {180, 180, 180, 255}, 12);
    
    // Apply button
    SDL_Rect applyButton = {
        animatedPreviewRect.x + 10,
        animatedPreviewRect.y + animatedPreviewRect.h - 50,
        100, 35
    };
    
    std::string buttonText = pack.isActive ? "Active" : "Apply";
    DrawButton(applyButton, buttonText, !pack.isActive);
}

void ResourcePackGUI::RenderButtons() {
    SDL_Rect animatedRect = guiRect_;
    animatedRect.x += static_cast<int>(slideOffset_);
    
    // Refresh button
    SDL_Rect refreshButton = {
        animatedRect.x + 20,
        animatedRect.y + animatedRect.h - 50,
        80, 35
    };
    DrawButton(refreshButton, "Refresh", true);
}

void ResourcePackGUI::RenderScrollbar() {
    if (maxScroll_ <= 0) return;
    
    SDL_Rect animatedListRect = listRect_;
    animatedListRect.x += static_cast<int>(slideOffset_);
    
    // Scrollbar track
    SDL_Rect scrollTrack = {
        animatedListRect.x + animatedListRect.w - 10,
        animatedListRect.y,
        8,
        animatedListRect.h
    };
    
    SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer_, &scrollTrack);
    
    // Scrollbar thumb
    float scrollRatio = static_cast<float>(scrollOffset_) / maxScroll_;
    int thumbHeight = std::max(20, animatedListRect.h / 4);
    int thumbY = animatedListRect.y + static_cast<int>(scrollRatio * (animatedListRect.h - thumbHeight));
    
    SDL_Rect scrollThumb = {
        scrollTrack.x + 1,
        thumbY,
        6,
        thumbHeight
    };
    
    SDL_SetRenderDrawColor(renderer_, highlightColor_.r, highlightColor_.g, highlightColor_.b, 255);
    SDL_RenderFillRect(renderer_, &scrollThumb);
}

void ResourcePackGUI::HandleEvent(const SDL_Event& event) {
    if (!visible_) return;
    
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                HandleMouseClick(event.button.x, event.button.y);
            }
            break;
        case SDL_MOUSEWHEEL:
            HandleMouseWheel(event.wheel.y);
            break;
        case SDL_MOUSEMOTION:
            // Update hover state
            SDL_Rect animatedListRect = listRect_;
            animatedListRect.x += static_cast<int>(slideOffset_);
            
            if (event.motion.x >= animatedListRect.x && event.motion.x <= animatedListRect.x + animatedListRect.w &&
                event.motion.y >= animatedListRect.y && event.motion.y <= animatedListRect.y + animatedListRect.h) {
                
                int itemHeight = 60;
                int relativeY = event.motion.y - animatedListRect.y + scrollOffset_;
                int itemIndex = relativeY / itemHeight;
                
                if (itemIndex >= 0 && itemIndex < resourcePacks_.size()) {
                    hoveredPackIndex_ = itemIndex;
                } else {
                    hoveredPackIndex_ = -1;
                }
            } else {
                hoveredPackIndex_ = -1;
            }
            break;
    }
}

void ResourcePackGUI::Show() {
    visible_ = true;
    RefreshResourcePacks();
}

void ResourcePackGUI::Hide() {
    visible_ = false;
}

void ResourcePackGUI::Toggle() {
    if (visible_) {
        Hide();
    } else {
        Show();
    }
}

void ResourcePackGUI::RefreshResourcePacks() {
    if (!assetManager_) return;
    
    resourcePacks_.clear();
    
    auto availablePacks = assetManager_->GetAvailableResourcePacks();
    std::string currentPack = assetManager_->GetCurrentResourcePack();
    
    for (const auto& packName : availablePacks) {
        ResourcePackInfo info;
        info.name = packName;
        info.displayName = packName;
        info.description = "Custom resource pack with unique visual style";
        info.author = "Unknown";
        info.version = "1.0";
        info.isActive = (packName == currentPack);
        
        // Try to load pack metadata if available
        // This would read from pack.json files in the future
        
        resourcePacks_.push_back(info);
    }
    
    // Update scroll limits
    int itemHeight = 60;
    int totalHeight = resourcePacks_.size() * itemHeight;
    maxScroll_ = std::max(0, totalHeight - listRect_.h);
    scrollOffset_ = std::min(scrollOffset_, maxScroll_);
    
    std::cout << "Refreshed resource packs: " << resourcePacks_.size() << " found" << std::endl;
}

void ResourcePackGUI::SelectResourcePack(const std::string& packName) {
    if (!assetManager_) return;
    
    if (assetManager_->LoadResourcePack(packName)) {
        RefreshResourcePacks(); // Refresh to update active status
        std::cout << "Applied resource pack: " << packName << std::endl;
    }
}

void ResourcePackGUI::UpdateScrolling() {
    // Smooth scrolling animation could be added here
}

void ResourcePackGUI::HandleMouseClick(int x, int y) {
    SDL_Rect animatedRect = guiRect_;
    animatedRect.x += static_cast<int>(slideOffset_);
    
    // Check close button
    SDL_Rect closeButton = {animatedRect.x + animatedRect.w - 40, animatedRect.y + 10, 30, 30};
    if (x >= closeButton.x && x <= closeButton.x + closeButton.w &&
        y >= closeButton.y && y <= closeButton.y + closeButton.h) {
        Hide();
        return;
    }
    
    // Check resource pack list
    SDL_Rect animatedListRect = listRect_;
    animatedListRect.x += static_cast<int>(slideOffset_);
    
    if (x >= animatedListRect.x && x <= animatedListRect.x + animatedListRect.w &&
        y >= animatedListRect.y && y <= animatedListRect.y + animatedListRect.h) {
        
        int itemHeight = 60;
        int relativeY = y - animatedListRect.y + scrollOffset_;
        int itemIndex = relativeY / itemHeight;
        
        if (itemIndex >= 0 && itemIndex < resourcePacks_.size()) {
            selectedPackIndex_ = itemIndex;
        }
        return;
    }
    
    // Check apply button
    if (selectedPackIndex_ >= 0 && selectedPackIndex_ < resourcePacks_.size()) {
        SDL_Rect animatedPreviewRect = previewRect_;
        animatedPreviewRect.x += static_cast<int>(slideOffset_);
        
        SDL_Rect applyButton = {
            animatedPreviewRect.x + 10,
            animatedPreviewRect.y + animatedPreviewRect.h - 50,
            100, 35
        };
        
        if (x >= applyButton.x && x <= applyButton.x + applyButton.w &&
            y >= applyButton.y && y <= applyButton.y + applyButton.h) {
            
            const auto& pack = resourcePacks_[selectedPackIndex_];
            if (!pack.isActive) {
                SelectResourcePack(pack.name);
            }
        }
    }
}

void ResourcePackGUI::HandleMouseWheel(int direction) {
    int scrollSpeed = 30;
    scrollOffset_ -= direction * scrollSpeed;
    scrollOffset_ = std::max(0, std::min(scrollOffset_, maxScroll_));
}

void ResourcePackGUI::DrawRoundedRect(const SDL_Rect& rect, int radius, const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer_, &rect);
    
    // Add corner rounding effect with smaller rectangles (simplified)
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    for (int i = 0; i < radius; ++i) {
        SDL_RenderDrawLine(renderer_, rect.x + i, rect.y + radius - i, rect.x + i, rect.y + rect.h - radius + i);
        SDL_RenderDrawLine(renderer_, rect.x + rect.w - i - 1, rect.y + radius - i, rect.x + rect.w - i - 1, rect.y + rect.h - radius + i);
    }
}

void ResourcePackGUI::DrawButton(const SDL_Rect& rect, const std::string& text, bool highlighted) {
    Color buttonColor = highlighted ? buttonHoverColor_ : buttonColor_;
    DrawRoundedRect(rect, 3, buttonColor);
    
    // Button border
    SDL_SetRenderDrawColor(renderer_, highlightColor_.r, highlightColor_.g, highlightColor_.b, 100);
    SDL_RenderDrawRect(renderer_, &rect);
    
    // Button text
    RenderText(text, rect.x + rect.w / 2, rect.y + rect.h / 2, textColor_, 14);
}

void ResourcePackGUI::RenderText(const std::string& text, int x, int y, const Color& color, int fontSize) {
    if (!font_ || text.empty()) return;
    
    TTF_Font* useFont = (fontSize > 18 && titleFont_) ? titleFont_ : font_;
    
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    SDL_Surface* textSurface = TTF_RenderText_Blended(useFont, text.c_str(), sdlColor);
    
    if (!textSurface) return;
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer_, textSurface);
    
    if (textTexture) {
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        
        SDL_Rect destRect = {x - textWidth / 2, y - textHeight / 2, textWidth, textHeight};
        SDL_RenderCopy(renderer_, textTexture, nullptr, &destRect);
        
        SDL_DestroyTexture(textTexture);
    }
    
    SDL_FreeSurface(textSurface);
}

} // namespace Lyricstator
