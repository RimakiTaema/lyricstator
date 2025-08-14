#pragma once
#include "common/Types.h"
#include <vector>
#include <string>
#include <fstream>

namespace Lyricstator {

class MidiParser {
public:
    MidiParser();
    ~MidiParser();
    
    // File loading
    bool LoadMidiFile(const std::string& filepath);
    void Clear();
    
    // Data access
    const std::vector<MidiNote>& GetNotes() const { return notes_; }
    const std::vector<TempoEvent>& GetTempoEvents() const { return tempoEvents_; }
    const std::vector<TimeSignature>& GetTimeSignatures() const { return timeSignatures_; }
    const std::vector<LyricEvent>& GetLyricEvents() const { return lyricEvents_; }
    
    // MIDI properties
    uint16_t GetTicksPerQuarterNote() const { return ticksPerQuarterNote_; }
    uint16_t GetFormat() const { return format_; }
    uint16_t GetTrackCount() const { return trackCount_; }
    
    // Time conversion
    uint32_t TicksToMilliseconds(uint32_t ticks) const;
    uint32_t MillisecondsToTicks(uint32_t milliseconds) const;
    double GetCurrentBPM(uint32_t ticks) const;
    
    // Analysis
    std::pair<uint8_t, uint8_t> GetNoteRange() const; // min, max note
    uint32_t GetDurationTicks() const;
    uint32_t GetDurationMs() const;
    
private:
    // MIDI file data
    std::vector<MidiNote> notes_;
    std::vector<TempoEvent> tempoEvents_;
    std::vector<TimeSignature> timeSignatures_;
    std::vector<LyricEvent> lyricEvents_;
    
    // MIDI file properties
    uint16_t format_;              // MIDI format (0, 1, 2)
    uint16_t trackCount_;          // Number of tracks
    uint16_t ticksPerQuarterNote_; // Ticks per quarter note
    
    // File parsing
    struct MidiHeader {
        uint16_t format;
        uint16_t trackCount;
        uint16_t division;
    };
    
    struct MidiEvent {
        uint32_t deltaTime;
        uint8_t status;
        std::vector<uint8_t> data;
    };
    
    // Parsing methods
    bool ParseHeader(std::ifstream& file);
    bool ParseTrack(std::ifstream& file, uint32_t trackLength);
    bool ParseEvent(std::ifstream& file, MidiEvent& event, uint8_t& runningStatus);
    
    // Data reading utilities
    uint16_t ReadUInt16BE(std::ifstream& file);
    uint32_t ReadUInt32BE(std::ifstream& file);
    uint32_t ReadVariableLength(std::ifstream& file);
    std::string ReadString(std::ifstream& file, size_t length);
    
    // Event processing
    void ProcessNoteEvent(const MidiEvent& event, uint32_t absoluteTime, uint8_t channel);
    void ProcessMetaEvent(const MidiEvent& event, uint32_t absoluteTime);
    void ProcessLyricEvent(const std::string& lyricText, uint32_t absoluteTime);
    void ProcessTempoEvent(const std::vector<uint8_t>& data, uint32_t absoluteTime);
    void ProcessTimeSignatureEvent(const std::vector<uint8_t>& data, uint32_t absoluteTime);
    
    // Note tracking for note-off events
    struct ActiveNote {
        uint8_t note;
        uint8_t velocity;
        uint32_t startTime;
        uint8_t channel;
    };
    std::vector<ActiveNote> activeNotes_;
    
    // Current parsing state
    uint32_t currentTempo_;  // Microseconds per quarter note
    bool validFile_;
};

} // namespace Lyricstator
