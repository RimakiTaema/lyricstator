#pragma once
#include "common/Types.h"
#include <vector>
#include <memory>
#include <functional>

namespace Lyricstator {

// Abstract base class for pitch detection algorithms
class PitchDetectionAlgorithm {
public:
    virtual ~PitchDetectionAlgorithm() = default;
    virtual PitchDetectionResult DetectPitch(const std::vector<float>& audioSamples, int sampleRate) = 0;
    virtual std::string GetAlgorithmName() const = 0;
    virtual void Reset() = 0;
};

// YIN algorithm implementation (stub)
class YinAlgorithm : public PitchDetectionAlgorithm {
public:
    YinAlgorithm();
    PitchDetectionResult DetectPitch(const std::vector<float>& audioSamples, int sampleRate) override;
    std::string GetAlgorithmName() const override { return "YIN"; }
    void Reset() override;
    
private:
    std::vector<float> yinBuffer_;
    float threshold_;
};

// Autocorrelation algorithm implementation (stub)
class AutocorrelationAlgorithm : public PitchDetectionAlgorithm {
public:
    AutocorrelationAlgorithm();
    PitchDetectionResult DetectPitch(const std::vector<float>& audioSamples, int sampleRate) override;
    std::string GetAlgorithmName() const override { return "Autocorrelation"; }
    void Reset() override;
    
private:
    std::vector<float> correlationBuffer_;
    int minPeriod_;
    int maxPeriod_;
};

// FFT-based algorithm implementation (stub)
class FFTAlgorithm : public PitchDetectionAlgorithm {
public:
    FFTAlgorithm();
    PitchDetectionResult DetectPitch(const std::vector<float>& audioSamples, int sampleRate) override;
    std::string GetAlgorithmName() const override { return "FFT"; }
    void Reset() override;
    
private:
    std::vector<float> fftBuffer_;
    std::vector<float> window_;
    int fftSize_;
};

// Main note detector class
class NoteDetector {
public:
    enum class Algorithm {
        YIN,
        AUTOCORRELATION, 
        FFT,
        HYBRID
    };
    
    NoteDetector();
    ~NoteDetector();
    
    // Initialization
    bool Initialize(int sampleRate = 44100, int bufferSize = 1024);
    void Shutdown();
    
    // Algorithm selection
    void SetAlgorithm(Algorithm algorithm);
    Algorithm GetCurrentAlgorithm() const { return currentAlgorithm_; }
    std::vector<std::string> GetAvailableAlgorithms() const;
    
    // Detection parameters
    void SetSensitivity(float sensitivity); // 0.0 - 1.0
    void SetFrequencyRange(float minHz, float maxHz);
    void SetConfidenceThreshold(float threshold);
    
    // Audio input
    void ProcessAudioBuffer(const float* samples, int numSamples);
    void ProcessAudioBuffer(const std::vector<float>& samples);
    
    // Detection results
    PitchDetectionResult DetectPitch();
    PitchDetectionResult GetLastDetection() const { return lastResult_; }
    
    // Analysis features
    std::vector<PitchDetectionResult> GetDetectionHistory(int maxResults = 100) const;
    float GetAverageConfidence(int windowSize = 10) const;
    bool IsVoiceActive() const;
    
    // Calibration and training (for future ML integration)
    void StartCalibration();
    void StopCalibration();
    bool IsCalibrating() const { return calibrating_; }
    bool SaveCalibrationData(const std::string& filepath);
    bool LoadCalibrationData(const std::string& filepath);
    
    // Real-time processing
    void SetRealTimeMode(bool enabled);
    void SetCallback(std::function<void(const PitchDetectionResult&)> callback);
    
private:
    // Algorithm instances
    std::unique_ptr<PitchDetectionAlgorithm> yinAlgorithm_;
    std::unique_ptr<PitchDetectionAlgorithm> autocorrelationAlgorithm_;
    std::unique_ptr<PitchDetectionAlgorithm> fftAlgorithm_;
    PitchDetectionAlgorithm* currentAlgorithmPtr_;
    Algorithm currentAlgorithm_;
    
    // Audio processing
    std::vector<float> audioBuffer_;
    std::vector<float> processBuffer_;
    int sampleRate_;
    int bufferSize_;
    int writeIndex_;
    bool bufferFull_;
    
    // Detection parameters
    float sensitivity_;
    float minFrequency_;
    float maxFrequency_;
    float confidenceThreshold_;
    
    // Detection state
    PitchDetectionResult lastResult_;
    std::vector<PitchDetectionResult> detectionHistory_;
    bool initialized_;
    bool realTimeMode_;
    
    // Voice activity detection
    float voiceActivityThreshold_;
    std::vector<float> energyBuffer_;
    int energyBufferIndex_;
    
    // Calibration
    bool calibrating_;
    std::vector<PitchDetectionResult> calibrationData_;
    
    // Callback for real-time processing
    std::function<void(const PitchDetectionResult&)> detectionCallback_;
    
    // Internal methods
    void UpdateDetectionHistory(const PitchDetectionResult& result);
    bool IsValidFrequency(float frequency) const;
    float CalculateVoiceActivity(const std::vector<float>& samples);
    PitchDetectionResult FilterResult(const PitchDetectionResult& rawResult);
    void ApplyTemporalSmoothing(PitchDetectionResult& result);
    
    // Audio preprocessing
    void ApplyPreEmphasis(std::vector<float>& samples);
    void ApplyWindowing(std::vector<float>& samples);
    void RemoveDCOffset(std::vector<float>& samples);
    
    // Utility functions
    float HzToMidi(float frequency);
    float MidiToHz(int midiNote);
    std::string FrequencyToNoteName(float frequency);
};

} // namespace Lyricstator