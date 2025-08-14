#include "AssetManager.h"
#include "../utils/ErrorHandler.h"
#include <SDL_image.h>
#include <filesystem>
#include <fstream>
#include <json/json.h>

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

AssetManager::~AssetManager() {
    cleanup();
}

void AssetManager::initializeAssetPath() {
    // Try different possible asset locations
    std::vector<std::string> possiblePaths = {
        "./assets/",
        "../assets/",
        "../../assets/",
        "./bin/assets/"
    };
    
    for (const auto& path : possiblePaths) {
        if (std::filesystem::exists(path)) {
            assetBasePath = path;
            loadDefaultResourcePack();
            // Scan for available resource packs
            std::string packDir = assetBasePath + "resource_packs/";
            if (std::filesystem::exists(packDir)) {
                for (const auto& entry : std::filesystem::directory_iterator(packDir)) {
                    if (entry.is_directory()) {
                        std::string packPath = entry.path().string() + "/pack.json";
                        if (std::filesystem::exists(packPath)) {
                            loadResourcePack(packPath);
                        }
                    }
                }
            }
            return;
        }
    }
    
    // Default fallback
    assetBasePath = "./assets/";
    loadDefaultResourcePack();
    ErrorHandler::getInstance().logWarning("Asset path not found, using default: " + assetBasePath);
}

bool AssetManager::loadResourcePack(const std::string& packPath) {
    try {
        ResourcePack pack = parseResourcePackConfig(packPath);
        resourcePacks[pack.name] = pack;
        ErrorHandler::getInstance().logInfo("Loaded resource pack: " + pack.displayName);
        return true;
    } catch (const std::exception& e) {
        ErrorHandler::getInstance().logError("Failed to load resource pack: " + packPath + " - " + e.what());
        return false;
    }
}

void AssetManager::setActiveResourcePack(const std::string& packName) {
    if (resourcePacks.find(packName) != resourcePacks.end()) {
        activeResourcePack = packName;
        ErrorHandler::getInstance().logInfo("Switched to resource pack: " + packName);
    } else {
        ErrorHandler::getInstance().logWarning("Resource pack not found: " + packName);
    }
}

std::vector<ResourcePack> AssetManager::getAvailableResourcePacks() {
    std::vector<ResourcePack> packs;
    for (const auto& pair : resourcePacks) {
        packs.push_back(pair.second);
    }
    return packs;
}

const ResourcePack& AssetManager::getActiveResourcePack() const {
    auto it = resourcePacks.find(activeResourcePack);
    return (it != resourcePacks.end()) ? it->second : resourcePacks.at("default");
}

void AssetManager::loadDefaultResourcePack() {
    ResourcePack defaultPack;
    defaultPack.name = "default";
    defaultPack.displayName = "Default Theme";
    defaultPack.author = "Lyricstator";
    defaultPack.version = "1.0";
    defaultPack.description = "The default Lyricstator theme";
    defaultPack.path = assetBasePath;
    
    resourcePacks["default"] = defaultPack;
}

ResourcePack AssetManager::parseResourcePackConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + configPath);
    }
    
    Json::Value root;
    file >> root;
    
    ResourcePack pack;
    pack.name = root["name"].asString();
    pack.displayName = root["displayName"].asString();
    pack.author = root["author"].asString();
    pack.version = root["version"].asString();
    pack.description = root["description"].asString();
    pack.path = std::filesystem::path(configPath).parent_path().string() + "/";
    
    // Parse colors
    if (root.isMember("colors")) {
        const Json::Value& colors = root["colors"];
        if (colors.isMember("background")) {
            const Json::Value& bg = colors["background"];
            pack.colors.background = {(Uint8)bg[0].asInt(), (Uint8)bg[1].asInt(), (Uint8)bg[2].asInt(), (Uint8)bg[3].asInt()};
        }
        if (colors.isMember("text")) {
            const Json::Value& text = colors["text"];
            pack.colors.text = {(Uint8)text[0].asInt(), (Uint8)text[1].asInt(), (Uint8)text[2].asInt(), (Uint8)text[3].asInt()};
        }
        if (colors.isMember("accent")) {
            const Json::Value& accent = colors["accent"];
            pack.colors.accent = {(Uint8)accent[0].asInt(), (Uint8)accent[1].asInt(), (Uint8)accent[2].asInt(), (Uint8)accent[3].asInt()};
        }
        if (colors.isMember("highlight")) {
            const Json::Value& highlight = colors["highlight"];
            pack.colors.highlight = {(Uint8)highlight[0].asInt(), (Uint8)highlight[1].asInt(), (Uint8)highlight[2].asInt(), (Uint8)highlight[3].asInt()};
        }
        if (colors.isMember("progress")) {
            const Json::Value& progress = colors["progress"];
            pack.colors.progress = {(Uint8)progress[0].asInt(), (Uint8)progress[1].asInt(), (Uint8)progress[2].asInt(), (Uint8)progress[3].asInt()};
        }
    }
    
    // Parse fonts
    if (root.isMember("fonts")) {
        const Json::Value& fonts = root["fonts"];
        pack.fonts.lyrics = fonts.get("lyrics", "lyrics.ttf").asString();
        pack.fonts.ui = fonts.get("ui", "default.ttf").asString();
        pack.fonts.bold = fonts.get("bold", "bold.ttf").asString();
        pack.fonts.lyricsSize = fonts.get("lyricsSize", 32).asInt();
        pack.fonts.uiSize = fonts.get("uiSize", 16).asInt();
        pack.fonts.boldSize = fonts.get("boldSize", 20).asInt();
    }
    
    // Parse images
    if (root.isMember("images")) {
        const Json::Value& images = root["images"];
        pack.images.background = images.get("background", "background.png").asString();
        pack.images.noteIndicator = images.get("noteIndicator", "note_indicator.png").asString();
        pack.images.progressBar = images.get("progressBar", "progress_bar.png").asString();
    }
    
    return pack;
}

TTF_Font* AssetManager::getThemeFont(const std::string& fontType) {
    const ResourcePack& pack = getActiveResourcePack();
    std::string fontPath;
    int fontSize;
    
    if (fontType == "lyrics") {
        fontPath = pack.fonts.lyrics;
        fontSize = pack.fonts.lyricsSize;
    } else if (fontType == "ui") {
        fontPath = pack.fonts.ui;
        fontSize = pack.fonts.uiSize;
    } else if (fontType == "bold") {
        fontPath = pack.fonts.bold;
        fontSize = pack.fonts.boldSize;
    } else {
        return nullptr;
    }
    
    std::string fontKey = pack.name + "_" + fontType;
    TTF_Font* font = getFont(fontKey);
    if (!font) {
        std::string fullPath = pack.path + fontPath;
        font = loadFont(fontKey, fullPath, fontSize);
    }
    
    return font;
}

SDL_Texture* AssetManager::getThemeTexture(const std::string& textureType, SDL_Renderer* renderer) {
    const ResourcePack& pack = getActiveResourcePack();
    std::string texturePath;
    
    if (textureType == "background") {
        texturePath = pack.images.background;
    } else if (textureType == "noteIndicator") {
        texturePath = pack.images.noteIndicator;
    } else if (textureType == "progressBar") {
        texturePath = pack.images.progressBar;
    } else {
        return nullptr;
    }
    
    std::string textureKey = pack.name + "_" + textureType;
    SDL_Texture* texture = getTexture(textureKey);
    if (!texture) {
        std::string fullPath = pack.path + texturePath;
        texture = loadTexture(textureKey, fullPath, renderer);
    }
    
    return texture;
}

SDL_Color AssetManager::getThemeColor(const std::string& colorType) {
    const ResourcePack& pack = getActiveResourcePack();
    
    if (colorType == "background") return pack.colors.background;
    if (colorType == "text") return pack.colors.text;
    if (colorType == "accent") return pack.colors.accent;
    if (colorType == "highlight") return pack.colors.highlight;
    if (colorType == "progress") return pack.colors.progress;
    
    return {255, 255, 255, 255}; // Default white
}

std::string AssetManager::getAssetPath(const std::string& relativePath) {
    if (assetBasePath.empty()) {
        initializeAssetPath();
    }
    return assetBasePath + relativePath;
}

std::string AssetManager::getResourcePackPath(const std::string& packName, const std::string& relativePath) {
    auto it = resourcePacks.find(packName);
    if (it != resourcePacks.end()) {
        return it->second.path + relativePath;
    }
    return getAssetPath(relativePath);
}

TTF_Font* AssetManager::loadFont(const std::string& name, const std::string& path, int size) {
    std::string fullPath = getAssetPath(path);
    TTF_Font* font = TTF_OpenFont(fullPath.c_str(), size);
    
    if (!font) {
        ErrorHandler::getInstance().logError("Failed to load font: " + fullPath + " - " + TTF_GetError());
        return nullptr;
    }
    
    fonts[name] = font;
    return font;
}

TTF_Font* AssetManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    return (it != fonts.end()) ? it->second : nullptr;
}

SDL_Texture* AssetManager::loadTexture(const std::string& name, const std::string& path, SDL_Renderer* renderer) {
    std::string fullPath = getAssetPath(path);
    SDL_Surface* surface = IMG_Load(fullPath.c_str());
    
    if (!surface) {
        ErrorHandler::getInstance().logError("Failed to load image: " + fullPath + " - " + IMG_GetError());
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        ErrorHandler::getInstance().logError("Failed to create texture from: " + fullPath + " - " + SDL_GetError());
        return nullptr;
    }
    
    textures[name] = texture;
    return texture;
}

SDL_Texture* AssetManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    return (it != textures.end()) ? it->second : nullptr;
}

void AssetManager::cleanup() {
    for (auto& pair : fonts) {
        if (pair.second) {
            TTF_CloseFont(pair.second);
        }
    }
    fonts.clear();
    
    for (auto& pair : textures) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();
}
