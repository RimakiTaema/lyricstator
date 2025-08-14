#pragma once
#include "common/Types.h"
#include <memory>
#include <string>

// Forward declarations
struct Mix_Chunk;
struct Mix_Music;

namespace Lyricstator {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // File loading
    bool LoadAudio(const std::string& filepath);
    void UnloadAudio();
    
    // Playback control
    void Play();
    void Pause();
    void Stop();
    void Seek(uint32_t timeMs);
    
    // Audio properties
    void SetVolume(float volume);
    void SetTempo(float multiplier);
    float GetVolume() const { return volume_; }
    float getTempo() const { return tempoMultiplier_; }
    
    // Status and timing
    bool IsPlaying() const;
    bool IsPaused() const;
    uint32_t GetCurrentTimeMs() const;
    uint32_t GetDurationMs() const;
    AudioFormat GetAudioFormat() const { return audioFormat_; }
    
    // Update (called per frame)
    void Update(float deltaTime);
    
    // Audio analysis (for visualization)
    std::vector<float> GetSpectrumData(int numBands = 64);
    float GetRMSLevel();
    
private:
    // SDL_mixer resources
    Mix_Music* music_;
    Mix_Chunk* chunk_;
    
    // Audio properties
    AudioFormat audioFormat_;
    float volume_;
    float tempoMultiplier_;
    
    // Playback state
    bool initialized_;
    bool isPlaying_;
    bool isPaused_;
    uint32_t startTime_;
    uint32_t pauseTime_;
    uint32_t seekOffset_;
    std::string currentFile_;
    
    // Internal methods
    bool LoadMusic(const std::string& filepath);
    bool LoadChunk(const std::string& filepath);
    AudioFormat DetectAudioFormat(const std::string& filepath);
    void UpdatePlaybackTime();
    
    // Audio analysis data
    std::vector<float> spectrumBuffer_;
    float rmsLevel_;
    void UpdateAudioAnalysis();
};

} // namespace Lyricstator
