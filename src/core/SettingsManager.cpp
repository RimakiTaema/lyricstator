#include "SettingsManager.h"
#include "utils/ErrorHandler.h"
#include <fstream>
#include <json/json.h>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace Lyricstator {

SettingsManager& SettingsManager::getInstance() {
    static SettingsManager instance;
    return instance;
}

bool SettingsManager::loadSettings(const std::string& filepath) {
    settingsFilePath_ = filepath.empty() ? "settings.json" : filepath;
    
    if (!std::filesystem::exists(settingsFilePath_)) {
        initializeDefaults();
        saveSettings(); // Create default settings file
        return true;
    }
    
    try {
        std::ifstream file(settingsFilePath_);
        if (!file.is_open()) {
            ErrorHandler::getInstance().logError("Cannot open settings file: " + settingsFilePath_);
            initializeDefaults();
            return false;
        }
        
        Json::Value root;
        file >> root;
        
        // Load audio settings
        if (root.isMember("audio")) {
            const Json::Value& audio = root["audio"];
            audioSettings_.masterVolume = audio.get("masterVolume", 1.0f).asFloat();
            audioSettings_.sampleRate = audio.get("sampleRate", 44100).asInt();
            audioSettings_.bufferSize = audio.get("bufferSize", 1024).asInt();
            audioSettings_.enableEqualizer = audio.get("enableEqualizer", true).asBool();
            audioSettings_.equalizerBandCount = audio.get("equalizerBandCount", 12).asInt();
            
            // Load equalizer bands
            if (audio.isMember("equalizerBands")) {
                const Json::Value& bands = audio["equalizerBands"];
                audioSettings_.equalizerBands.clear();
                for (const auto& band : bands) {
                    EqualizerBand eqBand;
                    eqBand.frequency = band.get("frequency", 1000.0f).asFloat();
                    eqBand.gain = band.get("gain", 0.0f).asFloat();
                    eqBand.enabled = band.get("enabled", true).asBool();
                    audioSettings_.equalizerBands.push_back(eqBand);
                }
            }
        }
        
        // Load directory settings
        if (root.isMember("directories")) {
            const Json::Value& dirs = root["directories"];
            directorySettings_.defaultExportPath = dirs.get("defaultExportPath", "./exports/").asString();
            directorySettings_.resourcePackPath = dirs.get("resourcePackPath", "./assets/resource_packs/").asString();
            directorySettings_.recursiveSearch = dirs.get("recursiveSearch", true).asBool();
            
            // Load song directories
            if (dirs.isMember("songDirectories")) {
                directorySettings_.songDirectories.clear();
                for (const auto& dir : dirs["songDirectories"]) {
                    directorySettings_.songDirectories.push_back(dir.asString());
                }
            }
            
            // Load supported formats
            if (dirs.isMember("supportedFormats")) {
                directorySettings_.supportedFormats.clear();
                for (const auto& format : dirs["supportedFormats"]) {
                    directorySettings_.supportedFormats.push_back(format.asString());
                }
            }
        }
        
        // Load UI settings
        if (root.isMember("ui")) {
            const Json::Value& ui = root["ui"];
            uiSettings_.theme = ui.get("theme", "default").asString();
            uiSettings_.fontSize = ui.get("fontSize", 16).asInt();
            uiSettings_.showSpectrum = ui.get("showSpectrum", true).asBool();
            uiSettings_.showPitchDetection = ui.get("showPitchDetection", true).asBool();
            uiSettings_.fullscreen = ui.get("fullscreen", false).asBool();
            uiSettings_.windowWidth = ui.get("windowWidth", 1280).asInt();
            uiSettings_.windowHeight = ui.get("windowHeight", 720).asInt();
        }
        
        // Load karaoke settings
        if (root.isMember("karaoke")) {
            const Json::Value& karaoke = root["karaoke"];
            karaokeSettings_.enablePitchDetection = karaoke.get("enablePitchDetection", true).asBool();
            karaokeSettings_.pitchSensitivity = karaoke.get("pitchSensitivity", 0.7f).asFloat();
            karaokeSettings_.showNoteIndicators = karaoke.get("showNoteIndicators", true).asBool();
            karaokeSettings_.enableScoring = karaoke.get("enableScoring", true).asBool();
            karaokeSettings_.lyricFadeTime = karaoke.get("lyricFadeTime", 0.5f).asFloat();
        }
        
        // Load keybindings
        if (root.isMember("keybindings")) {
            keyBindings_.clear();
            for (const auto& binding : root["keybindings"]) {
                KeyBinding kb;
                kb.key = static_cast<SDL_Scancode>(binding.get("key", SDL_SCANCODE_UNKNOWN).asInt());
                kb.ctrl = binding.get("ctrl", false).asBool();
                kb.shift = binding.get("shift", false).asBool();
                kb.alt = binding.get("alt", false).asBool();
                kb.action = binding.get("action", "").asString();
                kb.description = binding.get("description", "").asString();
                keyBindings_.push_back(kb);
            }
        }
        
        ErrorHandler::getInstance().logInfo("Settings loaded successfully from: " + settingsFilePath_);
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::getInstance().logError("Failed to load settings: " + std::string(e.what()));
        initializeDefaults();
        return false;
    }
}

bool SettingsManager::saveSettings(const std::string& filepath) const {
    std::string path = filepath.empty() ? settingsFilePath_ : filepath;
    
    try {
        Json::Value root;
        
        // Save audio settings
        Json::Value audio;
        audio["masterVolume"] = audioSettings_.masterVolume;
        audio["sampleRate"] = audioSettings_.sampleRate;
        audio["bufferSize"] = audioSettings_.bufferSize;
        audio["enableEqualizer"] = audioSettings_.enableEqualizer;
        audio["equalizerBandCount"] = audioSettings_.equalizerBandCount;
        
        // Save equalizer bands
        Json::Value bands(Json::arrayValue);
        for (const auto& band : audioSettings_.equalizerBands) {
            Json::Value bandJson;
            bandJson["frequency"] = band.frequency;
            bandJson["gain"] = band.gain;
            bandJson["enabled"] = band.enabled;
            bands.append(bandJson);
        }
        audio["equalizerBands"] = bands;
        root["audio"] = audio;
        
        // Save directory settings
        Json::Value dirs;
        dirs["defaultExportPath"] = directorySettings_.defaultExportPath;
        dirs["resourcePackPath"] = directorySettings_.resourcePackPath;
        dirs["recursiveSearch"] = directorySettings_.recursiveSearch;
        
        Json::Value songDirs(Json::arrayValue);
        for (const auto& dir : directorySettings_.songDirectories) {
            songDirs.append(dir);
        }
        dirs["songDirectories"] = songDirs;
        
        Json::Value formats(Json::arrayValue);
        for (const auto& format : directorySettings_.supportedFormats) {
            formats.append(format);
        }
        dirs["supportedFormats"] = formats;
        root["directories"] = dirs;
        
        // Save UI settings
        Json::Value ui;
        ui["theme"] = uiSettings_.theme;
        ui["fontSize"] = uiSettings_.fontSize;
        ui["showSpectrum"] = uiSettings_.showSpectrum;
        ui["showPitchDetection"] = uiSettings_.showPitchDetection;
        ui["fullscreen"] = uiSettings_.fullscreen;
        ui["windowWidth"] = uiSettings_.windowWidth;
        ui["windowHeight"] = uiSettings_.windowHeight;
        root["ui"] = ui;
        
        // Save karaoke settings
        Json::Value karaoke;
        karaoke["enablePitchDetection"] = karaokeSettings_.enablePitchDetection;
        karaoke["pitchSensitivity"] = karaokeSettings_.pitchSensitivity;
        karaoke["showNoteIndicators"] = karaokeSettings_.showNoteIndicators;
        karaoke["enableScoring"] = karaokeSettings_.enableScoring;
        karaoke["lyricFadeTime"] = karaokeSettings_.lyricFadeTime;
        root["karaoke"] = karaoke;
        
        // Save keybindings
        Json::Value keybindings(Json::arrayValue);
        for (const auto& binding : keyBindings_) {
            Json::Value kb;
            kb["key"] = static_cast<int>(binding.key);
            kb["ctrl"] = binding.ctrl;
            kb["shift"] = binding.shift;
            kb["alt"] = binding.alt;
            kb["action"] = binding.action;
            kb["description"] = binding.description;
            keybindings.append(kb);
        }
        root["keybindings"] = keybindings;
        
        std::ofstream file(path);
        if (!file.is_open()) {
            ErrorHandler::getInstance().logError("Cannot create settings file: " + path);
            return false;
        }
        
        file << root;
        ErrorHandler::getInstance().logInfo("Settings saved to: " + path);
        return true;
        
    } catch (const std::exception& e) {
        ErrorHandler::getInstance().logError("Failed to save settings: " + std::string(e.what()));
        return false;
    }
}

void SettingsManager::resetToDefaults() {
    initializeDefaults();
    notifyChange("all");
}

void SettingsManager::initializeDefaults() {
    // Initialize default audio settings
    audioSettings_.masterVolume = 1.0f;
    audioSettings_.sampleRate = 44100;
    audioSettings_.bufferSize = 1024;
    audioSettings_.enableEqualizer = true;
    audioSettings_.equalizerBandCount = 12;
    initializeDefaultEqualizer(12);
    
    // Initialize default directory settings
    directorySettings_.songDirectories = {"./songs/", "./music/"};
    directorySettings_.defaultExportPath = "./exports/";
    directorySettings_.resourcePackPath = "./assets/resource_packs/";
    directorySettings_.recursiveSearch = true;
    directorySettings_.supportedFormats = {"mp3", "wav", "ogg", "flac", "mid", "midi", "lystr"};
    
    // Initialize default UI settings
    uiSettings_.theme = "default";
    uiSettings_.fontSize = 16;
    uiSettings_.showSpectrum = true;
    uiSettings_.showPitchDetection = true;
    uiSettings_.fullscreen = false;
    uiSettings_.windowWidth = 1280;
    uiSettings_.windowHeight = 720;
    
    // Initialize default karaoke settings
    karaokeSettings_.enablePitchDetection = true;
    karaokeSettings_.pitchSensitivity = 0.7f;
    karaokeSettings_.showNoteIndicators = true;
    karaokeSettings_.enableScoring = true;
    karaokeSettings_.lyricFadeTime = 0.5f;
    
    // Initialize default keybindings
    initializeDefaultKeyBindings();
}

void SettingsManager::initializeDefaultKeyBindings() {
    keyBindings_.clear();
    
    keyBindings_.push_back({SDL_SCANCODE_SPACE, false, false, false, "play_pause", "Play/Pause"});
    keyBindings_.push_back({SDL_SCANCODE_ESCAPE, false, false, false, "stop", "Stop"});
    keyBindings_.push_back({SDL_SCANCODE_X, false, false, false, "toggle_resource_pack_gui", "Toggle Resource Pack GUI"});
    keyBindings_.push_back({SDL_SCANCODE_S, true, false, false, "toggle_song_browser", "Toggle Song Browser"});
    keyBindings_.push_back({SDL_SCANCODE_E, true, false, false, "toggle_equalizer", "Toggle Equalizer"});
    keyBindings_.push_back({SDL_SCANCODE_COMMA, true, false, false, "open_settings", "Open Settings"});
    keyBindings_.push_back({SDL_SCANCODE_F, true, false, false, "search_songs", "Search Songs"});
    keyBindings_.push_back({SDL_SCANCODE_O, true, false, false, "open_file", "Open File"});
    keyBindings_.push_back({SDL_SCANCODE_LEFT, false, false, false, "seek_backward", "Seek Backward"});
    keyBindings_.push_back({SDL_SCANCODE_RIGHT, false, false, false, "seek_forward", "Seek Forward"});
    keyBindings_.push_back({SDL_SCANCODE_UP, false, false, false, "volume_up", "Volume Up"});
    keyBindings_.push_back({SDL_SCANCODE_DOWN, false, false, false, "volume_down", "Volume Down"});
    keyBindings_.push_back({SDL_SCANCODE_F11, false, false, false, "toggle_fullscreen", "Toggle Fullscreen"});
}

void SettingsManager::initializeDefaultEqualizer(int bandCount) {
    audioSettings_.equalizerBands.clear();
    audioSettings_.equalizerBandCount = bandCount;
    
    // Create logarithmically spaced frequency bands
    float minFreq = 31.25f;  // 31.25 Hz
    float maxFreq = 16000.0f; // 16 kHz
    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    
    for (int i = 0; i < bandCount; ++i) {
        float logFreq = logMin + (logMax - logMin) * i / (bandCount - 1);
        float frequency = std::pow(10.0f, logFreq);
        
        EqualizerBand band;
        band.frequency = frequency;
        band.gain = 0.0f; // Flat response by default
        band.enabled = true;
        audioSettings_.equalizerBands.push_back(band);
    }
}

void SettingsManager::setEqualizerBandCount(int count) {
    if (count < 3) count = 3;
    if (count > 48) count = 48;
    
    audioSettings_.equalizerBandCount = count;
    initializeDefaultEqualizer(count);
    notifyChange("equalizer");
}

void SettingsManager::setEqualizerBand(int index, float frequency, float gain) {
    if (index >= 0 && index < static_cast<int>(audioSettings_.equalizerBands.size())) {
        audioSettings_.equalizerBands[index].frequency = frequency;
        audioSettings_.equalizerBands[index].gain = std::max(-20.0f, std::min(20.0f, gain));
        notifyChange("equalizer");
    }
}

void SettingsManager::addSongDirectory(const std::string& path) {
    auto& dirs = directorySettings_.songDirectories;
    if (std::find(dirs.begin(), dirs.end(), path) == dirs.end()) {
        dirs.push_back(path);
        notifyChange("directories");
    }
}

void SettingsManager::removeSongDirectory(const std::string& path) {
    auto& dirs = directorySettings_.songDirectories;
    dirs.erase(std::remove(dirs.begin(), dirs.end(), path), dirs.end());
    notifyChange("directories");
}

void SettingsManager::setKeyBinding(const std::string& action, SDL_Scancode key, bool ctrl, bool shift, bool alt) {
    // Remove existing binding for this action
    keyBindings_.erase(
        std::remove_if(keyBindings_.begin(), keyBindings_.end(),
            [&action](const KeyBinding& kb) { return kb.action == action; }),
        keyBindings_.end()
    );
    
    // Add new binding
    KeyBinding newBinding;
    newBinding.key = key;
    newBinding.ctrl = ctrl;
    newBinding.shift = shift;
    newBinding.alt = alt;
    newBinding.action = action;
    newBinding.description = action; // Default description
    keyBindings_.push_back(newBinding);
    
    notifyChange("keybindings");
}

KeyBinding* SettingsManager::findKeyBinding(SDL_Scancode key, bool ctrl, bool shift, bool alt) {
    for (auto& binding : keyBindings_) {
        if (binding.key == key && binding.ctrl == ctrl && 
            binding.shift == shift && binding.alt == alt) {
            return &binding;
        }
    }
    return nullptr;
}

std::string SettingsManager::getKeyBindingString(const KeyBinding& binding) const {
    std::string result;
    
    if (binding.ctrl) result += "Ctrl+";
    if (binding.shift) result += "Shift+";
    if (binding.alt) result += "Alt+";
    
    result += SDL_GetScancodeName(binding.key);
    return result;
}

void SettingsManager::notifyChange(const std::string& setting) {
    if (changeCallback_) {
        changeCallback_(setting);
    }
}

} // namespace Lyricstator
