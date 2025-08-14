#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>

struct ResourcePack {
    std::string name;
    std::string path;
    std::string displayName;
    std::string author;
    std::string version;
    std::string description;
    
    // Theme configuration
    struct Colors {
        SDL_Color background = {20, 20, 30, 255};
        SDL_Color text = {255, 255, 255, 255};
        SDL_Color accent = {100, 200, 255, 255};
        SDL_Color highlight = {255, 100, 100, 255};
        SDL_Color progress = {50, 255, 50, 255};
    } colors;
    
    struct Fonts {
        std::string lyrics = "lyrics.ttf";
        std::string ui = "default.ttf";
        std::string bold = "bold.ttf";
        int lyricsSize = 32;
        int uiSize = 16;
        int boldSize = 20;
    } fonts;
    
    struct Images {
        std::string background = "background.png";
        std::string noteIndicator = "note_indicator.png";
        std::string progressBar = "progress_bar.png";
    } images;
};

class AssetManager {
public:
    static AssetManager& getInstance();
    
    // Resource pack management
    bool loadResourcePack(const std::string& packPath);
    void setActiveResourcePack(const std::string& packName);
    std::vector<ResourcePack> getAvailableResourcePacks();
    const ResourcePack& getActiveResourcePack() const;
    
    // Font management
    TTF_Font* loadFont(const std::string& name, const std::string& path, int size);
    TTF_Font* getFont(const std::string& name);
    TTF_Font* getThemeFont(const std::string& fontType); // lyrics, ui, bold
    
    // Texture management
    SDL_Texture* loadTexture(const std::string& name, const std::string& path, SDL_Renderer* renderer);
    SDL_Texture* getTexture(const std::string& name);
    SDL_Texture* getThemeTexture(const std::string& textureType, SDL_Renderer* renderer);
    
    // Color management
    SDL_Color getThemeColor(const std::string& colorType);
    
    // Asset path helpers
    std::string getAssetPath(const std::string& relativePath);
    std::string getResourcePackPath(const std::string& packName, const std::string& relativePath);
    
    void cleanup();

private:
    AssetManager() = default;
    ~AssetManager();
    
    std::unordered_map<std::string, TTF_Font*> fonts;
    std::unordered_map<std::string, SDL_Texture*> textures;
    std::unordered_map<std::string, ResourcePack> resourcePacks;
    
    std::string assetBasePath;
    std::string activeResourcePack = "default";
    
    void initializeAssetPath();
    void loadDefaultResourcePack();
    ResourcePack parseResourcePackConfig(const std::string& configPath);
};
