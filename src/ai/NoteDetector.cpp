#include "ai/NoteDetector.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <fstream>

namespace Lyricstator {

// YIN Algorithm Implementation (Stub)
YinAlgorithm::YinAlgorithm() : threshold_(0.15f) {
    yinBuffer_.resize(2048);
}

PitchDetectionResult YinAlgorithm::DetectPitch(const std::vector<float>& audioSamples, int sampleRate) {
    // Stub implementation - generates realistic-looking fake pitch data
    PitchDetectionResult result;
    
    // Calculate energy to determine voice activity
    float energy = 0.0f;
    for (float sample : audioSamples) {
        energy += sample * sample;
    }
    energy /= audioSamples.size();
    
    if (energy > 0.001f) { // Voice activity threshold
        // Generate a pitch in vocal range (80-800 Hz) with some randomness
        static std::mt19937 gen(42); // Fixed seed for reproducible results
        std::uniform_real_distribution<float> pitchDist(150.0f, 400.0f);
        std::uniform_real_distribution<float> confidenceDist(0.7f, 0.95f);
        
        result.frequency = pitchDist(gen);
        result.confidence = confidenceDist(gen);
        result.voiceDetected = true;
    } else {
        result.frequency = 0.0f;
        result.confidence = 0.0f;
        result.voiceDetected = false;
    }
    
    result.timestamp = 0; // Will be set by caller
    return result;
}

void YinAlgorithm::Reset() {
    std::fill(yinBuffer_.begin(), yinBuffer_.end(), 0.0f);
}

// Autocorrelation Algorithm Implementation (Stub)
AutocorrelationAlgorithm::AutocorrelationAlgorithm() 
    : minPeriod_(20)
    , maxPeriod_(400)
{
    correlationBuffer_.resize(1024);
}

PitchDetectionResult AutocorrelationAlgorithm::DetectPitch(const std::vector<float>& audioSamples, int sampleRate) {
    // Stub implementation
    PitchDetectionResult result;
    
    float energy = 0.0f;
    for (float sample : audioSamples) {
        energy += sample * sample;
    }
    energy = std::sqrt(energy / audioSamples.size());
    
    if (energy > 0.02f) {
        // Simple autocorrelation-like behavior
        static std::mt19937 gen(123);
        std::uniform_real_distribution<float> pitchDist(100.0f, 500.0f);
        std::uniform_real_distribution<float> confidenceDist(0.6f, 0.9f);
        
        result.frequency = pitchDist(gen);
        result.confidence = confidenceDist(gen);
        result.voiceDetected = true;
    } else {
        result.frequency = 0.0f;
        result.confidence = 0.0f;
        result.voiceDetected = false;
    }
    
    result.timestamp = 0;
    return result;
}

void AutocorrelationAlgorithm::Reset() {
    std::fill(correlationBuffer_.begin(), correlationBuffer_.end(), 0.0f);
}

// FFT Algorithm Implementation (Stub)
FFTAlgorithm::FFTAlgorithm() : fftSize_(1024) {
    fftBuffer_.resize(fftSize_);
    window_.resize(fftSize_);
    
    // Create Hann window
    for (int i = 0; i < fftSize_; ++i) {
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize_ - 1)));
    }
}

PitchDetectionResult FFTAlgorithm::DetectPitch(const std::vector<float>& audioSamples, int sampleRate) {
    // Stub implementation
    PitchDetectionResult result;
    
    // Calculate RMS
    float rms = 0.0f;
    for (float sample : audioSamples) {
        rms += sample * sample;
    }
    rms = std::sqrt(rms / audioSamples.size());
    
    if (rms > 0.015f) {
        // Simulate FFT-based pitch detection
        static std::mt19937 gen(456);
        std::uniform_real_distribution<float> pitchDist(120.0f, 600.0f);
        std::uniform_real_distribution<float> confidenceDist(0.8f, 0.95f);
        
        result.frequency = pitchDist(gen);
        result.confidence = confidenceDist(gen);
        result.voiceDetected = true;
    } else {
        result.frequency = 0.0f;
        result.confidence = 0.0f;
        result.voiceDetected = false;
    }
    
    result.timestamp = 0;
    return result;
}

void FFTAlgorithm::Reset() {
    std::fill(fftBuffer_.begin(), fftBuffer_.end(), 0.0f);
}

// Main NoteDetector Implementation
NoteDetector::NoteDetector()
    : currentAlgorithmPtr_(nullptr)
    , currentAlgorithm_(Algorithm::YIN)
    , sampleRate_(44100)
    , bufferSize_(1024)
    , writeIndex_(0)
    , bufferFull_(false)
    , sensitivity_(0.7f)
    , minFrequency_(80.0f)
    , maxFrequency_(800.0f)
    , confidenceThreshold_(0.5f)
    , initialized_(false)
    , realTimeMode_(false)
    , voiceActivityThreshold_(0.01f)
    , energyBufferIndex_(0)
    , calibrating_(false)
{
    energyBuffer_.resize(10, 0.0f);
}

NoteDetector::~NoteDetector() {
    Shutdown();
}

bool NoteDetector::Initialize(int sampleRate, int bufferSize) {
    std::cout << "Initializing NoteDetector..." << std::endl;
    
    sampleRate_ = sampleRate;
    bufferSize_ = bufferSize;
    
    // Initialize audio buffers
    audioBuffer_.resize(bufferSize_ * 2); // Double buffer for overlap
    processBuffer_.resize(bufferSize_);
    
    // Create algorithm instances
    yinAlgorithm_ = std::make_unique<YinAlgorithm>();
    autocorrelationAlgorithm_ = std::make_unique<AutocorrelationAlgorithm>();
    fftAlgorithm_ = std::make_unique<FFTAlgorithm>();
    
    // Set initial algorithm
    SetAlgorithm(Algorithm::YIN);
    
    // Initialize detection state
    lastResult_ = {};
    detectionHistory_.clear();
    detectionHistory_.reserve(1000);
    
    initialized_ = true;
    std::cout << "NoteDetector initialized successfully" << std::endl;
    std::cout << "Sample Rate: " << sampleRate_ << " Hz" << std::endl;
    std::cout << "Buffer Size: " << bufferSize_ << " samples" << std::endl;
    std::cout << "Algorithm: " << currentAlgorithmPtr_->GetAlgorithmName() << std::endl;
    
    return true;
}

void NoteDetector::Shutdown() {
    if (!initialized_) return;
    
    yinAlgorithm_.reset();
    autocorrelationAlgorithm_.reset();
    fftAlgorithm_.reset();
    currentAlgorithmPtr_ = nullptr;
    
    audioBuffer_.clear();
    processBuffer_.clear();
    detectionHistory_.clear();
    
    initialized_ = false;
    std::cout << "NoteDetector shutdown complete" << std::endl;
}

void NoteDetector::SetAlgorithm(Algorithm algorithm) {
    currentAlgorithm_ = algorithm;
    
    switch (algorithm) {
        case Algorithm::YIN:
            currentAlgorithmPtr_ = yinAlgorithm_.get();
            break;
        case Algorithm::AUTOCORRELATION:
            currentAlgorithmPtr_ = autocorrelationAlgorithm_.get();
            break;
        case Algorithm::FFT:
            currentAlgorithmPtr_ = fftAlgorithm_.get();
            break;
        case Algorithm::HYBRID:
            // For now, default to YIN for hybrid mode
            currentAlgorithmPtr_ = yinAlgorithm_.get();
            break;
    }
    
    if (currentAlgorithmPtr_) {
        currentAlgorithmPtr_->Reset();
        std::cout << "Switched to algorithm: " << currentAlgorithmPtr_->GetAlgorithmName() << std::endl;
    }
}

std::vector<std::string> NoteDetector::GetAvailableAlgorithms() const {
    return {"YIN", "Autocorrelation", "FFT", "Hybrid"};
}

void NoteDetector::SetSensitivity(float sensitivity) {
    sensitivity_ = std::max(0.0f, std::min(1.0f, sensitivity));
    voiceActivityThreshold_ = 0.005f + (1.0f - sensitivity_) * 0.05f;
}

void NoteDetector::SetFrequencyRange(float minHz, float maxHz) {
    minFrequency_ = std::max(20.0f, minHz);
    maxFrequency_ = std::min(20000.0f, maxHz);
    
    if (minFrequency_ >= maxFrequency_) {
        maxFrequency_ = minFrequency_ + 100.0f;
    }
}

void NoteDetector::SetConfidenceThreshold(float threshold) {
    confidenceThreshold_ = std::max(0.0f, std::min(1.0f, threshold));
}

void NoteDetector::ProcessAudioBuffer(const float* samples, int numSamples) {
    if (!initialized_ || !samples) return;
    
    // Add samples to circular buffer
    for (int i = 0; i < numSamples; ++i) {
        audioBuffer_[writeIndex_] = samples[i];
        writeIndex_ = (writeIndex_ + 1) % audioBuffer_.size();
        
        if (writeIndex_ == 0) {
            bufferFull_ = true;
        }
    }
}

void NoteDetector::ProcessAudioBuffer(const std::vector<float>& samples) {
    ProcessAudioBuffer(samples.data(), static_cast<int>(samples.size()));
}

PitchDetectionResult NoteDetector::DetectPitch() {
    if (!initialized_ || !currentAlgorithmPtr_ || !bufferFull_) {
        return lastResult_;
    }
    
    // Copy audio data to process buffer
    int readIndex = writeIndex_;
    for (int i = 0; i < bufferSize_; ++i) {
        processBuffer_[i] = audioBuffer_[readIndex];
        readIndex = (readIndex + 1) % audioBuffer_.size();
    }
    
    // Preprocess audio
    RemoveDCOffset(processBuffer_);
    ApplyPreEmphasis(processBuffer_);
    ApplyWindowing(processBuffer_);
    
    // Calculate voice activity
    float voiceActivity = CalculateVoiceActivity(processBuffer_);
    
    // Detect pitch using current algorithm
    PitchDetectionResult rawResult = currentAlgorithmPtr_->DetectPitch(processBuffer_, sampleRate_);
    rawResult.timestamp = 0; // Will be set by caller if needed
    
    // Apply voice activity detection
    if (voiceActivity < voiceActivityThreshold_) {
        rawResult.voiceDetected = false;
        rawResult.confidence = 0.0f;
    }
    
    // Filter and smooth result
    PitchDetectionResult filteredResult = FilterResult(rawResult);
    ApplyTemporalSmoothing(filteredResult);
    
    // Update state
    lastResult_ = filteredResult;
    UpdateDetectionHistory(filteredResult);
    
    // Call callback if in real-time mode
    if (realTimeMode_ && detectionCallback_) {
        detectionCallback_(filteredResult);
    }
    
    // Store calibration data if calibrating
    if (calibrating_) {
        calibrationData_.push_back(filteredResult);
    }
    
    return filteredResult;
}

std::vector<PitchDetectionResult> NoteDetector::GetDetectionHistory(int maxResults) const {
    int count = std::min(maxResults, static_cast<int>(detectionHistory_.size()));
    if (count <= 0) return {};
    
    return std::vector<PitchDetectionResult>(
        detectionHistory_.end() - count,
        detectionHistory_.end()
    );
}

float NoteDetector::GetAverageConfidence(int windowSize) const {
    if (detectionHistory_.empty()) return 0.0f;
    
    int count = std::min(windowSize, static_cast<int>(detectionHistory_.size()));
    float sum = 0.0f;
    
    for (int i = detectionHistory_.size() - count; i < detectionHistory_.size(); ++i) {
        sum += detectionHistory_[i].confidence;
    }
    
    return sum / count;
}

bool NoteDetector::IsVoiceActive() const {
    return lastResult_.voiceDetected && lastResult_.confidence > confidenceThreshold_;
}

void NoteDetector::StartCalibration() {
    calibrating_ = true;
    calibrationData_.clear();
    std::cout << "Started pitch detection calibration" << std::endl;
}

void NoteDetector::StopCalibration() {
    calibrating_ = false;
    std::cout << "Stopped calibration. Collected " << calibrationData_.size() << " data points" << std::endl;
}

bool NoteDetector::SaveCalibrationData(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to save calibration data to: " << filepath << std::endl;
        return false;
    }
    
    file << "timestamp,frequency,confidence,voiceDetected\n";
    for (const auto& result : calibrationData_) {
        file << result.timestamp << "," 
             << result.frequency << "," 
             << result.confidence << "," 
             << (result.voiceDetected ? 1 : 0) << "\n";
    }
    
    file.close();
    std::cout << "Saved " << calibrationData_.size() << " calibration data points to: " << filepath << std::endl;
    return true;
}

bool NoteDetector::LoadCalibrationData(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to load calibration data from: " << filepath << std::endl;
        return false;
    }
    
    calibrationData_.clear();
    std::string line;
    std::getline(file, line); // Skip header
    
    while (std::getline(file, line)) {
        // Simple CSV parsing
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        size_t pos3 = line.find(',', pos2 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
            PitchDetectionResult result;
            result.timestamp = std::stoul(line.substr(0, pos1));
            result.frequency = std::stof(line.substr(pos1 + 1, pos2 - pos1 - 1));
            result.confidence = std::stof(line.substr(pos2 + 1, pos3 - pos2 - 1));
            result.voiceDetected = std::stoi(line.substr(pos3 + 1)) != 0;
            calibrationData_.push_back(result);
        }
    }
    
    file.close();
    std::cout << "Loaded " << calibrationData_.size() << " calibration data points from: " << filepath << std::endl;
    return true;
}

void NoteDetector::SetRealTimeMode(bool enabled) {
    realTimeMode_ = enabled;
}

void NoteDetector::SetCallback(std::function<void(const PitchDetectionResult&)> callback) {
    detectionCallback_ = callback;
}

// Private methods implementation
void NoteDetector::UpdateDetectionHistory(const PitchDetectionResult& result) {
    detectionHistory_.push_back(result);
    
    // Keep history size manageable
    if (detectionHistory_.size() > 1000) {
        detectionHistory_.erase(detectionHistory_.begin(), detectionHistory_.begin() + 500);
    }
}

bool NoteDetector::IsValidFrequency(float frequency) const {
    return frequency >= minFrequency_ && frequency <= maxFrequency_;
}

float NoteDetector::CalculateVoiceActivity(const std::vector<float>& samples) {
    float energy = 0.0f;
    for (float sample : samples) {
        energy += sample * sample;
    }
    energy = std::sqrt(energy / samples.size());
    
    // Update energy buffer for smoothing
    energyBuffer_[energyBufferIndex_] = energy;
    energyBufferIndex_ = (energyBufferIndex_ + 1) % energyBuffer_.size();
    
    // Return average energy
    float avgEnergy = 0.0f;
    for (float e : energyBuffer_) {
        avgEnergy += e;
    }
    return avgEnergy / energyBuffer_.size();
}

PitchDetectionResult NoteDetector::FilterResult(const PitchDetectionResult& rawResult) {
    PitchDetectionResult filtered = rawResult;
    
    // Frequency range validation
    if (!IsValidFrequency(filtered.frequency)) {
        filtered.frequency = 0.0f;
        filtered.confidence = 0.0f;
        filtered.voiceDetected = false;
    }
    
    // Confidence threshold
    if (filtered.confidence < confidenceThreshold_) {
        filtered.voiceDetected = false;
    }
    
    return filtered;
}

void NoteDetector::ApplyTemporalSmoothing(PitchDetectionResult& result) {
    if (detectionHistory_.empty()) return;
    
    const int smoothingWindow = 3;
    int count = std::min(smoothingWindow, static_cast<int>(detectionHistory_.size()));
    
    float avgFreq = result.frequency;
    float avgConf = result.confidence;
    
    for (int i = detectionHistory_.size() - count; i < detectionHistory_.size(); ++i) {
        avgFreq += detectionHistory_[i].frequency;
        avgConf += detectionHistory_[i].confidence;
    }
    
    result.frequency = avgFreq / (count + 1);
    result.confidence = avgConf / (count + 1);
}

void NoteDetector::ApplyPreEmphasis(std::vector<float>& samples) {
    const float alpha = 0.97f;
    for (int i = samples.size() - 1; i > 0; --i) {
        samples[i] = samples[i] - alpha * samples[i - 1];
    }
}

void NoteDetector::ApplyWindowing(std::vector<float>& samples) {
    // Apply Hann window
    int n = samples.size();
    for (int i = 0; i < n; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (n - 1)));
        samples[i] *= window;
    }
}

void NoteDetector::RemoveDCOffset(std::vector<float>& samples) {
    float mean = 0.0f;
    for (float sample : samples) {
        mean += sample;
    }
    mean /= samples.size();
    
    for (float& sample : samples) {
        sample -= mean;
    }
}

float NoteDetector::HzToMidi(float frequency) {
    return 69.0f + 12.0f * std::log2(frequency / 440.0f);
}

float NoteDetector::MidiToHz(int midiNote) {
    return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}

std::string NoteDetector::FrequencyToNoteName(float frequency) {
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    int midiNote = static_cast<int>(std::round(HzToMidi(frequency)));
    int octave = (midiNote / 12) - 1;
    int noteIndex = midiNote % 12;
    
    return std::string(noteNames[noteIndex]) + std::to_string(octave);
}

} // namespace Lyricstator