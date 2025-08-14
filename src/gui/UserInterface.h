#pragma once
#include <string>

struct SDL_Renderer;
union SDL_Event;

namespace Lyricstator {

class UserInterface {
public:
    UserInterface();
    ~UserInterface();
    
    bool Initialize(SDL_Renderer* renderer);
    void Shutdown();
    
    void Update(float deltaTime);
    void Render();
    void HandleEvent(const SDL_Event& event);
    
private:
    SDL_Renderer* renderer_;
    bool initialized_;
};

} // namespace Lyricstator
