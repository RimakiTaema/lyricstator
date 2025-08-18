#ifdef ANDROID_BUILD
#include "AssetManager.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <SDL.h>

// Android-specific asset loading
std::string AssetManager::getAssetPath(const std::string& filename) {
    // On Android, assets are accessed differently
    return "assets/" + filename;
}

SDL_RWops* AssetManager::loadAssetRW(const std::string& filename) {
    // Load from Android APK assets
    return SDL_RWFromFile(("assets/" + filename).c_str(), "rb");
}

bool AssetManager::assetExists(const std::string& filename) {
    SDL_RWops* rw = SDL_RWFromFile(("assets/" + filename).c_str(), "rb");
    if (rw) {
        SDL_RWclose(rw);
        return true;
    }
    return false;
}
#endif
