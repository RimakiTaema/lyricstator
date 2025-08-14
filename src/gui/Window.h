#pragma once
#include <string>

struct SDL_Window;
struct SDL_Renderer;

namespace Lyricstator {

class Window {
public:
    Window();
    ~Window();
    
    bool Initialize(int width, int height, const std::string& title);
    void Shutdown();
    
    void Clear();
    void Present();
    void Resize(int width, int height);
    
    SDL_Renderer* GetRenderer() const { return renderer_; }
    SDL_Window* GetSDLWindow() const { return window_; }
    
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    
private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    int width_;
    int height_;
    bool initialized_;
};

} // namespace Lyricstator
