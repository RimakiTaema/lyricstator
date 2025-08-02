#include "audio/AudioManager.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Lyricstator {

AudioManager::AudioManager()
    : music_(nullptr)
    , chunk_(nullptr)
    , volume_(1.0f)
    , tempoMultiplier_(1.0f)
    , initialized_(false)
    , isPlaying_(false)
    , isPaused_(false)
    , startTime_(0)
    , pauseTime_(0)
    , seekOffset_(0)
    , rmsLevel_(0.0f)
{
    audioFormat_.sampleRate = 44100;
    audioFormat_.channels = 2;
    audioFormat_.bitDepth = 16;
    audioFormat_.format = "unknown";
    
    spectrumBuffer_.resize(64, 0.0f);
}

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Initialize() {
    if (initialized_) {
        return true;
    }
    
    std::cout << "Initializing AudioManager..." << std::endl;
    
    // SDL_mixer should already be initialized by Application
    // Set up audio format detection and analysis
    
    initialized_ = true;
    std::cout << "AudioManager initialized successfully" << std::endl;
    return true;
}

void AudioManager::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    Stop();
    UnloadAudio();
    
    initialized_ = false;
    std::cout << "AudioManager shutdown complete" << std::endl;
}

bool AudioManager::LoadAudio(const std::string& filepath) {
    std::cout << "Loading audio file: " << filepath << std::endl;
    
    UnloadAudio();
    
    // Detect audio format
    audioFormat_ = DetectAudioFormat(filepath);
    
    // Try loading as music first (for longer files like MP3, OGG)
    if (LoadMusic(filepath)) {
        currentFile_ = filepath;
        std::cout << "Loaded as music: " << filepath << std::endl;
        return true;
    }
    
    // Fallback to loading as chunk (for WAV files)
    if (LoadChunk(filepath)) {
        currentFile_ = filepath;
        std::cout << "Loaded as sound chunk: " << filepath << std::endl;
        return true;
    }
    
    std::cerr << "Failed to load audio file: " << filepath << std::endl;
    std::cerr << "SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
}

bool AudioManager::LoadMusic(const std::string& filepath) {
    music_ = Mix_LoadMUS(filepath.c_str());
    return music_ != nullptr;
}

bool AudioManager::LoadChunk(const std::string& filepath) {
    chunk_ = Mix_LoadWAV(filepath.c_str());
    return chunk_ != nullptr;
}

void AudioManager::UnloadAudio() {
    Stop();
    
    if (music_) {
        Mix_FreeMusic(music_);
        music_ = nullptr;
    }
    
    if (chunk_) {
        Mix_FreeChunk(chunk_);
        chunk_ = nullptr;
    }
    
    currentFile_.clear();
}

void AudioManager::Play() {
    if (!initialized_ || (!music_ && !chunk_)) {
        return;
    }
    
    if (isPaused_) {
        // Resume from pause
        if (music_) {
            Mix_ResumeMusic();
        } else {
            // For chunks, we need to restart from the pause position
            // This is a limitation of SDL_mixer
        }
        isPaused_ = false;
        isPlaying_ = true;
        
        // Adjust start time to account for pause duration
        uint32_t pauseDuration = SDL_GetTicks() - pauseTime_;
        startTime_ += pauseDuration;
    } else {
        // Start from beginning or seek position
        if (music_) {
            if (Mix_PlayMusic(music_, 0) == 0) {
                isPlaying_ = true;
                startTime_ = SDL_GetTicks() - seekOffset_;
            }
        } else if (chunk_) {
            if (Mix_PlayChannel(-1, chunk_, 0) >= 0) {
                isPlaying_ = true;
                startTime_ = SDL_GetTicks() - seekOffset_;
            }
        }
    }
    
    std::cout << "Audio playback started" << std::endl;
}

void AudioManager::Pause() {
    if (!isPlaying_ || isPaused_) {
        return;
    }
    
    if (music_) {
        Mix_PauseMusic();
    } else {
        Mix_Pause(-1); // Pause all channels
    }
    
    isPaused_ = true;
    pauseTime_ = SDL_GetTicks();
    std::cout << "Audio playback paused" << std::endl;
}

void AudioManager::Stop() {
    if (!isPlaying_ && !isPaused_) {
        return;
    }
    
    if (music_) {
        Mix_HaltMusic();
    } else {
        Mix_HaltChannel(-1); // Stop all channels
    }
    
    isPlaying_ = false;
    isPaused_ = false;
    seekOffset_ = 0;
    std::cout << "Audio playback stopped" << std::endl;
}

void AudioManager::Seek(uint32_t timeMs) {
    bool wasPlaying = isPlaying_;
    
    Stop();
    seekOffset_ = timeMs;
    
    if (wasPlaying) {
        Play();
    }
    
    std::cout << "Seeked to: " << timeMs << "ms" << std::endl;
}

void AudioManager::SetVolume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
    
    int mixVolume = static_cast<int>(volume_ * MIX_MAX_VOLUME);
    
    if (music_) {
        Mix_VolumeMusic(mixVolume);
    }
    
    if (chunk_) {
        Mix_VolumeChunk(chunk_, mixVolume);
    }
}

void AudioManager::SetTempo(float multiplier) {
    tempoMultiplier_ = std::max(0.1f, std::min(4.0f, multiplier));
    // Note: SDL_mixer doesn't support tempo changes directly
    // This would require additional audio processing libraries like SoundTouch
    std::cout << "Tempo set to: " << tempoMultiplier_ << "x (not implemented in SDL_mixer)" << std::endl;
}

bool AudioManager::IsPlaying() const {
    if (music_) {
        return Mix_PlayingMusic() && !isPaused_;
    } else if (chunk_) {
        return Mix_Playing(-1) > 0 && !isPaused_;
    }
    return false;
}

bool AudioManager::IsPaused() const {
    if (music_) {
        return Mix_PausedMusic() || isPaused_;
    } else {
        return Mix_Paused(-1) > 0 || isPaused_;
    }
}

uint32_t AudioManager::GetCurrentTimeMs() const {
    if (!isPlaying_ && !isPaused_) {
        return seekOffset_;
    }
    
    if (isPaused_) {
        return (pauseTime_ - startTime_);
    }
    
    return (SDL_GetTicks() - startTime_);
}

uint32_t AudioManager::GetDurationMs() const {
    // Note: SDL_mixer doesn't provide duration information
    // This would require additional libraries like libsndfile for WAV
    // or taglib for MP3/OGG to read metadata
    return 0; // Placeholder
}

void AudioManager::Update(float deltaTime) {
    UpdatePlaybackTime();
    UpdateAudioAnalysis();
}

void AudioManager::UpdatePlaybackTime() {
    // Check if playback has finished
    if (isPlaying_ && !IsPlaying()) {
        isPlaying_ = false;
        isPaused_ = false;
        std::cout << "Audio playback finished" << std::endl;
    }
}

void AudioManager::UpdateAudioAnalysis() {
    // Placeholder for audio analysis
    // Real implementation would require access to audio stream data
    // This could be implemented with SDL audio callbacks or additional libraries
    
    // Generate some fake spectrum data for demonstration
    if (isPlaying_) {
        for (size_t i = 0; i < spectrumBuffer_.size(); ++i) {
            spectrumBuffer_[i] = std::sin(SDL_GetTicks() * 0.001f + i * 0.1f) * 0.5f + 0.5f;
            spectrumBuffer_[i] *= (1.0f - static_cast<float>(i) / spectrumBuffer_.size()); // Decay with frequency
        }
        
        // Calculate RMS level
        float sum = 0.0f;
        for (float val : spectrumBuffer_) {
            sum += val * val;
        }
        rmsLevel_ = std::sqrt(sum / spectrumBuffer_.size());
    } else {
        // Clear spectrum when not playing
        std::fill(spectrumBuffer_.begin(), spectrumBuffer_.end(), 0.0f);
        rmsLevel_ = 0.0f;
    }
}

std::vector<float> AudioManager::GetSpectrumData(int numBands) {
    if (numBands <= 0 || numBands > static_cast<int>(spectrumBuffer_.size())) {
        return spectrumBuffer_;
    }
    
    std::vector<float> result(numBands);
    float scale = static_cast<float>(spectrumBuffer_.size()) / numBands;
    
    for (int i = 0; i < numBands; ++i) {
        int index = static_cast<int>(i * scale);
        result[i] = spectrumBuffer_[index];
    }
    
    return result;
}

float AudioManager::GetRMSLevel() {
    return rmsLevel_;
}

AudioFormat AudioManager::DetectAudioFormat(const std::string& filepath) {
    AudioFormat format;
    format.sampleRate = 44100; // Default
    format.channels = 2;       // Default stereo
    format.bitDepth = 16;      // Default
    
    // Extract file extension
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string extension = filepath.substr(dotPos + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        format.format = extension;
    }
    
    // Note: For real format detection, we would need to parse file headers
    // or use libraries like libsndfile, libav, etc.
    
    return format;
}

} // namespace Lyricstator