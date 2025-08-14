#include "audio/MidiParser.h"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace Lyricstator {

MidiParser::MidiParser()
    : format_(0)
    , trackCount_(0)
    , ticksPerQuarterNote_(480)
    , currentTempo_(500000) // Default 120 BPM (500,000 microseconds per quarter note)
    , validFile_(false)
{
}

MidiParser::~MidiParser() {
    Clear();
}

bool MidiParser::LoadMidiFile(const std::string& filepath) {
    std::cout << "Parsing MIDI file: " << filepath << std::endl;
    
    Clear();
    
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open MIDI file: " << filepath << std::endl;
        return false;
    }
    
    // Parse MIDI header
    if (!ParseHeader(file)) {
        std::cerr << "Failed to parse MIDI header" << std::endl;
        file.close();
        return false;
    }
    
    std::cout << "MIDI Format: " << format_ << std::endl;
    std::cout << "Track Count: " << trackCount_ << std::endl;
    std::cout << "Ticks per Quarter: " << ticksPerQuarterNote_ << std::endl;
    
    // Parse all tracks
    for (uint16_t track = 0; track < trackCount_; ++track) {
        // Read track header
        char trackHeader[4];
        file.read(trackHeader, 4);
        
        if (std::strncmp(trackHeader, "MTrk", 4) != 0) {
            std::cerr << "Invalid track header at track " << track << std::endl;
            file.close();
            return false;
        }
        
        uint32_t trackLength = ReadUInt32BE(file);
        std::cout << "Parsing track " << track << " (length: " << trackLength << " bytes)" << std::endl;
        
        if (!ParseTrack(file, trackLength)) {
            std::cerr << "Failed to parse track " << track << std::endl;
            file.close();
            return false;
        }
    }
    
    file.close();
    
    // Sort events by time
    std::sort(notes_.begin(), notes_.end(), 
        [](const MidiNote& a, const MidiNote& b) {
            return a.startTime < b.startTime;
        });
    
    std::sort(lyricEvents_.begin(), lyricEvents_.end(),
        [](const LyricEvent& a, const LyricEvent& b) {
            return a.startTime < b.startTime;
        });
    
    validFile_ = true;
    
    std::cout << "MIDI parsing complete:" << std::endl;
    std::cout << "  Notes: " << notes_.size() << std::endl;
    std::cout << "  Lyrics: " << lyricEvents_.size() << std::endl;
    std::cout << "  Tempo events: " << tempoEvents_.size() << std::endl;
    std::cout << "  Time signatures: " << timeSignatures_.size() << std::endl;
    
    return true;
}

void MidiParser::Clear() {
    notes_.clear();
    tempoEvents_.clear();
    timeSignatures_.clear();
    lyricEvents_.clear();
    activeNotes_.clear();
    
    format_ = 0;
    trackCount_ = 0;
    ticksPerQuarterNote_ = 480;
    currentTempo_ = 500000;
    validFile_ = false;
}

bool MidiParser::ParseHeader(std::ifstream& file) {
    // Read "MThd" chunk type
    char chunkType[4];
    file.read(chunkType, 4);
    
    if (std::strncmp(chunkType, "MThd", 4) != 0) {
        std::cerr << "Invalid MIDI file header" << std::endl;
        return false;
    }
    
    // Read header length (should be 6)
    uint32_t headerLength = ReadUInt32BE(file);
    if (headerLength != 6) {
        std::cerr << "Invalid MIDI header length: " << headerLength << std::endl;
        return false;
    }
    
    // Read format, track count, and division
    format_ = ReadUInt16BE(file);
    trackCount_ = ReadUInt16BE(file);
    uint16_t division = ReadUInt16BE(file);
    
    // Check if division is in ticks per quarter note format
    if (division & 0x8000) {
        std::cerr << "SMPTE time division not supported" << std::endl;
        return false;
    }
    
    ticksPerQuarterNote_ = division;
    
    return true;
}

bool MidiParser::ParseTrack(std::ifstream& file, uint32_t trackLength) {
    uint32_t bytesRead = 0;
    uint32_t absoluteTime = 0;
    uint8_t runningStatus = 0;
    
    activeNotes_.clear();
    
    while (bytesRead < trackLength) {
        MidiEvent event;
        
        if (!ParseEvent(file, event, runningStatus)) {
            std::cerr << "Failed to parse MIDI event" << std::endl;
            return false;
        }
        
        absoluteTime += event.deltaTime;
        
        // Process different types of events
        if (event.status == 0xFF) {
            // Meta event
            ProcessMetaEvent(event, absoluteTime);
        } else if ((event.status & 0xF0) == 0x90 || (event.status & 0xF0) == 0x80) {
            // Note on/off event
            uint8_t channel = event.status & 0x0F;
            ProcessNoteEvent(event, absoluteTime, channel);
        }
        
        // Update bytes read counter
        bytesRead += 1; // Delta time (variable length, counted in ReadVariableLength)
        bytesRead += 1; // Status byte
        bytesRead += event.data.size();
    }
    
    // Process any remaining active notes
    for (const auto& activeNote : activeNotes_) {
        MidiNote note;
        note.note = activeNote.note;
        note.velocity = activeNote.velocity;
        note.startTime = activeNote.startTime;
        note.duration = absoluteTime - activeNote.startTime;
        note.channel = activeNote.channel;
        notes_.push_back(note);
    }
    
    return true;
}

bool MidiParser::ParseEvent(std::ifstream& file, MidiEvent& event, uint8_t& runningStatus) {
    // Read delta time
    event.deltaTime = ReadVariableLength(file);
    
    // Read status byte
    uint8_t statusByte;
    file.read(reinterpret_cast<char*>(&statusByte), 1);
    
    if (statusByte & 0x80) {
        // New status byte
        event.status = statusByte;
        runningStatus = statusByte;
    } else {
        // Running status - use previous status and treat this byte as data
        event.status = runningStatus;
        event.data.push_back(statusByte);
    }
    
    // Read event data based on status
    if (event.status == 0xFF) {
        // Meta event
        uint8_t metaType;
        file.read(reinterpret_cast<char*>(&metaType), 1);
        event.data.push_back(metaType);
        
        uint32_t length = ReadVariableLength(file);
        for (uint32_t i = 0; i < length; ++i) {
            uint8_t byte;
            file.read(reinterpret_cast<char*>(&byte), 1);
            event.data.push_back(byte);
        }
    } else if ((event.status & 0xF0) == 0x90 || (event.status & 0xF0) == 0x80) {
        // Note on/off - read note and velocity
        if (event.data.empty()) { // If not already read due to running status
            uint8_t note, velocity;
            file.read(reinterpret_cast<char*>(&note), 1);
            file.read(reinterpret_cast<char*>(&velocity), 1);
            event.data.push_back(note);
            event.data.push_back(velocity);
        } else {
            // Running status case - need to read velocity
            uint8_t velocity;
            file.read(reinterpret_cast<char*>(&velocity), 1);
            event.data.push_back(velocity);
        }
    } else if ((event.status & 0xF0) == 0xC0 || (event.status & 0xF0) == 0xD0) {
        // Program change or channel pressure - 1 data byte
        if (event.data.empty()) {
            uint8_t data;
            file.read(reinterpret_cast<char*>(&data), 1);
            event.data.push_back(data);
        }
    } else if ((event.status & 0xF0) >= 0x80 && (event.status & 0xF0) <= 0xE0) {
        // Other channel messages - 2 data bytes
        if (event.data.empty()) {
            uint8_t data1, data2;
            file.read(reinterpret_cast<char*>(&data1), 1);
            file.read(reinterpret_cast<char*>(&data2), 1);
            event.data.push_back(data1);
            event.data.push_back(data2);
        } else {
            // Running status case
            uint8_t data2;
            file.read(reinterpret_cast<char*>(&data2), 1);
            event.data.push_back(data2);
        }
    }
    
    return true;
}

void MidiParser::ProcessNoteEvent(const MidiEvent& event, uint32_t absoluteTime, uint8_t channel) {
    if (event.data.size() < 2) return;
    
    uint8_t note = event.data[0];
    uint8_t velocity = event.data[1];
    
    if ((event.status & 0xF0) == 0x90 && velocity > 0) {
        // Note on
        ActiveNote activeNote;
        activeNote.note = note;
        activeNote.velocity = velocity;
        activeNote.startTime = absoluteTime;
        activeNote.channel = channel;
        activeNotes_.push_back(activeNote);
    } else {
        // Note off (or note on with velocity 0)
        auto it = std::find_if(activeNotes_.begin(), activeNotes_.end(),
            [note, channel](const ActiveNote& activeNote) {
                return activeNote.note == note && activeNote.channel == channel;
            });
        
        if (it != activeNotes_.end()) {
            MidiNote midiNote;
            midiNote.note = it->note;
            midiNote.velocity = it->velocity;
            midiNote.startTime = it->startTime;
            midiNote.duration = absoluteTime - it->startTime;
            midiNote.channel = it->channel;
            notes_.push_back(midiNote);
            
            activeNotes_.erase(it);
        }
    }
}

void MidiParser::ProcessMetaEvent(const MidiEvent& event, uint32_t absoluteTime) {
    if (event.data.empty()) return;
    
    uint8_t metaType = event.data[0];
    
    switch (metaType) {
        case 0x05: // Lyric
        case 0x01: // Text
            if (event.data.size() > 1) {
                std::string lyricText(event.data.begin() + 1, event.data.end());
                ProcessLyricEvent(lyricText, absoluteTime);
            }
            break;
            
        case 0x51: // Tempo
            if (event.data.size() >= 4) {
                std::vector<uint8_t> tempoData(event.data.begin() + 1, event.data.end());
                ProcessTempoEvent(tempoData, absoluteTime);
            }
            break;
            
        case 0x58: // Time signature
            if (event.data.size() >= 5) {
                std::vector<uint8_t> timeSigData(event.data.begin() + 1, event.data.end());
                ProcessTimeSignatureEvent(timeSigData, absoluteTime);
            }
            break;
            
        default:
            // Other meta events - ignore for now
            break;
    }
}

void MidiParser::ProcessLyricEvent(const std::string& lyricText, uint32_t absoluteTime) {
    LyricEvent lyricEvent;
    lyricEvent.text = lyricText;
    lyricEvent.startTime = TicksToMilliseconds(absoluteTime);
    lyricEvent.endTime = lyricEvent.startTime + 1000; // Default 1 second duration
    lyricEvent.pitch = 0.0f; // Will be determined by synchronization
    lyricEvent.highlighted = false;
    
    lyricEvents_.push_back(lyricEvent);
}

void MidiParser::ProcessTempoEvent(const std::vector<uint8_t>& data, uint32_t absoluteTime) {
    if (data.size() < 3) return;
    
    uint32_t microsecondsPerQuarter = (data[0] << 16) | (data[1] << 8) | data[2];
    currentTempo_ = microsecondsPerQuarter;
    
    TempoEvent tempoEvent;
    tempoEvent.tick = absoluteTime;
    tempoEvent.microsecondsPerQuarter = microsecondsPerQuarter;
    tempoEvent.bpm = 60000000.0 / microsecondsPerQuarter;
    
    tempoEvents_.push_back(tempoEvent);
}

void MidiParser::ProcessTimeSignatureEvent(const std::vector<uint8_t>& data, uint32_t absoluteTime) {
    if (data.size() < 4) return;
    
    TimeSignature timeSig;
    timeSig.tick = absoluteTime;
    timeSig.numerator = data[0];
    timeSig.denominator = 1 << data[1]; // 2^data[1]
    
    timeSignatures_.push_back(timeSig);
}

uint32_t MidiParser::TicksToMilliseconds(uint32_t ticks) const {
    // Find the current tempo at this tick position
    uint32_t currentTempo = currentTempo_;
    for (const auto& tempoEvent : tempoEvents_) {
        if (tempoEvent.tick <= ticks) {
            currentTempo = tempoEvent.microsecondsPerQuarter;
        } else {
            break;
        }
    }
    
    // Convert ticks to milliseconds
    // ms = (ticks / ticksPerQuarter) * (microsecondsPerQuarter / 1000)
    return static_cast<uint32_t>(
        (static_cast<double>(ticks) / ticksPerQuarterNote_) * 
        (static_cast<double>(currentTempo) / 1000.0)
    );
}

uint32_t MidiParser::MillisecondsToTicks(uint32_t milliseconds) const {
    // Simplified conversion using current tempo
    return static_cast<uint32_t>(
        (static_cast<double>(milliseconds) * 1000.0 / currentTempo_) * 
        ticksPerQuarterNote_
    );
}

double MidiParser::GetCurrentBPM(uint32_t ticks) const {
    uint32_t currentTempo = currentTempo_;
    for (const auto& tempoEvent : tempoEvents_) {
        if (tempoEvent.tick <= ticks) {
            currentTempo = tempoEvent.microsecondsPerQuarter;
        } else {
            break;
        }
    }
    
    return 60000000.0 / currentTempo;
}

std::pair<uint8_t, uint8_t> MidiParser::GetNoteRange() const {
    if (notes_.empty()) {
        return {60, 60}; // Middle C as default
    }
    
    uint8_t minNote = 127, maxNote = 0;
    for (const auto& note : notes_) {
        minNote = std::min(minNote, note.note);
        maxNote = std::max(maxNote, note.note);
    }
    
    return {minNote, maxNote};
}

uint32_t MidiParser::GetDurationTicks() const {
    if (notes_.empty()) return 0;
    
    uint32_t maxTick = 0;
    for (const auto& note : notes_) {
        maxTick = std::max(maxTick, note.startTime + note.duration);
    }
    
    return maxTick;
}

uint32_t MidiParser::GetDurationMs() const {
    return TicksToMilliseconds(GetDurationTicks());
}

// Utility functions for reading binary data
uint16_t MidiParser::ReadUInt16BE(std::ifstream& file) {
    uint16_t value;
    file.read(reinterpret_cast<char*>(&value), 2);
    return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8); // Convert to big-endian
}

uint32_t MidiParser::ReadUInt32BE(std::ifstream& file) {
    uint32_t value;
    file.read(reinterpret_cast<char*>(&value), 4);
    return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | 
           ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

uint32_t MidiParser::ReadVariableLength(std::ifstream& file) {
    uint32_t value = 0;
    uint8_t byte;
    
    do {
        file.read(reinterpret_cast<char*>(&byte), 1);
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    
    return value;
}

std::string MidiParser::ReadString(std::ifstream& file, size_t length) {
    std::string result(length, '\0');
    file.read(&result[0], length);
    return result;
}

} // namespace Lyricstator
