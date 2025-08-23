#include "QtMidiParser.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

namespace Lyricstator {

QtMidiParser::QtMidiParser(QObject* parent)
    : QObject(parent)
    , lastError_()
{
    // Initialize default MIDI file
    midiFile_.format = 1;
    midiFile_.numTracks = 0;
    midiFile_.timeDivision = 480; // Default PPQN
    midiFile_.tempo = 120.0f;
    midiFile_.timeSignatureNumerator = 4;
    midiFile_.timeSignatureDenominator = 4;
    midiFile_.totalDuration = 0;
}

QtMidiParser::~QtMidiParser() {
}

bool QtMidiParser::loadMidiFile(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        lastError_ = QString("Failed to open MIDI file: %1").arg(filepath);
        emit parsingError(lastError_);
        return false;
    }
    
    try {
        // Clear previous data
        midiFile_.tracks.clear();
        midiFile_.filename = filepath;
        midiFile_.totalDuration = 0;
        
        // Parse MIDI header
        if (!parseMidiHeader(file)) {
            return false;
        }
        
        // Parse tracks
        for (int i = 0; i < midiFile_.numTracks; ++i) {
            MidiTrack track;
            if (!parseMidiTrack(file, track)) {
                qDebug() << "Failed to parse track" << i;
                continue;
            }
            midiFile_.tracks.append(track);
        }
        
        // Calculate total duration
        for (const auto& track : midiFile_.tracks) {
            for (const auto& note : track.notes) {
                uint32_t endTime = note.startTime + note.duration;
                if (endTime > midiFile_.totalDuration) {
                    midiFile_.totalDuration = endTime;
                }
            }
        }
        
        file.close();
        emit midiFileLoaded(filepath);
        qDebug() << "MIDI file loaded successfully:" << filepath;
        return true;
        
    } catch (const std::exception& e) {
        lastError_ = QString("Exception during MIDI parsing: %1").arg(e.what());
        emit parsingError(lastError_);
        file.close();
        return false;
    }
}

bool QtMidiParser::saveMidiFile(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        lastError_ = QString("Failed to create MIDI file: %1").arg(filepath);
        emit parsingError(lastError_);
        return false;
    }
    
    try {
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::BigEndian);
        
        // Write MIDI header
        stream.writeRawData("MThd", 4);
        stream << (uint32_t)6; // Header length
        stream << midiFile_.format;
        stream << (uint16_t)midiFile_.tracks.size();
        stream << midiFile_.timeDivision;
        
        // Write tracks
        for (const auto& track : midiFile_.tracks) {
            stream.writeRawData("MTrk", 4);
            
            // Calculate track length
            QByteArray trackData;
            QDataStream trackStream(&trackData, QIODevice::WriteOnly);
            trackStream.setByteOrder(QDataStream::BigEndian);
            
            uint32_t currentTime = 0;
            for (const auto& note : track.notes) {
                // Write delta time
                uint32_t deltaTime = note.startTime - currentTime;
                writeVariableLength(trackStream, deltaTime);
                currentTime = note.startTime;
                
                // Write note on
                trackStream << (uint8_t)(0x90 | note.channel);
                trackStream << note.note;
                trackStream << note.velocity;
                
                // Write note off
                writeVariableLength(trackStream, note.duration);
                trackStream << (uint8_t)(0x80 | note.channel);
                trackStream << note.note;
                trackStream << (uint8_t)0;
            }
            
            // Write track end
            trackStream << (uint8_t)0xFF;
            trackStream << (uint8_t)0x2F;
            trackStream << (uint8_t)0x00;
            
            stream << (uint32_t)trackData.size();
            stream.writeRawData(trackData.data(), trackData.size());
        }
        
        file.close();
        emit midiFileSaved(filepath);
        qDebug() << "MIDI file saved successfully:" << filepath;
        return true;
        
    } catch (const std::exception& e) {
        lastError_ = QString("Exception during MIDI saving: %1").arg(e.what());
        emit parsingError(lastError_);
        file.close();
        return false;
    }
}

const MidiTrack& QtMidiParser::getTrack(int index) const {
    static MidiTrack emptyTrack;
    if (index >= 0 && index < midiFile_.tracks.size()) {
        return midiFile_.tracks[index];
    }
    return emptyTrack;
}

MidiTrack& QtMidiParser::getTrack(int index) {
    static MidiTrack emptyTrack;
    if (index >= 0 && index < midiFile_.tracks.size()) {
        return midiFile_.tracks[index];
    }
    return emptyTrack;
}

QVector<MidiNote> QtMidiParser::getNotesInTimeRange(uint32_t startTime, uint32_t endTime) const {
    QVector<MidiNote> notes;
    
    for (const auto& track : midiFile_.tracks) {
        for (const auto& note : track.notes) {
            if (note.startTime >= startTime && note.startTime < endTime) {
                notes.append(note);
            }
        }
    }
    
    return notes;
}

QVector<MidiNote> QtMidiParser::getNotesForTrack(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < midiFile_.tracks.size()) {
        return midiFile_.tracks[trackIndex].notes;
    }
    return QVector<MidiNote>();
}

void QtMidiParser::setTempo(float tempo) {
    midiFile_.tempo = tempo;
    emit midiFileLoaded(midiFile_.filename); // Notify of change
}

bool QtMidiParser::exportToJson(const QString& filepath) const {
    QJsonObject root;
    root["filename"] = midiFile_.filename;
    root["format"] = midiFile_.format;
    root["numTracks"] = midiFile_.numTracks;
    root["timeDivision"] = midiFile_.timeDivision;
    root["tempo"] = midiFile_.tempo;
    root["totalDuration"] = midiFile_.totalDuration;
    
    QJsonArray tracksArray;
    for (const auto& track : midiFile_.tracks) {
        QJsonObject trackObj;
        trackObj["name"] = track.name;
        trackObj["isLyricTrack"] = track.isLyricTrack;
        trackObj["isMelodyTrack"] = track.isMelodyTrack;
        
        QJsonArray notesArray;
        for (const auto& note : track.notes) {
            QJsonObject noteObj;
            noteObj["channel"] = note.channel;
            noteObj["note"] = note.note;
            noteObj["velocity"] = note.velocity;
            noteObj["startTime"] = (int)note.startTime;
            noteObj["duration"] = (int)note.duration;
            noteObj["lyric"] = note.lyric;
            notesArray.append(noteObj);
        }
        trackObj["notes"] = notesArray;
        tracksArray.append(trackObj);
    }
    root["tracks"] = tracksArray;
    
    QJsonDocument doc(root);
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        return true;
    }
    
    return false;
}

bool QtMidiParser::exportToLystr(const QString& filepath) const {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    
    // Write header
    stream << "# Lyricstator Script File\n";
    stream << "# Generated from MIDI: " << midiFile_.filename << "\n";
    stream << "# Tempo: " << midiFile_.tempo << " BPM\n";
    stream << "# Time Signature: " << (int)midiFile_.timeSignatureNumerator 
           << "/" << (int)midiFile_.timeSignatureDenominator << "\n\n";
    
    // Write lyrics
    for (const auto& track : midiFile_.tracks) {
        if (track.isLyricTrack) {
            for (const auto& note : track.notes) {
                if (!note.lyric.isEmpty()) {
                    uint32_t timeMs = ticksToMs(note.startTime);
                    stream << QString("[%1.%2] %3\n")
                              .arg(timeMs / 1000)
                              .arg((timeMs % 1000) / 100, 3, 10, QChar('0'))
                              .arg(note.lyric);
                }
            }
        }
    }
    
    file.close();
    return true;
}

bool QtMidiParser::parseMidiHeader(QFile& file) {
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::BigEndian);
    
    // Check magic number
    char magic[4];
    stream.readRawData(magic, 4);
    if (strncmp(magic, "MThd", 4) != 0) {
        lastError_ = "Invalid MIDI file: Missing MThd header";
        emit parsingError(lastError_);
        return false;
    }
    
    // Read header length
    uint32_t headerLength;
    stream >> headerLength;
    if (headerLength != 6) {
        lastError_ = QString("Invalid MIDI header length: %1").arg(headerLength);
        emit parsingError(lastError_);
        return false;
    }
    
    // Read format, tracks, and time division
    stream >> midiFile_.format;
    stream >> midiFile_.numTracks;
    stream >> midiFile_.timeDivision;
    
    return true;
}

bool QtMidiParser::parseMidiTrack(QFile& file, MidiTrack& track) {
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::BigEndian);
    
    // Check track header
    char magic[4];
    stream.readRawData(magic, 4);
    if (strncmp(magic, "MTrk", 4) != 0) {
        lastError_ = "Invalid track: Missing MTrk header";
        emit parsingError(lastError_);
        return false;
    }
    
    // Read track length
    uint32_t trackLength;
    stream >> trackLength;
    
    // Parse track data
    uint32_t currentTime = 0;
    while (currentTime < trackLength) {
        if (!parseMidiEvent(stream, track, currentTime)) {
            break;
        }
    }
    
    return true;
}

bool QtMidiParser::parseMidiEvent(QDataStream& stream, MidiTrack& track, uint32_t& currentTime) {
    // Read delta time
    uint32_t deltaTime = readVariableLength(stream);
    currentTime += deltaTime;
    
    // Read event type
    uint8_t eventType;
    stream >> eventType;
    
    if (eventType == 0xFF) {
        // Meta event
        uint8_t metaType;
        stream >> metaType;
        
        if (metaType == 0x2F) {
            // End of track
            return false;
        } else if (metaType == 0x51) {
            // Tempo
            uint8_t tempoData[3];
            stream.readRawData((char*)tempoData, 3);
            uint32_t tempo = (tempoData[0] << 16) | (tempoData[1] << 8) | tempoData[2];
            midiFile_.tempo = 60000000.0f / tempo;
        }
    } else if ((eventType & 0xF0) == 0x90) {
        // Note on
        uint8_t note, velocity;
        stream >> note >> velocity;
        
        if (velocity > 0) {
            MidiNote midiNote;
            midiNote.channel = eventType & 0x0F;
            midiNote.note = note;
            midiNote.velocity = velocity;
            midiNote.startTime = currentTime;
            midiNote.duration = 0; // Will be set when note off is found
            midiNote.lyric = midiNoteToLyric(note);
            
            track.notes.append(midiNote);
        }
    } else if ((eventType & 0xF0) == 0x80) {
        // Note off
        uint8_t note, velocity;
        stream >> note >> velocity;
        
        // Find corresponding note on and set duration
        for (int i = track.notes.size() - 1; i >= 0; --i) {
            if (track.notes[i].note == note && track.notes[i].duration == 0) {
                track.notes[i].duration = currentTime - track.notes[i].startTime;
                break;
            }
        }
    }
    
    return true;
}

uint32_t QtMidiParser::readVariableLength(QDataStream& stream) {
    uint32_t value = 0;
    uint8_t byte;
    
    do {
        stream >> byte;
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    
    return value;
}

void QtMidiParser::writeVariableLength(QDataStream& stream, uint32_t value) {
    if (value < 0x80) {
        stream << (uint8_t)value;
    } else if (value < 0x4000) {
        stream << (uint8_t)(0x80 | (value >> 7));
        stream << (uint8_t)(value & 0x7F);
    } else if (value < 0x200000) {
        stream << (uint8_t)(0x80 | (value >> 14));
        stream << (uint8_t)(0x80 | ((value >> 7) & 0x7F));
        stream << (uint8_t)(value & 0x7F);
    } else {
        stream << (uint8_t)(0x80 | (value >> 21));
        stream << (uint8_t)(0x80 | ((value >> 14) & 0x7F));
        stream << (uint8_t)(0x80 | ((value >> 7) & 0x7F));
        stream << (uint8_t)(value & 0x7F);
    }
}

QString QtMidiParser::midiNoteToLyric(uint8_t note) {
    // Simple mapping - can be enhanced
    static const QString notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (note / 12) - 1;
    int noteIndex = note % 12;
    
    return QString("%1%2").arg(notes[noteIndex]).arg(octave);
}

uint8_t QtMidiParser::lyricToMidiNote(const QString& lyric) {
    // Simple reverse mapping - can be enhanced
    // This is a basic implementation
    return 60; // Middle C
}

uint32_t QtMidiParser::ticksToMs(uint32_t ticks) const {
    if (midiFile_.timeDivision > 0) {
        return (ticks * 60000) / (midiFile_.timeDivision * midiFile_.tempo);
    }
    return 0;
}

uint32_t QtMidiParser::msToTicks(uint32_t ms) const {
    if (midiFile_.timeDivision > 0) {
        return (ms * midiFile_.timeDivision * midiFile_.tempo) / 60000;
    }
    return 0;
}

} // namespace Lyricstator