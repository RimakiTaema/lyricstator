#include "QtKeybindEditor.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStyleOption>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QSettings>
#include <QLineEdit>
#include <QHeaderView>
#include <QFocusEvent>
#include <QKeyEvent>
#include <cmath>
#include <algorithm>

// Static constants
const QStringList QtKeybindEditor::DEFAULT_CATEGORIES = {
    "Playback", "Navigation", "Interface", "Audio", "Tools", "System"
};

const QMap<QString, QString> QtKeybindEditor::DEFAULT_ACTIONS = {
    {"play_pause", "Play/Pause"},
    {"stop", "Stop"},
    {"next_track", "Next Track"},
    {"previous_track", "Previous Track"},
    {"seek_forward", "Seek Forward"},
    {"seek_backward", "Seek Backward"},
    {"volume_up", "Volume Up"},
    {"volume_down", "Volume Down"},
    {"mute", "Mute/Unmute"},
    {"open_file", "Open File"},
    {"open_midi", "Open MIDI"},
    {"open_audio", "Open Audio"},
    {"open_lyrics", "Open Lyrics"},
    {"resource_pack", "Resource Pack GUI"},
    {"equalizer", "Equalizer"},
    {"keybind_editor", "Keybind Editor"},
    {"song_browser", "Song Browser"},
    {"fullscreen", "Toggle Fullscreen"},
    {"zoom_in", "Zoom In"},
    {"zoom_out", "Zoom Out"},
    {"reset_zoom", "Reset Zoom"},
    {"quit", "Quit Application"}
};

QtKeybindEditor::QtKeybindEditor(QWidget *parent)
    : QDialog(parent)
    , application_(nullptr)
    , mainPanel_(nullptr)
    , titleLabel_(nullptr)
    , closeButton_(nullptr)
    , resetButton_(nullptr)
    , exportButton_(nullptr)
    , importButton_(nullptr)
    , profileComboBox_(nullptr)
    , saveProfileButton_(nullptr)
    , deleteProfileButton_(nullptr)
    , searchBox_(nullptr)
    , categoryFilter_(nullptr)
    , actionTree_(nullptr)
    , slideAnimation_(nullptr)
    , fadeAnimation_(nullptr)
    , animationGroup_(nullptr)
    , animationTimer_(nullptr)
    , settings_(nullptr)
    , animationTime_(0.0f)
    , isVisible_(false)
    , isAnimating_(false)
    , isCapturingKey_(false)
    , capturingItem_(nullptr)
{
    setWindowTitle("Keyboard Shortcuts");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(false);
    setFocusPolicy(Qt::StrongFocus);
    
    // Initialize settings
    settings_ = new QSettings("Lyricstator", "Keybinds", this);
    
    setupUI();
    setupStyling();
    setupCallbacks();
    loadKeybindSettings();
    initializeDefaultProfiles();
    
    // Setup animation timer
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &QtKeybindEditor::updateAnimation);
    animationTimer_->start(16); // ~60 FPS
    
    refreshActionList();
}

QtKeybindEditor::~QtKeybindEditor()
{
    saveKeybindSettings();
    if (animationTimer_) {
        animationTimer_->stop();
    }
}

void QtKeybindEditor::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtKeybindEditor::setKeybind(const QString& actionName, const QString& key)
{
    // Find the action in the current profile
    if (profiles_.contains(currentProfile_)) {
        KeybindProfile& profile = profiles_[currentProfile_];
        if (profile.actions.contains(actionName)) {
            profile.actions[actionName].currentKey = key;
            
            // Update the UI
            for (int i = 0; i < actionTree_->topLevelItemCount(); ++i) {
                QTreeWidgetItem* item = actionTree_->topLevelItem(i);
                if (item->data(0, Qt::UserRole).toString() == actionName) {
                    item->setText(2, key);
                    break;
                }
            }
            
            saveKeybindSettings();
        }
    }
}

QString QtKeybindEditor::getKeybind(const QString& actionName) const
{
    if (profiles_.contains(currentProfile_)) {
        const KeybindProfile& profile = profiles_[currentProfile_];
        if (profile.actions.contains(actionName)) {
            return profile.actions[actionName].currentKey;
        }
    }
    return QString();
}

void QtKeybindEditor::resetToDefault()
{
    int ret = QMessageBox::question(this, "Reset Keybinds", 
                                   "Reset all keybinds to default values?",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Reset current profile to default
        if (profiles_.contains(currentProfile_)) {
            KeybindProfile& profile = profiles_[currentProfile_];
            for (auto it = profile.actions.begin(); it != profile.actions.end(); ++it) {
                it->currentKey = it->defaultKey;
            }
            
            refreshActionList();
            saveKeybindSettings();
        }
    }
}

void QtKeybindEditor::loadProfile(const QString& profileName)
{
    if (profiles_.contains(profileName)) {
        currentProfile_ = profileName;
        applyProfile(profiles_[profileName]);
        refreshActionList();
        saveKeybindSettings();
    }
}

void QtKeybindEditor::saveProfile(const QString& profileName, const QString& description)
{
    if (profiles_.contains(currentProfile_)) {
        KeybindProfile newProfile;
        newProfile.name = profileName;
        newProfile.description = description;
        newProfile.actions = profiles_[currentProfile_].actions;
        newProfile.isDefault = false;
        
        profiles_[profileName] = newProfile;
        refreshProfileList();
        saveProfilesToFile();
    }
}

void QtKeybindEditor::deleteProfile(const QString& profileName)
{
    if (profileName == "Default") {
        QMessageBox::warning(this, "Cannot Delete", "The Default profile cannot be deleted.");
        return;
    }
    
    int ret = QMessageBox::question(this, "Delete Profile", 
                                   QString("Delete profile '%1'?").arg(profileName),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        profiles_.remove(profileName);
        if (currentProfile_ == profileName) {
            currentProfile_ = "Default";
        }
        refreshProfileList();
        refreshActionList();
        saveProfilesToFile();
    }
}

QStringList QtKeybindEditor::getAvailableProfiles() const
{
    return profiles_.keys();
}

void QtKeybindEditor::exportKeybinds(const QString& filePath)
{
    if (profiles_.contains(currentProfile_)) {
        QJsonObject rootObj;
        QJsonObject profileObj;
        const KeybindProfile& profile = profiles_[currentProfile_];
        
        profileObj["name"] = profile.name;
        profileObj["description"] = profile.description;
        
        QJsonObject actionsObj;
        for (auto it = profile.actions.begin(); it != profile.actions.end(); ++it) {
            QJsonObject actionObj;
            actionObj["name"] = it->name;
            actionObj["description"] = it->description;
            actionObj["category"] = it->category;
            actionObj["defaultKey"] = it->defaultKey;
            actionObj["currentKey"] = it->currentKey;
            actionObj["enabled"] = it->enabled;
            
            actionsObj[it->name] = actionObj;
        }
        
        profileObj["actions"] = actionsObj;
        rootObj["profile"] = profileObj;
        
        QJsonDocument doc(rootObj);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            QMessageBox::information(this, "Export Successful", 
                                   "Keybinds exported successfully!");
        } else {
            QMessageBox::critical(this, "Export Failed", 
                                "Failed to export keybinds!");
        }
    }
}

void QtKeybindEditor::importKeybinds(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Import Failed", "Failed to open file!");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootObj = doc.object();
    
    if (rootObj.contains("profile")) {
        QJsonObject profileObj = rootObj["profile"].toObject();
        QString profileName = profileObj["name"].toString();
        
        // Check if profile already exists
        if (profiles_.contains(profileName)) {
            int ret = QMessageBox::question(this, "Profile Exists", 
                                           QString("Profile '%1' already exists. Overwrite?").arg(profileName),
                                           QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) return;
        }
        
        // Create new profile
        KeybindProfile newProfile;
        newProfile.name = profileName;
        newProfile.description = profileObj["description"].toString();
        newProfile.isDefault = false;
        
        if (profileObj.contains("actions")) {
            QJsonObject actionsObj = profileObj["actions"].toObject();
            
            for (auto it = actionsObj.begin(); it != actionsObj.end(); ++it) {
                QJsonObject actionObj = it->toObject();
                
                KeybindAction action;
                action.name = actionObj["name"].toString();
                action.description = actionObj["description"].toString();
                action.category = actionObj["category"].toString();
                action.defaultKey = actionObj["defaultKey"].toString();
                action.currentKey = actionObj["currentKey"].toString();
                action.enabled = actionObj["enabled"].toBool();
                action.editable = true;
                
                newProfile.actions[action.name] = action;
            }
        }
        
        profiles_[profileName] = newProfile;
        currentProfile_ = profileName;
        refreshProfileList();
        refreshActionList();
        saveProfilesToFile();
        
        QMessageBox::information(this, "Import Successful", 
                               "Keybinds imported successfully!");
    } else {
        QMessageBox::critical(this, "Import Failed", "Invalid file format!");
    }
}

void QtKeybindEditor::setupUI()
{
    resize(800, 600);
    
    // Center on screen
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    createMainPanel();
    createControlPanel();
    createKeybindList();
    createActionTree();
}

void QtKeybindEditor::createMainPanel()
{
    mainPanel_ = new QWidget(this);
    mainLayout_ = new QVBoxLayout(mainPanel_);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(15);
    
    setCentralWidget(mainPanel_);
}

void QtKeybindEditor::createControlPanel()
{
    // Title bar
    titleLabel_ = new QLabel("Keyboard Shortcuts");
    titleLabel_->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 24px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
    )");
    
    closeButton_ = new QPushButton("×");
    closeButton_->setFixedSize(30, 30);
    closeButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(220, 60, 60);
            color: white;
            border: none;
            border-radius: 15px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(255, 80, 80);
        }
    )");
    
    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->addWidget(titleLabel_);
    titleLayout->addStretch();
    titleLayout->addWidget(closeButton_);
    
    mainLayout_->addLayout(titleLayout);
    
    // Control buttons
    controlLayout_ = new QHBoxLayout();
    
    resetButton_ = new QPushButton("Reset to Default");
    resetButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(200, 150, 50);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: rgb(220, 170, 70);
        }
    )");
    
    exportButton_ = new QPushButton("Export");
    exportButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 150, 100);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: rgb(120, 170, 120);
        }
    )");
    
    importButton_ = new QPushButton("Import");
    importButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 100, 150);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: rgb(120, 120, 170);
        }
    ));
    
    controlLayout_->addWidget(resetButton_);
    controlLayout_->addWidget(exportButton_);
    controlLayout_->addWidget(importButton_);
    controlLayout_->addStretch();
    
    mainLayout_->addLayout(controlLayout_);
}

void QtKeybindEditor::createKeybindList()
{
    // Profile selection
    profileLayout_ = new QHBoxLayout();
    
    QLabel* profileLabel = new QLabel("Profile:");
    profileLabel_->setStyleSheet("color: white; font-weight: bold;");
    
    profileComboBox_ = new QComboBox();
    profileComboBox_->setStyleSheet(R"(
        QComboBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 150px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
    )");
    
    saveProfileButton_ = new QPushButton("Save Profile");
    saveProfileButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(100, 100, 200);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: rgb(120, 120, 220);
        }
    )");
    
    deleteProfileButton_ = new QPushButton("Delete Profile");
    deleteProfileButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(200, 100, 100);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: rgb(220, 120, 120);
        }
    )");
    
    profileLayout_->addWidget(profileLabel);
    profileLayout_->addWidget(profileComboBox_);
    profileLayout_->addWidget(saveProfileButton_);
    profileLayout_->addWidget(deleteProfileButton_);
    profileLayout_->addStretch();
    
    mainLayout_->addLayout(profileLayout_);
    
    // Search and filter
    filterLayout_ = new QHBoxLayout();
    
    searchBox_ = new QLineEdit();
    searchBox_->setPlaceholderText("Search shortcuts...");
    searchBox_->setStyleSheet(R"(
        QLineEdit {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
        }
        QLineEdit:focus {
            border-color: rgb(120, 180, 255);
        }
    )");
    
    QLabel* categoryLabel = new QLabel("Category:");
    categoryLabel->setStyleSheet("color: white; font-weight: bold;");
    
    categoryFilter_ = new QComboBox();
    categoryFilter_->addItem("All Categories");
    categoryFilter_->addItems(DEFAULT_CATEGORIES);
    categoryFilter_->setStyleSheet(R"(
        QComboBox {
            background-color: rgb(40, 45, 55);
            border: 1px solid rgb(100, 150, 255);
            border-radius: 6px;
            color: white;
            padding: 6px;
            min-width: 120px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-style: solid;
            border-width: 3px;
            border-color: white transparent transparent transparent;
        }
    )");
    
    filterLayout_->addWidget(searchBox_);
    filterLayout_->addWidget(categoryLabel);
    filterLayout_->addWidget(categoryFilter_);
    filterLayout_->addStretch();
    
    mainLayout_->addLayout(filterLayout_);
}

void QtKeybindEditor::createActionTree()
{
    actionTree_ = new QTreeWidget();
    actionTree_->setHeaderLabels({"Action", "Description", "Key", "Category"});
    actionTree_->setRootIsDecorated(false);
    actionTree_->setAlternatingRowColors(true);
    actionTree_->setSortingEnabled(true);
    actionTree_->sortByColumn(0, Qt::AscendingOrder);
    
    // Set column widths
    actionTree_->setColumnWidth(0, 200); // Action
    actionTree_->setColumnWidth(1, 250); // Description
    actionTree_->setColumnWidth(2, 100); // Key
    actionTree_->setColumnWidth(3, 120); // Category
    
    actionTree_->setStyleSheet(R"(
        QTreeWidget {
            background-color: rgb(20, 25, 40);
            border: 1px solid rgb(60, 80, 120);
            border-radius: 5px;
            color: white;
            alternate-background-color: rgb(30, 35, 50);
            selection-background-color: rgb(100, 150, 200);
        }
        QTreeWidget::item:hover {
            background-color: rgb(40, 50, 70);
        }
        QTreeWidget::item:selected {
            background-color: rgb(100, 150, 200);
        }
        QHeaderView::section {
            background-color: rgb(40, 50, 70);
            color: white;
            border: 1px solid rgb(60, 80, 120);
            padding: 5px;
        }
    )");
    
    mainLayout_->addWidget(actionTree_);
}

void QtKeybindEditor::setupStyling()
{
    setStyleSheet(R"(
        QDialog {
            background-color: rgba(25, 30, 40, 250);
            border: 2px solid rgb(100, 150, 255);
            border-radius: 12px;
        }
    )");
    
    applyModernStyling();
}

void QtKeybindEditor::applyModernStyling()
{
    // Add shadow effects
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 150));
    shadowEffect->setOffset(5, 5);
    setGraphicsEffect(shadowEffect);
}

void QtKeybindEditor::setupCallbacks()
{
    connect(closeButton_, &QPushButton::clicked, this, &QtKeybindEditor::onCloseClicked);
    connect(resetButton_, &QPushButton::clicked, this, &QtKeybindEditor::onResetClicked);
    connect(exportButton_, &QPushButton::clicked, this, &QtKeybindEditor::onExportClicked);
    connect(importButton_, &QPushButton::clicked, this, &QtKeybindEditor::onImportClicked);
    connect(profileComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtKeybindEditor::onProfileSelected);
    connect(saveProfileButton_, &QPushButton::clicked, this, &QtKeybindEditor::onSaveProfileClicked);
    connect(deleteProfileButton_, &QPushButton::clicked, this, &QtKeybindEditor::onDeleteProfileClicked);
    connect(actionTree_, &QTreeWidget::itemClicked, this, &QtKeybindEditor::onActionItemClicked);
    connect(actionTree_, &QTreeWidget::itemDoubleClicked, this, &QtKeybindEditor::onActionItemDoubleClicked);
    connect(searchBox_, &QLineEdit::textChanged, this, &QtKeybindEditor::onSearchTextChanged);
    connect(categoryFilter_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &QtKeybindEditor::onCategoryFilterChanged);
}

void QtKeybindEditor::loadKeybindSettings()
{
    settings_->beginGroup("Keybinds");
    currentProfile_ = settings_->value("currentProfile", "Default").toString();
    settings_->endGroup();
    
    // Load profiles from file
    loadProfilesFromFile();
    
    // Ensure default profile exists
    if (!profiles_.contains("Default")) {
        initializeDefaultProfiles();
    }
}

void QtKeybindEditor::saveKeybindSettings()
{
    settings_->beginGroup("Keybinds");
    settings_->setValue("currentProfile", currentProfile_);
    settings_->endGroup();
    
    // Save profiles to file
    saveProfilesToFile();
}

void QtKeybindEditor::initializeDefaultProfiles()
{
    // Create default profile
    KeybindProfile defaultProfile;
    defaultProfile.name = "Default";
    defaultProfile.description = "Default keybind configuration";
    defaultProfile.isDefault = true;
    
    // Initialize default actions
    for (auto it = DEFAULT_ACTIONS.begin(); it != DEFAULT_ACTIONS.end(); ++it) {
        KeybindAction action;
        action.name = it.key();
        action.description = it.value();
        action.category = getCategoryForAction(it.key());
        action.defaultKey = getDefaultKeyForAction(it.key());
        action.currentKey = action.defaultKey;
        action.enabled = true;
        action.editable = true;
        
        defaultProfile.actions[action.name] = action;
    }
    
    profiles_["Default"] = defaultProfile;
    
    if (currentProfile_.isEmpty()) {
        currentProfile_ = "Default";
    }
    
    refreshProfileList();
}

void QtKeybindEditor::refreshProfileList()
{
    if (!profileComboBox_) return;
    
    profileComboBox_->clear();
    for (auto it = profiles_.begin(); it != profiles_.end(); ++it) {
        profileComboBox_->addItem(it->name);
    }
    
    // Set current profile
    int index = profileComboBox_->findText(currentProfile_);
    if (index >= 0) {
        profileComboBox_->setCurrentIndex(index);
    }
}

void QtKeybindEditor::refreshActionList()
{
    if (!actionTree_) return;
    
    actionTree_->clear();
    allActions_.clear();
    
    if (profiles_.contains(currentProfile_)) {
        const KeybindProfile& profile = profiles_[currentProfile_];
        
        for (auto it = profile.actions.begin(); it != profile.actions.end(); ++it) {
            allActions_.append(it.value());
        }
    }
    
    // Apply filtering
    filterActions();
    
    // Create tree items
    for (const KeybindAction& action : filteredActions_) {
        createActionItem(action);
    }
}

void QtKeybindEditor::applyProfile(const KeybindProfile& profile)
{
    // Apply the profile's keybinds to the application
    if (application_) {
        for (auto it = profile.actions.begin(); it != profile.actions.end(); ++it) {
            // TODO: Call application keybind methods
            // application_->setKeybind(it->name, it->currentKey);
        }
    }
}

void QtKeybindEditor::showWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    // Setup animations
    slideAnimation_ = new QPropertyAnimation(this, "pos");
    slideAnimation_->setDuration(400);
    slideAnimation_->setStartValue(QPoint(x() - width(), y()));
    slideAnimation_->setEndValue(QPoint(x(), y()));
    slideAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    fadeAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    fadeAnimation_->setDuration(400);
    fadeAnimation_->setStartValue(0.0);
    fadeAnimation_->setEndValue(1.0);
    fadeAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    animationGroup_ = new QParallelAnimationGroup();
    animationGroup_->addAnimation(slideAnimation_);
    animationGroup_->addAnimation(fadeAnimation_);
    
    connect(animationGroup_, &QParallelAnimationGroup::finished, [this]() {
        isAnimating_ = false;
        animationGroup_->deleteLater();
        animationGroup_ = nullptr;
    });
    
    animationGroup_->start();
}

void QtKeybindEditor::hideWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    fadeAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    fadeAnimation_->setDuration(300);
    fadeAnimation_->setStartValue(1.0);
    fadeAnimation_->setEndValue(0.0);
    fadeAnimation_->setEasingCurve(QEasingCurve::InCubic);
    
    connect(fadeAnimation_, &QPropertyAnimation::finished, [this]() {
        isVisible_ = false;
        isAnimating_ = false;
        QDialog::hide();
        fadeAnimation_->deleteLater();
        fadeAnimation_ = nullptr;
    });
    
    fadeAnimation_->start();
}

void QtKeybindEditor::startKeyCapture(QTreeWidgetItem* item)
{
    if (isCapturingKey_) return;
    
    isCapturingKey_ = true;
    capturingItem_ = item;
    capturedKey_.clear();
    
    // Highlight the item
    item->setBackground(2, QColor(255, 255, 0, 100));
    item->setText(2, "Press a key...");
    
    // Set focus to capture keys
    setFocus();
    
    onKeyCaptureStarted();
}

void QtKeybindEditor::stopKeyCapture()
{
    if (!isCapturingKey_) return;
    
    isCapturingKey_ = false;
    
    if (capturingItem_ && !capturedKey_.isEmpty()) {
        // Update the item
        capturingItem_->setText(2, capturedKey_);
        capturingItem_->setBackground(2, QColor());
        
        // Update the action
        QString actionName = capturingItem_->data(0, Qt::UserRole).toString();
        setKeybind(actionName, capturedKey_);
    }
    
    capturingItem_ = nullptr;
    capturedKey_.clear();
    
    onKeyCaptureFinished();
}

QString QtKeybindEditor::keyEventToString(QKeyEvent* event) const
{
    return keyToString(event->key(), event->modifiers());
}

QString QtKeybindEditor::keyToString(int key, Qt::KeyboardModifiers modifiers) const
{
    QStringList parts;
    
    // Add modifiers
    if (modifiers & Qt::ControlModifier) parts << "Ctrl";
    if (modifiers & Qt::AltModifier) parts << "Alt";
    if (modifiers & Qt::ShiftModifier) parts << "Shift";
    if (modifiers & Qt::MetaModifier) parts << "Meta";
    
    // Add key
    QString keyText;
    switch (key) {
        case Qt::Key_Space: keyText = "Space"; break;
        case Qt::Key_Tab: keyText = "Tab"; break;
        case Qt::Key_Return: keyText = "Enter"; break;
        case Qt::Key_Enter: keyText = "Enter"; break;
        case Qt::Key_Escape: keyText = "Escape"; break;
        case Qt::Key_Backspace: keyText = "Backspace"; break;
        case Qt::Key_Delete: keyText = "Delete"; break;
        case Qt::Key_Insert: keyText = "Insert"; break;
        case Qt::Key_Home: keyText = "Home"; break;
        case Qt::Key_End: keyText = "End"; break;
        case Qt::Key_PageUp: keyText = "Page Up"; break;
        case Qt::Key_PageDown: keyText = "Page Down"; break;
        case Qt::Key_Up: keyText = "Up"; break;
        case Qt::Key_Down: keyText = "Down"; break;
        case Qt::Key_Left: keyText = "Left"; break;
        case Qt::Key_Right: keyText = "Right"; break;
        case Qt::Key_F1: keyText = "F1"; break;
        case Qt::Key_F2: keyText = "F2"; break;
        case Qt::Key_F3: keyText = "F3"; break;
        case Qt::Key_F4: keyText = "F4"; break;
        case Qt::Key_F5: keyText = "F5"; break;
        case Qt::Key_F6: keyText = "F6"; break;
        case Qt::Key_F7: keyText = "F7"; break;
        case Qt::Key_F8: keyText = "F8"; break;
        case Qt::Key_F9: keyText = "F9"; break;
        case Qt::Key_F10: keyText = "F10"; break;
        case Qt::Key_F11: keyText = "F11"; break;
        case Qt::Key_F12: keyText = "F12"; break;
        default:
            if (key >= Qt::Key_A && key <= Qt::Key_Z) {
                keyText = QChar(key);
            } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
                keyText = QChar(key);
            } else {
                keyText = QString("Key%1").arg(key);
            }
            break;
    }
    
    if (!keyText.isEmpty()) {
        parts << keyText;
    }
    
    return parts.join("+");
}

void QtKeybindEditor::updateActionItem(QTreeWidgetItem* item, const KeybindAction& action)
{
    item->setText(0, action.name);
    item->setText(1, action.description);
    item->setText(2, action.currentKey);
    item->setText(3, action.category);
    item->setData(0, Qt::UserRole, action.name);
    
    // Set item flags
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (action.editable) {
        flags |= Qt::ItemIsEditable;
    }
    item->setFlags(flags);
}

void QtKeybindEditor::createActionItem(const KeybindAction& action)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(actionTree_);
    updateActionItem(item, action);
}

void QtKeybindEditor::filterActions()
{
    filteredActions_.clear();
    
    QString searchText = searchBox_->text().toLower();
    QString categoryFilter = categoryFilter_->currentText();
    
    for (const KeybindAction& action : allActions_) {
        bool passesSearch = searchText.isEmpty() ||
                           action.name.toLower().contains(searchText) ||
                           action.description.toLower().contains(searchText);
        
        bool passesCategory = categoryFilter == "All Categories" ||
                             action.category == categoryFilter;
        
        if (passesSearch && passesCategory) {
            filteredActions_.append(action);
        }
    }
}

void QtKeybindEditor::loadProfilesFromFile()
{
    QString profilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/keybind_profiles.json";
    QFile file(profilesPath);
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject rootObj = doc.object();
        
        if (rootObj.contains("profiles")) {
            QJsonArray profilesArray = rootObj["profiles"].toArray();
            
            for (const QJsonValue& value : profilesArray) {
                QJsonObject profileObj = value.toObject();
                
                KeybindProfile profile;
                profile.name = profileObj["name"].toString();
                profile.description = profileObj["description"].toString();
                profile.isDefault = profileObj["isDefault"].toBool();
                
                if (profileObj.contains("actions")) {
                    QJsonObject actionsObj = profileObj["actions"].toObject();
                    
                    for (auto it = actionsObj.begin(); it != actionsObj.end(); ++it) {
                        QJsonObject actionObj = it->toObject();
                        
                        KeybindAction action;
                        action.name = actionObj["name"].toString();
                        action.description = actionObj["description"].toString();
                        action.category = actionObj["category"].toString();
                        action.defaultKey = actionObj["defaultKey"].toString();
                        action.currentKey = actionObj["currentKey"].toString();
                        action.enabled = actionObj["enabled"].toBool();
                        action.editable = actionObj["editable"].toBool();
                        
                        profile.actions[action.name] = action;
                    }
                }
                
                profiles_[profile.name] = profile;
            }
        }
    }
}

void QtKeybindEditor::saveProfilesToFile()
{
    QString profilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/keybind_profiles.json";
    QDir().mkpath(QFileInfo(profilesPath).absolutePath());
    
    QFile file(profilesPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject rootObj;
        QJsonArray profilesArray;
        
        for (auto it = profiles_.begin(); it != profiles_.end(); ++it) {
            QJsonObject profileObj;
            profileObj["name"] = it->name;
            profileObj["description"] = it->description;
            profileObj["isDefault"] = it->isDefault;
            
            QJsonObject actionsObj;
            for (auto actionIt = it->actions.begin(); actionIt != it->actions.end(); ++actionIt) {
                QJsonObject actionObj;
                actionObj["name"] = actionIt->name;
                actionObj["description"] = actionIt->description;
                actionObj["category"] = actionIt->category;
                actionObj["defaultKey"] = actionIt->defaultKey;
                actionObj["currentKey"] = actionIt->currentKey;
                actionObj["enabled"] = actionIt->enabled;
                actionObj["editable"] = actionIt->editable;
                
                actionsObj[actionIt->name] = actionObj;
            }
            
            profileObj["actions"] = actionsObj;
            profilesArray.append(profileObj);
        }
        
        rootObj["profiles"] = profilesArray;
        
        QJsonDocument doc(rootObj);
        file.write(doc.toJson());
    }
}

void QtKeybindEditor::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    // Custom painting if needed
}

void QtKeybindEditor::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
}

void QtKeybindEditor::keyPressEvent(QKeyEvent* event)
{
    if (isCapturingKey_) {
        capturedKey_ = keyEventToString(event);
        stopKeyCapture();
        event->accept();
        return;
    }
    
    QDialog::keyPressEvent(event);
}

void QtKeybindEditor::keyReleaseEvent(QKeyEvent* event)
{
    if (isCapturingKey_) {
        event->accept();
        return;
    }
    
    QDialog::keyReleaseEvent(event);
}

void QtKeybindEditor::focusOutEvent(QFocusEvent* event)
{
    if (isCapturingKey_) {
        stopKeyCapture();
    }
    
    QDialog::focusOutEvent(event);
}

// Helper methods
QString QtKeybindEditor::getCategoryForAction(const QString& actionName) const
{
    if (actionName.contains("play") || actionName.contains("stop") || actionName.contains("track")) {
        return "Playback";
    } else if (actionName.contains("seek") || actionName.contains("volume")) {
        return "Audio";
    } else if (actionName.contains("open") || actionName.contains("file")) {
        return "Interface";
    } else if (actionName.contains("gui") || actionName.contains("editor")) {
        return "Tools";
    } else if (actionName.contains("quit") || actionName.contains("fullscreen")) {
        return "System";
    } else {
        return "Interface";
    }
}

QString QtKeybindEditor::getDefaultKeyForAction(const QString& actionName) const
{
    // Default key mappings
    static QMap<QString, QString> defaultKeys = {
        {"play_pause", "Space"},
        {"stop", "Escape"},
        {"next_track", "Right"},
        {"previous_track", "Left"},
        {"seek_forward", "Up"},
        {"seek_backward", "Down"},
        {"volume_up", "="},
        {"volume_down", "-"},
        {"mute", "M"},
        {"open_file", "Ctrl+O"},
        {"open_midi", "Ctrl+M"},
        {"open_audio", "Ctrl+A"},
        {"open_lyrics", "Ctrl+L"},
        {"resource_pack", "F1"},
        {"equalizer", "F2"},
        {"keybind_editor", "F3"},
        {"song_browser", "F4"},
        {"fullscreen", "F11"},
        {"zoom_in", "Ctrl++"},
        {"zoom_out", "Ctrl+-"},
        {"reset_zoom", "Ctrl+0"},
        {"quit", "Ctrl+Q"}
    };
    
    return defaultKeys.value(actionName, "");
}

// Slot implementations
void QtKeybindEditor::onCloseClicked()
{
    hideWithAnimation();
}

void QtKeybindEditor::onResetClicked()
{
    resetToDefault();
}

void QtKeybindEditor::onExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export Keybinds", 
                                                   "keybinds.json", "JSON Files (*.json)");
    if (!filePath.isEmpty()) {
        exportKeybinds(filePath);
    }
}

void QtKeybindEditor::onImportClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Import Keybinds", 
                                                   "", "JSON Files (*.json)");
    if (!filePath.isEmpty()) {
        importKeybinds(filePath);
    }
}

void QtKeybindEditor::onProfileSelected()
{
    if (profileComboBox_) {
        QString profileName = profileComboBox_->currentText();
        if (profiles_.contains(profileName)) {
            loadProfile(profileName);
        }
    }
}

void QtKeybindEditor::onSaveProfileClicked()
{
    bool ok;
    QString profileName = QInputDialog::getText(this, "Save Profile", 
                                               "Profile name:", QLineEdit::Normal, "", &ok);
    if (ok && !profileName.isEmpty()) {
        QString description = QInputDialog::getText(this, "Save Profile", 
                                                  "Description (optional):", QLineEdit::Normal, "", &ok);
        saveProfile(profileName, description);
    }
}

void QtKeybindEditor::onDeleteProfileClicked()
{
    if (profileComboBox_) {
        QString profileName = profileComboBox_->currentText();
        deleteProfile(profileName);
    }
}

void QtKeybindEditor::onActionItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    if (!item) return;
    
    // Show action info in status or tooltip
    QString actionName = item->data(0, Qt::UserRole).toString();
    QString currentKey = item->text(2);
    
    // Could update a status bar or info panel here
}

void QtKeybindEditor::onActionItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (column == 2 && item) { // Key column
        QString actionName = item->data(0, Qt::UserRole).toString();
        
        // Check if action is editable
        if (profiles_.contains(currentProfile_)) {
            const KeybindProfile& profile = profiles_[currentProfile_];
            if (profile.actions.contains(actionName) && profile.actions[actionName].editable) {
                startKeyCapture(item);
            }
        }
    }
}

void QtKeybindEditor::onSearchTextChanged()
{
    filterActions();
    refreshActionList();
}

void QtKeybindEditor::onCategoryFilterChanged()
{
    filterActions();
    refreshActionList();
}

void QtKeybindEditor::onKeyCaptureStarted()
{
    // Could show a visual indicator or status message
}

void QtKeybindEditor::onKeyCaptureFinished()
{
    // Could hide visual indicators or status messages
}

void QtKeybindEditor::updateAnimation()
{
    animationTime_ += 0.016f; // Assume 60 FPS
    
    // Subtle animations could be added here
    if (isVisible_ && !isAnimating_) {
        // Could add subtle effects like highlighting, etc.
    }
}

void QtKeybindEditor::show()
{
    isVisible_ = true;
    QDialog::show();
    showWithAnimation();
}

void QtKeybindEditor::hide()
{
    hideWithAnimation();
}

void QtKeybindEditor::toggle()
{
    if (isVisible_) {
        hide();
    } else {
        show();
    }
}