#include "TGUISongBrowser.h"
#include "core/SettingsManager.h"
#include "utils/FileUtils.h"
#include "utils/ErrorHandler.h"
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <regex>

namespace Lyricstator {

TGUISongBrowser::TGUISongBrowser()
    : gui_(nullptr)
    , initialized_(false)
    , isVisible_(false)
    , slideAnimation_(0.0f)
    , isAnimating_(false)
{
}

TGUISongBrowser::~TGUISongBrowser() {
    Shutdown();
}

bool TGUISongBrowser::Initialize(tgui::Gui* gui) {
    if (initialized_ || !gui) {
        return false;
    }
    
    gui_ = gui;
    
    CreateUI();
    SetupStyling();
    SetupCallbacks();
    
    // Load directories from settings
    auto& settings = SettingsManager::getInstance();
    directories_ = settings.getDirectorySettings().songDirectories;
    UpdateDirectoryList();
    
    initialized_ = true;
    ErrorHandler::getInstance().logInfo("TGUISongBrowser initialized successfully");
    return true;
}

void TGUISongBrowser::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    Hide();
    
    if (gui_ && mainPanel_) {
        gui_->remove(mainPanel_);
    }
    
    initialized_ = false;
}

void TGUISongBrowser::CreateUI() {
    // Main panel - covers most of the screen
    mainPanel_ = tgui::Panel::create();
    mainPanel_->setSize("80%", "85%");
    mainPanel_->setPosition("10%", "7.5%");
    mainPanel_->getRenderer()->setBackgroundColor(tgui::Color(25, 25, 35, 240));
    mainPanel_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    mainPanel_->getRenderer()->setBorders(2);
    mainPanel_->setVisible(false);
    
    // Title bar
    titleLabel_ = tgui::Label::create("Song Browser");
    titleLabel_->setSize("60%", "8%");
    titleLabel_->setPosition("2%", "2%");
    titleLabel_->getRenderer()->setTextColor(tgui::Color::White);
    titleLabel_->setTextSize(24);
    mainPanel_->add(titleLabel_);
    
    // Close button
    closeButton_ = tgui::Button::create("×");
    closeButton_->setSize("6%", "8%");
    closeButton_->setPosition("92%", "2%");
    closeButton_->getRenderer()->setBackgroundColor(tgui::Color(200, 50, 50));
    closeButton_->getRenderer()->setBackgroundColorHover(tgui::Color(255, 70, 70));
    closeButton_->getRenderer()->setTextColor(tgui::Color::White);
    closeButton_->setTextSize(20);
    mainPanel_->add(closeButton_);
    
    // Search box
    searchBox_ = tgui::EditBox::create();
    searchBox_->setSize("60%", "6%");
    searchBox_->setPosition("2%", "12%");
    searchBox_->setDefaultText("Search songs...");
    searchBox_->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 50));
    searchBox_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    searchBox_->getRenderer()->setTextColor(tgui::Color::White);
    searchBox_->setTextSize(16);
    mainPanel_->add(searchBox_);
    
    // Refresh button
    refreshButton_ = tgui::Button::create("Refresh");
    refreshButton_->setSize("15%", "6%");
    refreshButton_->setPosition("64%", "12%");
    refreshButton_->getRenderer()->setBackgroundColor(tgui::Color(50, 150, 50));
    refreshButton_->getRenderer()->setBackgroundColorHover(tgui::Color(70, 180, 70));
    refreshButton_->getRenderer()->setTextColor(tgui::Color::White);
    refreshButton_->setTextSize(14);
    mainPanel_->add(refreshButton_);
    
    // Add directory button
    addDirectoryButton_ = tgui::Button::create("Add Folder");
    addDirectoryButton_->setSize("15%", "6%");
    addDirectoryButton_->setPosition("81%", "12%");
    addDirectoryButton_->getRenderer()->setBackgroundColor(tgui::Color(100, 100, 200));
    addDirectoryButton_->getRenderer()->setBackgroundColorHover(tgui::Color(120, 120, 220));
    addDirectoryButton_->getRenderer()->setTextColor(tgui::Color::White);
    addDirectoryButton_->setTextSize(14);
    mainPanel_->add(addDirectoryButton_);
    
    // Song list view
    songListView_ = tgui::ListView::create();
    songListView_->setSize("96%", "65%");
    songListView_->setPosition("2%", "20%");
    songListView_->addColumn("Title", 300);
    songListView_->addColumn("Artist", 200);
    songListView_->addColumn("Format", 80);
    songListView_->addColumn("Size", 80);
    songListView_->addColumn("Path", 400);
    songListView_->getRenderer()->setBackgroundColor(tgui::Color(30, 30, 40));
    songListView_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    songListView_->getRenderer()->setTextColor(tgui::Color::White);
    songListView_->getRenderer()->setHeaderBackgroundColor(tgui::Color(50, 50, 70));
    songListView_->getRenderer()->setHeaderTextColor(tgui::Color::White);
    songListView_->getRenderer()->setSelectedBackgroundColor(tgui::Color(100, 150, 255, 100));
    songListView_->setTextSize(14);
    mainPanel_->add(songListView_);
    
    // Directory panel (scrollable)
    directoryPanel_ = tgui::ScrollablePanel::create();
    directoryPanel_->setSize("96%", "10%");
    directoryPanel_->setPosition("2%", "87%");
    directoryPanel_->getRenderer()->setBackgroundColor(tgui::Color(35, 35, 45));
    directoryPanel_->getRenderer()->setBorderColor(tgui::Color(100, 150, 255));
    directoryPanel_->getRenderer()->setBorders(1);
    mainPanel_->add(directoryPanel_);
    
    // Status label
    statusLabel_ = tgui::Label::create("Ready");
    statusLabel_->setSize("50%", "3%");
    statusLabel_->setPosition("2%", "97%");
    statusLabel_->getRenderer()->setTextColor(tgui::Color(200, 200, 200));
    statusLabel_->setTextSize(12);
    mainPanel_->add(statusLabel_);
    
    // Progress bar (initially hidden)
    scanProgressBar_ = tgui::ProgressBar::create();
    scanProgressBar_->setSize("46%", "3%");
    scanProgressBar_->setPosition("52%", "97%");
    scanProgressBar_->setMinimum(0);
    scanProgressBar_->setMaximum(100);
    scanProgressBar_->setValue(0);
    scanProgressBar_->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 50));
    scanProgressBar_->getRenderer()->setFillColor(tgui::Color(100, 200, 100));
    scanProgressBar_->setVisible(false);
    mainPanel_->add(scanProgressBar_);
    
    gui_->add(mainPanel_);
}

void TGUISongBrowser::SetupStyling() {
    // Apply consistent styling with rounded corners and shadows
    auto applyModernStyle = [](tgui::Widget::Ptr widget) {
        if (auto button = std::dynamic_pointer_cast<tgui::Button>(widget)) {
            button->getRenderer()->setRoundedBorderRadius(8);
        } else if (auto editBox = std::dynamic_pointer_cast<tgui::EditBox>(widget)) {
            editBox->getRenderer()->setRoundedBorderRadius(6);
        } else if (auto panel = std::dynamic_pointer_cast<tgui::Panel>(widget)) {
            panel->getRenderer()->setRoundedBorderRadius(12);
        }
    };
    
    applyModernStyle(mainPanel_);
    applyModernStyle(searchBox_);
    applyModernStyle(refreshButton_);
    applyModernStyle(addDirectoryButton_);
    applyModernStyle(closeButton_);
}

void TGUISongBrowser::SetupCallbacks() {
    // Search box text changed
    searchBox_->onTextChange([this]() {
        OnSearchTextChanged();
    });
    
    // Buttons
    refreshButton_->onPress([this]() {
        OnRefreshClicked();
    });
    
    addDirectoryButton_->onPress([this]() {
        OnAddDirectoryClicked();
    });
    
    closeButton_->onPress([this]() {
        OnCloseClicked();
    });
    
    // Song list selection
    songListView_->onItemSelect([this](int index) {
        OnSongSelected(index);
    });
    
    // Double-click to load song
    songListView_->onDoubleClick([this](int index) {
        OnSongSelected(index);
        if (songSelectedCallback_ && index >= 0 && index < static_cast<int>(filteredSongs_.size())) {
            songSelectedCallback_(filteredSongs_[index]);
        }
    });
}

void TGUISongBrowser::Show() {
    if (!initialized_ || isVisible_) {
        return;
    }
    
    isVisible_ = true;
    StartSlideAnimation(true);
    
    // Refresh song list when showing
    RefreshSongList();
}

void TGUISongBrowser::Hide() {
    if (!initialized_ || !isVisible_) {
        return;
    }
    
    isVisible_ = false;
    StartSlideAnimation(false);
}

void TGUISongBrowser::Toggle() {
    if (isVisible_) {
        Hide();
    } else {
        Show();
    }
}

void TGUISongBrowser::RefreshSongList() {
    if (!initialized_) {
        return;
    }
    
    statusLabel_->setText("Scanning directories...");
    scanProgressBar_->setVisible(true);
    scanProgressBar_->setValue(0);
    
    allSongs_.clear();
    ScanDirectories();
    
    // Apply current search filter
    if (currentSearchQuery_.empty()) {
        filteredSongs_ = allSongs_;
    } else {
        SearchSongs(currentSearchQuery_);
    }
    
    UpdateSongList();
    
    statusLabel_->setText("Found " + std::to_string(filteredSongs_.size()) + " songs");
    scanProgressBar_->setVisible(false);
}

void TGUISongBrowser::ScanDirectories() {
    auto& settings = SettingsManager::getInstance();
    const auto& dirSettings = settings.getDirectorySettings();
    
    int totalDirs = directories_.size();
    int currentDir = 0;
    
    for (const auto& directory : directories_) {
        if (std::filesystem::exists(directory)) {
            ScanDirectory(directory, dirSettings.recursiveSearch);
        }
        
        currentDir++;
        float progress = (float)currentDir / totalDirs * 100.0f;
        scanProgressBar_->setValue(static_cast<unsigned int>(progress));
    }
}

void TGUISongBrowser::ScanDirectory(const std::string& path, bool recursive) {
    try {
        auto& settings = SettingsManager::getInstance();
        const auto& supportedFormats = settings.getDirectorySettings().supportedFormats;
        
        auto iterator = recursive ? 
            std::filesystem::recursive_directory_iterator(path) :
            std::filesystem::directory_iterator(path);
        
        for (const auto& entry : iterator) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                std::string extension = FileUtils::GetFileExtension(filepath);
                
                // Check if file format is supported
                if (std::find(supportedFormats.begin(), supportedFormats.end(), extension) != supportedFormats.end()) {
                    SongInfo song = CreateSongInfo(filepath);
                    allSongs_.push_back(song);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        ErrorHandler::getInstance().logError("Failed to scan directory: " + path + " - " + e.what());
    }
}

SongInfo TGUISongBrowser::CreateSongInfo(const std::string& filepath) {
    SongInfo song;
    song.filepath = filepath;
    song.filename = FileUtils::GetFileName(filepath);
    song.format = FileUtils::GetFileExtension(filepath);
    
    // Extract title and artist from filename (basic implementation)
    song.title = ExtractTitle(filepath);
    song.artist = ExtractArtist(filepath);
    song.album = ""; // Could be extracted from metadata
    
    // Get file size
    try {
        song.fileSize = std::filesystem::file_size(filepath);
    } catch (...) {
        song.fileSize = 0;
    }
    
    // Check for associated files
    std::string basePath = filepath.substr(0, filepath.find_last_of('.'));
    song.hasLyrics = FileUtils::FileExists(basePath + ".lystr") || FileUtils::FileExists(basePath + ".txt");
    song.hasMidi = FileUtils::IsMidiFile(filepath) || FileUtils::FileExists(basePath + ".mid") || FileUtils::FileExists(basePath + ".midi");
    
    song.duration = ""; // Could be extracted from audio metadata
    
    return song;
}

std::string TGUISongBrowser::ExtractTitle(const std::string& filepath) {
    std::string filename = FileUtils::GetFileName(filepath);
    
    // Remove extension
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        filename = filename.substr(0, dotPos);
    }
    
    // Try to parse "Artist - Title" format
    size_t dashPos = filename.find(" - ");
    if (dashPos != std::string::npos) {
        return filename.substr(dashPos + 3);
    }
    
    return filename;
}

std::string TGUISongBrowser::ExtractArtist(const std::string& filepath) {
    std::string filename = FileUtils::GetFileName(filepath);
    
    // Remove extension
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        filename = filename.substr(0, dotPos);
    }
    
    // Try to parse "Artist - Title" format
    size_t dashPos = filename.find(" - ");
    if (dashPos != std::string::npos) {
        return filename.substr(0, dashPos);
    }
    
    return "Unknown Artist";
}

std::string TGUISongBrowser::FormatFileSize(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream oss;
    oss.precision(1);
    oss << std::fixed << size << " " << units[unitIndex];
    return oss.str();
}

void TGUISongBrowser::SearchSongs(const std::string& query) {
    currentSearchQuery_ = query;
    filteredSongs_.clear();
    
    if (query.empty()) {
        filteredSongs_ = allSongs_;
    } else {
        for (const auto& song : allSongs_) {
            if (MatchesSearch(song, query)) {
                filteredSongs_.push_back(song);
            }
        }
    }
    
    UpdateSongList();
    statusLabel_->setText("Found " + std::to_string(filteredSongs_.size()) + " songs matching '" + query + "'");
}

bool TGUISongBrowser::MatchesSearch(const SongInfo& song, const std::string& query) {
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    auto toLower = [](const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    };
    
    return toLower(song.title).find(lowerQuery) != std::string::npos ||
           toLower(song.artist).find(lowerQuery) != std::string::npos ||
           toLower(song.filename).find(lowerQuery) != std::string::npos ||
           toLower(song.format).find(lowerQuery) != std::string::npos;
}

void TGUISongBrowser::UpdateSongList() {
    songListView_->removeAllItems();
    
    for (const auto& song : filteredSongs_) {
        std::vector<tgui::String> columns = {
            song.title,
            song.artist,
            song.format,
            FormatFileSize(song.fileSize),
            song.filepath
        };
        songListView_->addItem(columns);
    }
}

void TGUISongBrowser::OnSongSelected(int index) {
    if (index >= 0 && index < static_cast<int>(filteredSongs_.size())) {
        const auto& song = filteredSongs_[index];
        statusLabel_->setText("Selected: " + song.title + " by " + song.artist);
    }
}

void TGUISongBrowser::OnSearchTextChanged() {
    std::string query = searchBox_->getText().toStdString();
    SearchSongs(query);
}

void TGUISongBrowser::OnRefreshClicked() {
    RefreshSongList();
}

void TGUISongBrowser::OnAddDirectoryClicked() {
    // This would typically open a file dialog
    // For now, we'll add a placeholder implementation
    ErrorHandler::getInstance().logInfo("Add directory functionality would open file dialog");
    statusLabel_->setText("Add directory: File dialog not implemented yet");
}

void TGUISongBrowser::OnCloseClicked() {
    Hide();
}

void TGUISongBrowser::StartSlideAnimation(bool show) {
    isAnimating_ = true;
    slideAnimation_ = show ? 0.0f : 1.0f;
    
    if (show) {
        mainPanel_->setVisible(true);
    }
}

void TGUISongBrowser::Update(float deltaTime) {
    if (!initialized_) {
        return;
    }
    
    UpdateAnimation(deltaTime);
}

void TGUISongBrowser::UpdateAnimation(float deltaTime) {
    if (!isAnimating_) {
        return;
    }
    
    const float animationSpeed = 4.0f;
    
    if (isVisible_) {
        slideAnimation_ += deltaTime * animationSpeed;
        if (slideAnimation_ >= 1.0f) {
            slideAnimation_ = 1.0f;
            isAnimating_ = false;
        }
    } else {
        slideAnimation_ -= deltaTime * animationSpeed;
        if (slideAnimation_ <= 0.0f) {
            slideAnimation_ = 0.0f;
            isAnimating_ = false;
            mainPanel_->setVisible(false);
        }
    }
    
    // Apply smooth easing animation
    float easedProgress = slideAnimation_ * slideAnimation_ * (3.0f - 2.0f * slideAnimation_);
    
    // Slide in from the left
    float targetX = 10.0f;
    float startX = -80.0f;
    float currentX = startX + (targetX - startX) * easedProgress;
    
    mainPanel_->setPosition(tgui::String(std::to_string(currentX) + "%"), "7.5%");
    
    // Fade in
    tgui::Color bgColor = mainPanel_->getRenderer()->getBackgroundColor();
    bgColor.setAlpha(static_cast<tgui::Uint8>(240 * easedProgress));
    mainPanel_->getRenderer()->setBackgroundColor(bgColor);
}

void TGUISongBrowser::UpdateDirectoryList() {
    directoryPanel_->removeAllWidgets();
    
    float yPos = 5.0f;
    for (const auto& directory : directories_) {
        CreateDirectoryItem(directory);
    }
}

void TGUISongBrowser::CreateDirectoryItem(const std::string& path) {
    auto dirLabel = tgui::Label::create(path);
    dirLabel->setSize("90%", "25%");
    dirLabel->setPosition("5%", std::to_string(directories_.size() * 30) + "px");
    dirLabel->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
    dirLabel->setTextSize(12);
    directoryPanel_->add(dirLabel);
}

void TGUISongBrowser::AddDirectory(const std::string& path) {
    if (std::find(directories_.begin(), directories_.end(), path) == directories_.end()) {
        directories_.push_back(path);
        UpdateDirectoryList();
        
        // Update settings
        auto& settings = SettingsManager::getInstance();
        settings.addSongDirectory(path);
        
        if (directoryChangedCallback_) {
            directoryChangedCallback_();
        }
    }
}

void TGUISongBrowser::RemoveDirectory(const std::string& path) {
    directories_.erase(std::remove(directories_.begin(), directories_.end(), path), directories_.end());
    UpdateDirectoryList();
    
    // Update settings
    auto& settings = SettingsManager::getInstance();
    settings.removeSongDirectory(path);
    
    if (directoryChangedCallback_) {
        directoryChangedCallback_();
    }
}

void TGUISongBrowser::ClearSearch() {
    searchBox_->setText("");
    currentSearchQuery_.clear();
    filteredSongs_ = allSongs_;
    UpdateSongList();
    statusLabel_->setText("Showing all " + std::to_string(allSongs_.size()) + " songs");
}

void TGUISongBrowser::HandleKeyPress(const tgui::Event& event) {
    if (event.type == tgui::Event::Type::KeyPressed) {
        if (event.key.code == tgui::Event::KeyboardKey::Escape) {
            Hide();
        } else if (event.key.code == tgui::Event::KeyboardKey::F5) {
            RefreshSongList();
        } else if (event.key.code == tgui::Event::KeyboardKey::F && event.key.control) {
            searchBox_->setFocused(true);
        }
    }
}

} // namespace Lyricstator
