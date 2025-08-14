#include "gui/Window.h"
#include <SDL2/SDL.h>
#include <iostream>

namespace Lyricstator {

Window::Window() : window_(nullptr), renderer_(nullptr), width_(0), height_(0), initialized_(false) {
}

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(int width, int height, const std::string& title) {
    width_ = width;
    height_ = height;
    
    // Create window
    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );
    
    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }
    
    // Enable drag and drop
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    
    initialized_ = true;
    std::cout << "Window initialized: " << width << "x" << height << std::endl;
    return true;
}

void Window::Shutdown() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    
    initialized_ = false;
}

void Window::Clear() {
    if (renderer_) {
        SDL_SetRenderDrawColor(renderer_, 20, 20, 30, 255); // Dark blue background
        SDL_RenderClear(renderer_);
    }
}

void Window::Present() {
    if (renderer_) {
        SDL_RenderPresent(renderer_);
    }
}

void Window::Resize(int width, int height) {
    width_ = width;
    height_ = height;
    if (window_) {
        SDL_SetWindowSize(window_, width, height);
    }
}

} // namespace Lyricstator
