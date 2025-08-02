#include "gui/UserInterface.h"
#include <SDL2/SDL.h>
#include <iostream>

namespace Lyricstator {

UserInterface::UserInterface() : renderer_(nullptr), initialized_(false) {
}

UserInterface::~UserInterface() {
    Shutdown();
}

bool UserInterface::Initialize(SDL_Renderer* renderer) {
    renderer_ = renderer;
    initialized_ = true;
    std::cout << "UserInterface initialized" << std::endl;
    return true;
}

void UserInterface::Shutdown() {
    initialized_ = false;
}

void UserInterface::Update(float deltaTime) {
    // Update UI animations
}

void UserInterface::Render() {
    if (!renderer_) return;
    
    // Render UI elements
    // Draw control panel
    SDL_SetRenderDrawColor(renderer_, 40, 40, 60, 255);
    SDL_Rect controlPanel = {10, 10, 200, 100};
    SDL_RenderFillRect(renderer_, &controlPanel);
    
    // Draw play button placeholder
    SDL_SetRenderDrawColor(renderer_, 100, 200, 100, 255);
    SDL_Rect playButton = {20, 20, 80, 30};
    SDL_RenderFillRect(renderer_, &playButton);
    
    // Note: Real UI would use SDL_ttf for text and buttons
}

void UserInterface::HandleEvent(const SDL_Event& event) {
    // Handle UI events
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        // Check if play button was clicked
        if (event.button.x >= 20 && event.button.x <= 100 &&
            event.button.y >= 20 && event.button.y <= 50) {
            std::cout << "Play button clicked" << std::endl;
        }
    }
}

} // namespace Lyricstator