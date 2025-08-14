#include "utils/FileUtils.h"
#include <filesystem>
#include <algorithm>

namespace Lyricstator {

bool FileUtils::FileExists(const std::string& filepath) {
    return std::filesystem::exists(filepath);
}

std::string FileUtils::GetFileExtension(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    
    std::string ext = filepath.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string FileUtils::GetFileName(const std::string& filepath) {
    size_t slashPos = filepath.find_last_of("/\\");
    if (slashPos == std::string::npos) return filepath;
    return filepath.substr(slashPos + 1);
}

std::string FileUtils::GetDirectory(const std::string& filepath) {
    size_t slashPos = filepath.find_last_of("/\\");
    if (slashPos == std::string::npos) return ".";
    return filepath.substr(0, slashPos);
}

std::vector<std::string> FileUtils::ListFiles(const std::string& directory, const std::string& extension) {
    std::vector<std::string> files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                if (extension.empty() || GetFileExtension(filepath) == extension) {
                    files.push_back(filepath);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Directory doesn't exist or can't be accessed
    }
    
    return files;
}

bool FileUtils::IsAudioFile(const std::string& filepath) {
    std::string ext = GetFileExtension(filepath);
    return (ext == "wav" || ext == "mp3" || ext == "ogg" || ext == "flac");
}

bool FileUtils::IsMidiFile(const std::string& filepath) {
    std::string ext = GetFileExtension(filepath);
    return (ext == "mid" || ext == "midi");
}

bool FileUtils::IsLyricFile(const std::string& filepath) {
    std::string ext = GetFileExtension(filepath);
    return (ext == "lystr" || ext == "txt");
}

} // namespace Lyricstator
