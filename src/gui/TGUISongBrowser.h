#pragma once
#include <TGUI/TGUI.hpp>
#include <vector>
#include <string>
#include <functional>

namespace Lyricstator {

struct SongInfo {
    std::string filepath;
    std::string filename;
    std::string title;
    std::string artist;
    std::string album;
    std::string format;
    uint64_t fileSize;
    std::string duration; // "mm:ss" format
    bool hasLyrics;
    bool hasMidi;
};

class TGUISongBrowser {
public:
    TGUISongBrowser();
    ~TGUISongBrowser();
    
    // Initialization
    bool Initialize(tgui::Gui* gui);
    void Shutdown();
    
    // Visibility control
    void Show();
    void Hide();
    void Toggle();
    bool IsVisible() const { return isVisible_; }
    
    // Song management
    void RefreshSongList();
    void SearchSongs(const std::string& query);
    void ClearSearch();
    
    // Directory management
    void AddDirectory(const std::string& path);
    void RemoveDirectory(const std::string& path);
    void ScanDirectories();
    
    // Callbacks
    void SetSongSelectedCallback(std::function<void(const SongInfo&)> callback) { songSelectedCallback_ = callback; }
    void SetDirectoryChangedCallback(std::function<void()> callback) { directoryChangedCallback_ = callback; }
    
    // Update
    void Update(float deltaTime);
    
    // Event handling
    void HandleKeyPress(const tgui::Event& event);
    
private:
    tgui::Gui* gui_;
    bool initialized_;
    bool isVisible_;
    
    // UI Components
    tgui::Panel::Ptr mainPanel_;
    tgui::Label::Ptr titleLabel_;
    tgui::EditBox::Ptr searchBox_;
    tgui::Button::Ptr refreshButton_;
    tgui::Button::Ptr addDirectoryButton_;
    tgui::Button::Ptr closeButton_;
    tgui::ListView::Ptr songListView_;
    tgui::ScrollablePanel::Ptr directoryPanel_;
    tgui::Label::Ptr statusLabel_;
    tgui::ProgressBar::Ptr scanProgressBar_;
    
    // Data
    std::vector<SongInfo> allSongs_;
    std::vector<SongInfo> filteredSongs_;
    std::vector<std::string> directories_;
    std::string currentSearchQuery_;
    
    // Callbacks
    std::function<void(const SongInfo&)> songSelectedCallback_;
    std::function<void()> directoryChangedCallback_;
    
    // Animation
    float slideAnimation_;
    bool isAnimating_;
    
    // Internal methods
    void CreateUI();
    void SetupStyling();
    void SetupCallbacks();
    void UpdateSongList();
    void PopulateSongList(const std::vector<SongInfo>& songs);
    void OnSongSelected(int index);
    void OnSearchTextChanged();
    void OnRefreshClicked();
    void OnAddDirectoryClicked();
    void OnCloseClicked();
    
    // File scanning
    void ScanDirectory(const std::string& path, bool recursive = true);
    SongInfo CreateSongInfo(const std::string& filepath);
    std::string ExtractTitle(const std::string& filepath);
    std::string ExtractArtist(const std::string& filepath);
    std::string FormatFileSize(uint64_t bytes);
    bool MatchesSearch(const SongInfo& song, const std::string& query);
    
    // Directory management UI
    void UpdateDirectoryList();
    void CreateDirectoryItem(const std::string& path);
    
    // Animation helpers
    void StartSlideAnimation(bool show);
    void UpdateAnimation(float deltaTime);
};

} // namespace Lyricstator
