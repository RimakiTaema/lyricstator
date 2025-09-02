#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>

namespace Lyricstator {

struct MidiNote {
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
    uint32_t startTime;
    uint32_t duration;
    QString lyric;
};

struct MidiTrack {
    QString name;
    QVector<MidiNote> notes;
    bool isLyricTrack;
    bool isMelodyTrack;
};

struct MidiFile {
    QString filename;
    uint16_t format;
    uint16_t numTracks;
    uint16_t timeDivision;
    QVector<MidiTrack> tracks;
    uint32_t totalDuration;
    
    // Tempo and time signature info
    float tempo;
    uint8_t timeSignatureNumerator;
    uint8_t timeSignatureDenominator;
};

class QtMidiParser : public QObject {
    Q_OBJECT

public:
    QtMidiParser(QObject* parent = nullptr);
    ~QtMidiParser();

    // File parsing
    bool loadMidiFile(const QString& filepath);
    bool saveMidiFile(const QString& filepath);
    
    // Data access
    const MidiFile& getMidiFile() const { return midiFile_; }
    MidiFile& getMidiFile() { return midiFile_; }
    
    // Track management
    int getTrackCount() const { return midiFile_.tracks.size(); }
    const MidiTrack& getTrack(int index) const;
    MidiTrack& getTrack(int index);
    
    // Note management
    QVector<MidiNote> getNotesInTimeRange(uint32_t startTime, uint32_t endTime) const;
    QVector<MidiNote> getNotesForTrack(int trackIndex) const;
    
    // Analysis
    float getTempo() const { return midiFile_.tempo; }
    void setTempo(float tempo);
    uint32_t getTotalDuration() const { return midiFile_.totalDuration; }
    
    // Export
    bool exportToJson(const QString& filepath) const;
    bool exportToLystr(const QString& filepath) const;
    
    // Validation
    bool isValid() const { return !midiFile_.filename.isEmpty(); }
    QString getLastError() const { return lastError_; }

signals:
    void midiFileLoaded(const QString& filename);
    void midiFileSaved(const QString& filename);
    void parsingError(const QString& error);

private:
    MidiFile midiFile_;
    QString lastError_;
    
    // MIDI parsing helpers
    bool parseMidiHeader(QFile& file);
    bool parseMidiTrack(QFile& file, MidiTrack& track);
    bool parseMidiEvent(QDataStream& stream, MidiTrack& track, uint32_t& currentTime);
    
    // Utility methods
    uint32_t readVariableLength(QDataStream& stream);
    void writeVariableLength(QDataStream& stream, uint32_t value);
    QString midiNoteToLyric(uint8_t note);
    uint8_t lyricToMidiNote(const QString& lyric);
    
    // Time conversion
    uint32_t ticksToMs(uint32_t ticks) const;
    uint32_t msToTicks(uint32_t ms) const;
};

} // namespace Lyricstator