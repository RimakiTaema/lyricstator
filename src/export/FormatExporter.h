#pragma once
#include "common/Types.h"
#include <string>

namespace Lyricstator {

class FormatExporter {
public:
    FormatExporter();
    ~FormatExporter();
    
    bool Export(const std::string& filepath, ExportFormat format,
                const std::string& audioFile, const std::string& midiFile,
                const std::string& lyricScript);
    
private:
    bool ExportXMK(const std::string& filepath);
    bool ExportEXMK(const std::string& filepath);
    bool ExportLYSTEMK(const std::string& filepath);
    bool ExportMIDIWithLyrics(const std::string& filepath);
};

} // namespace Lyricstator
