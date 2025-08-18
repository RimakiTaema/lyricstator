#include "core/Application.h"
#include "audio/AudioManager.h"
#include "audio/MidiParser.h"
#include "ai/NoteDetector.h"
#include "scripting/LystrParser.h"
#include "scripting/LystrInterpreter.h"
#include "gui/Window.h"
#include "gui/TGUIKaraokeDisplay.h"
#include "gui/UserInterface.h"
#include "gui/TGUIResourcePackGUI.h"
#include "gui/TGUISongBrowser.h" // Added TGUISongBrowser include
#include "gui/TGUIEqualizer.h" // Added TGUIEqualizer include
#include "gui/TGUIKeybindEditor.h" // Added TGUIKeybindEditor include
#include "sync/SynchronizationManager.h"
#include "export/FormatExporter.h"
#include "utils/ErrorHandler.h"
#include "core/AssetManager.h"
#include "core/SettingsManager.h" // Added SettingsManager include

#include <TGUI/TGUI.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

namespace Lyricstator {

Application::Application()
    : running_(false)
    , initialized_(false)
    , playbackState_(PlaybackState::STOPPED)
    , windowWidth_(1280)
    , windowHeight_(720)
    , volume_(1.0f)
    , tempoMultiplier_(1.0f)
    , pitchDetectionEnabled_(true)
    , lastFrameTime_(std::chrono::steady_clock::now())
    , settingsManager_(nullptr) // Initialize settingsManager pointer
{
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    if (initialized_) {
        return true;
    }
    
    std::cout << "Initializing Lyricstator..." << std::endl;
    
    InitializeSettings();
    
    // Initialize SDL
    if (!InitializeSDL()) {
        return false;
    }
    
    // Create subsystems
    try {
        errorHandler_ = std::make_unique<ErrorHandler>();
        assetManager_ = std::make_unique<AssetManager>();
        audioManager_ = std::make_unique<AudioManager>();
        midiParser_ = std::make_unique<MidiParser>();
        noteDetector_ = std::make_unique<NoteDetector>();
        lystrParser_ = std::make_unique<LystrParser>();
        lystrInterpreter_ = std::make_unique<LystrInterpreter>();
        window_ = std::make_unique<Window>();
        karaokeDisplay_ = std::make_unique<TGUIKaraokeDisplay>();
        userInterface_ = std::make_unique<UserInterface>();
        resourcePackGUI_ = std::make_unique<TGUIResourcePackGUI>();
        songBrowser_ = std::make_unique<TGUISongBrowser>(); // Create TGUISongBrowser instance
        equalizer_ = std::make_unique<TGUIEqualizer>(); // Create TGUIEqualizer instance
        keybindEditor_ = std::make_unique<TGUIKeybindEditor>(); // Create TGUIKeybindEditor instance
        syncManager_ = std::make_unique<SynchronizationManager>();
        formatExporter_ = std::make_unique<FormatExporter>();
        
        // Initialize subsystems
        if (!window_->Initialize(windowWidth_, windowHeight_, "Lyricstator")) {
            std::cerr << "Failed to initialize window" << std::endl;
            return false;
        }
        
        gui_ = std::make_unique<tgui::Gui>();
        gui_->setTarget(*window_->GetWindow());
        
        if (!assetManager_->Initialize()) {
            std::cerr << "Failed to initialize asset manager" << std::endl;
            return false;
        }
        
        if (!audioManager_->Initialize()) {
            std::cerr << "Failed to initialize audio manager" << std::endl;
            return false;
        }
        
        if (!karaokeDisplay_->Initialize(*gui_, assetManager_.get())) {
            std::cerr << "Failed to initialize karaoke display" << std::endl;
            return false;
        }
        
        if (!userInterface_->Initialize(window_->GetRenderer())) {
            std::cerr << "Failed to initialize user interface" << std::endl;
            return false;
        }
        
        if (!resourcePackGUI_->Initialize(*gui_, assetManager_.get())) {
            std::cerr << "Failed to initialize resource pack GUI" << std::endl;
            return false;
        }
        
        if (!songBrowser_->Initialize(gui_.get())) {
            std::cerr << "Failed to initialize song browser" << std::endl;
            return false;
        }
        
        if (!equalizer_->Initialize(gui_.get())) {
            std::cerr << "Failed to initialize equalizer" << std::endl;
            return false;
        }
        
        if (!keybindEditor_->Initialize(gui_.get())) {
            std::cerr << "Failed to initialize keybind editor" << std::endl;
            return false;
        }
        
        keybindEditor_->SetKeybindChangedCallback([this](const std::string& action, SDL_Scancode key, bool ctrl, bool shift, bool alt) {
            std::cout << "Keybinding changed: " << action << " -> " << SDL_GetScancodeName(key) << std::endl;
        });
        
        equalizer_->SetBandChangedCallback([this](int bandIndex, float gain) {
            // Apply equalizer changes to audio system
            std::cout << "Equalizer band " << bandIndex << " changed to " << gain << "dB" << std::endl;
        });
        
        equalizer_->SetEqualizerToggleCallback([this](bool enabled) {
            std::cout << "Equalizer " << (enabled ? "enabled" : "disabled") << std::endl;
        });
        
        songBrowser_->SetSongSelectedCallback([this](const SongInfo& song) {
            std::string extension = song.format;
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            
            if (extension == "mid" || extension == "midi") {
                LoadMidiFile(song.filepath);
            } else if (extension == "wav" || extension == "mp3" || extension == "ogg" || extension == "flac") {
                LoadAudioFile(song.filepath);
            } else if (extension == "lystr") {
                LoadLyricScript(song.filepath);
            }
        });
        
        if (!syncManager_->Initialize()) {
            std::cerr << "Failed to initialize synchronization manager" << std::endl;
            return false;
        }
        
        // Setup event callback
        eventCallback_ = [this](const AppEvent& event) {
            OnEvent(event);
        };
        
        initialized_ = true;
        std::cout << "Lyricstator initialized successfully!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

bool Application::InitializeSDL() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    
    return true;
}

void Application::Run() {
    if (!initialized_) {
        std::cerr << "Application not initialized!" << std::endl;
        return;
    }
    
    running_ = true;
    lastFrameTime_ = std::chrono::steady_clock::now();
    
    std::cout << "Starting main loop..." << std::endl;
    
    while (running_) {
        float deltaTime = GetDeltaTime();
        
        HandleSDLEvents();
        ProcessEvents();
        UpdateSystems(deltaTime);
        
        window_->Clear();
        gui_->draw();
        userInterface_->Render();
        window_->Present();
        
        SDL_Delay(16);
    }
    
    std::cout << "Main loop ended" << std::endl;
}

void Application::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    std::cout << "Shutting down Lyricstator..." << std::endl;
    
    running_ = false;
    
    if (settingsManager_) {
        settingsManager_->saveSettings();
    }
    
    formatExporter_.reset();
    syncManager_.reset();
    keybindEditor_.reset();
    equalizer_.reset();
    songBrowser_.reset();
    resourcePackGUI_.reset();
    userInterface_.reset();
    karaokeDisplay_.reset();
    window_.reset();
    lystrInterpreter_.reset();
    lystrParser_.reset();
    noteDetector_.reset();
    midiParser_.reset();
    audioManager_.reset();
    assetManager_.reset();
    errorHandler_.reset();
    
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();
    
    initialized_ = false;
    std::cout << "Shutdown complete" << std::endl;
}

void Application::HandleSDLEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        gui_->handleEvent(event);
        
        ProcessKeyboardInput(event);
        
        switch (event.type) {
            case SDL_QUIT:
                running_ = false;
                break;
            case SDL_DROPFILE:
                if (event.drop.file) {
                    std::string filepath = event.drop.file;
                    std::string extension = filepath.substr(filepath.find_last_of('.'));
                    
                    if (extension == ".mid" || extension == ".midi") {
                        LoadMidiFile(filepath);
                    } else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
                        LoadAudioFile(filepath);
                    } else if (extension == ".lystr") {
                        LoadLyricScript(filepath);
                    } else {
                        ShowErrorDialog("Unsupported file format: " + extension, ErrorType::UNSUPPORTED_FORMAT);
                    }
                    
                    SDL_free(event.drop.file);
                }
                break;
        }
        
        if (!resourcePackGUI_->IsVisible() && !songBrowser_->IsVisible() && !equalizer_->IsVisible() && !keybindEditor_->IsVisible()) {
            userInterface_->HandleEvent(event);
        }
    }
}

void Application::UpdateSystems(float deltaTime) {
    if (playbackState_ == PlaybackState::PLAYING) {
        uint32_t currentTime = GetCurrentTimeMs();
        
        audioManager_->Update(deltaTime);
        
        if (pitchDetectionEnabled_ && noteDetector_) {
            auto detectionResult = noteDetector_->DetectPitch();
            if (detectionResult.voiceDetected) {
                PushEvent(AppEvent(EventType::NOTE_DETECTED, "", &detectionResult));
            }
        }
        
        syncManager_->Update(currentTime);
        
        lystrInterpreter_->Update(currentTime);
        
        if (equalizer_->IsVisible()) {
            auto spectrumData = audioManager_->GetSpectrumData(64);
            equalizer_->UpdateSpectrum(spectrumData);
        }
    }
    
    karaokeDisplay_->Update(deltaTime);
    userInterface_->Update(deltaTime);
    resourcePackGUI_->Update(deltaTime);
    songBrowser_->Update(deltaTime);
    equalizer_->Update(deltaTime);
    keybindEditor_->Update(deltaTime);
}

float Application::GetDeltaTime() {
    auto currentTime = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime_);
    lastFrameTime_ = currentTime;
    return delta.count() / 1000000.0f;
}

bool Application::LoadAudioFile(const std::string& filepath) {
    std::cout << "Loading audio file: " << filepath << std::endl;
    
    if (!audioManager_->LoadAudio(filepath)) {
        ShowErrorDialog("Failed to load audio file: " + filepath, ErrorType::AUDIO_ERROR);
        return false;
    }
    
    currentAudioFile_ = filepath;
    PushEvent(AppEvent(EventType::AUDIO_LOADED, filepath));
    return true;
}

bool Application::LoadMidiFile(const std::string& filepath) {
    std::cout << "Loading MIDI file: " << filepath << std::endl;
    
    if (!midiParser_->LoadMidiFile(filepath)) {
        ShowErrorDialog("Failed to load MIDI file: " + filepath, ErrorType::PARSING_ERROR);
        return false;
    }
    
    currentMidiFile_ = filepath;
    PushEvent(AppEvent(EventType::MIDI_LOADED, filepath));
    return true;
}

bool Application::LoadLyricScript(const std::string& filepath) {
    std::cout << "Loading lyric script: " << filepath << std::endl;
    
    if (!lystrParser_->ParseFile(filepath)) {
        ShowErrorDialog("Failed to parse lyric script: " + filepath, ErrorType::PARSING_ERROR);
        return false;
    }
    
    auto commands = lystrParser_->GetCommands();
    lystrInterpreter_->LoadScript(commands);
    
    currentLyricScript_ = filepath;
    PushEvent(AppEvent(EventType::LYRIC_SCRIPT_LOADED, filepath));
    return true;
}

void Application::Play() {
    if (playbackState_ == PlaybackState::PLAYING) {
        return;
    }
    
    if (!currentAudioFile_.empty() || !currentMidiFile_.empty()) {
        audioManager_->Play();
        playbackState_ = PlaybackState::PLAYING;
        PushEvent(AppEvent(EventType::PLAYBACK_STARTED));
        std::cout << "Playback started" << std::endl;
    }
}

void Application::Pause() {
    if (playbackState_ == PlaybackState::PLAYING) {
        audioManager_->Pause();
        playbackState_ = PlaybackState::PAUSED;
        PushEvent(AppEvent(EventType::PLAYBACK_PAUSED));
        std::cout << "Playback paused" << std::endl;
    }
}

void Application::Stop() {
    if (playbackState_ != PlaybackState::STOPPED) {
        audioManager_->Stop();
        playbackState_ = PlaybackState::STOPPED;
        PushEvent(AppEvent(EventType::PLAYBACK_STOPPED));
        std::cout << "Playback stopped" << std::endl;
    }
}

void Application::Seek(uint32_t timeMs) {
    audioManager_->Seek(timeMs);
    syncManager_->Seek(timeMs);
    lystrInterpreter_->Seek(timeMs);
}

void Application::SetTempo(float multiplier) {
    tempoMultiplier_ = multiplier;
    audioManager_->SetTempo(multiplier);
}

uint32_t Application::GetCurrentTimeMs() const {
    return audioManager_->GetCurrentTimeMs();
}

void Application::PushEvent(const AppEvent& event) {
    eventQueue_.push(event);
}

void Application::ProcessEvents() {
    while (!eventQueue_.empty()) {
        auto event = eventQueue_.front();
        eventQueue_.pop();
        
        if (eventCallback_) {
            eventCallback_(event);
        }
    }
}

void Application::OnEvent(const AppEvent& event) {
    switch (event.type) {
        case EventType::LYRIC_HIGHLIGHT:
            karaokeDisplay_->SetLyric(event.data);
            break;
        case EventType::NOTE_DETECTED:
            if (event.userData) {
                auto* result = static_cast<PitchDetectionResult*>(event.userData);
                karaokeDisplay_->UpdatePitchAccuracy(result->confidence);
            }
            break;
        case EventType::ERROR_OCCURRED:
            std::cerr << "Error: " << event.data << std::endl;
            break;
        default:
            break;
    }
}

void Application::ShowErrorDialog(const std::string& message, ErrorType type) {
    errorHandler_->ShowError(message, type);
    PushEvent(AppEvent(EventType::ERROR_OCCURRED, message));
}

bool Application::ExportProject(const std::string& filepath, ExportFormat format) {
    return formatExporter_->Export(filepath, format, currentAudioFile_, currentMidiFile_, currentLyricScript_);
}

void Application::SetWindowSize(int width, int height) {
    windowWidth_ = width;
    windowHeight_ = height;
    if (window_) {
        window_->Resize(width, height);
    }
}

void Application::SetVolume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
    if (audioManager_) {
        audioManager_->SetVolume(volume_);
    }
}

void Application::SetPitchDetectionEnabled(bool enabled) {
    pitchDetectionEnabled_ = enabled;
}

void Application::InitializeSettings() {
    settingsManager_ = &SettingsManager::getInstance();
    settingsManager_->loadSettings();
    
    // Apply settings to application
    auto& uiSettings = settingsManager_->getUISettings();
    windowWidth_ = uiSettings.windowWidth;
    windowHeight_ = uiSettings.windowHeight;
    
    auto& audioSettings = settingsManager_->getAudioSettings();
    volume_ = audioSettings.masterVolume;
    
    // Setup settings change callback
    settingsManager_->setChangeCallback([this](const std::string& setting) {
        OnSettingsChanged(setting);
    });
}

void Application::OnSettingsChanged(const std::string& setting) {
    if (setting == "audio" || setting == "all") {
        auto& audioSettings = settingsManager_->getAudioSettings();
        SetVolume(audioSettings.masterVolume);
    }
    
    if (setting == "ui" || setting == "all") {
        auto& uiSettings = settingsManager_->getUISettings();
        if (uiSettings.windowWidth != windowWidth_ || uiSettings.windowHeight != windowHeight_) {
            SetWindowSize(uiSettings.windowWidth, uiSettings.windowHeight);
        }
    }
    
    if (setting == "directories" || setting == "all") {
        // Refresh song browser when directories change
        if (songBrowser_) {
            songBrowser_->RefreshSongList();
        }
    }
    
    if (setting == "equalizer" || setting == "all") {
        auto& audioSettings = settingsManager_->getAudioSettings();
        if (equalizer_) {
            equalizer_->SetBandCount(audioSettings.equalizerBandCount);
            equalizer_->EnableEqualizer(audioSettings.enableEqualizer);
        }
    }
}

void Application::ProcessKeyboardInput(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        SDL_Scancode scancode = event.key.keysym.scancode;
        bool ctrl = (event.key.keysym.mod & KMOD_CTRL) != 0;
        bool shift = (event.key.keysym.mod & KMOD_SHIFT) != 0;
        bool alt = (event.key.keysym.mod & KMOD_ALT) != 0;
        
        // Find matching keybinding
        auto* binding = settingsManager_->findKeyBinding(scancode, ctrl, shift, alt);
        if (binding) {
            HandleKeyBinding(binding->action);
        }
    }
}

void Application::HandleKeyBinding(const std::string& action) {
    if (action == "play_pause") {
        if (playbackState_ == PlaybackState::PLAYING) {
            Pause();
        } else {
            Play();
        }
    } else if (action == "stop") {
        Stop();
    } else if (action == "toggle_resource_pack_gui") {
        resourcePackGUI_->Toggle();
    } else if (action == "toggle_song_browser") {
        songBrowser_->Toggle();
    } else if (action == "toggle_equalizer") {
        equalizer_->Toggle();
    } else if (action == "open_settings") {
        keybindEditor_->Toggle();
    } else if (action == "search_songs") {
        if (!songBrowser_->IsVisible()) {
            songBrowser_->Show();
        }
    } else if (action == "volume_up") {
        SetVolume(volume_ + 0.1f);
    } else if (action == "volume_down") {
        SetVolume(volume_ - 0.1f);
    } else if (action == "seek_forward") {
        Seek(GetCurrentTimeMs() + 5000); // 5 seconds forward
    } else if (action == "seek_backward") {
        uint32_t currentTime = GetCurrentTimeMs();
        Seek(currentTime > 5000 ? currentTime - 5000 : 0); // 5 seconds backward
    }
}

} // namespace Lyricstator
