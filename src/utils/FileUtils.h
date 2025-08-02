#pragma once
#include <string>
#include <vector>

namespace Lyricstator {

class FileUtils {
public:
    static bool FileExists(const std::string& filepath);
    static std::string GetFileExtension(const std::string& filepath);
    static std::string GetFileName(const std::string& filepath);
    static std::string GetDirectory(const std::string& filepath);
    static std::vector<std::string> ListFiles(const std::string& directory, const std::string& extension = "");
    static bool IsAudioFile(const std::string& filepath);
    static bool IsMidiFile(const std::string& filepath);
    static bool IsLyricFile(const std::string& filepath);
};

} // namespace Lyricstator