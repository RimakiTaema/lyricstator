#include "QtResourcePackGUI.h"
#include <QApplication>
#include <QScreen>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QPixmap>
#include <QIcon>
#include <QStyleOption>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QEasingCurve>
#include <cmath>

QtResourcePackGUI::QtResourcePackGUI(QWidget *parent)
    : QDialog(parent)
    , application_(nullptr)
    , mainPanel_(nullptr)
    , titleLabel_(nullptr)
    , closeButton_(nullptr)
    , packList_(nullptr)
    , previewPanel_(nullptr)
    , previewScrollArea_(nullptr)
    , applyButton_(nullptr)
    , slideAnimation_(nullptr)
    , fadeAnimation_(nullptr)
    , animationGroup_(nullptr)
    , animationTimer_(nullptr)
    , animationTime_(0.0f)
    , isVisible_(false)
    , isAnimating_(false)
{
    setWindowTitle("Resource Pack Manager");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    
    setupUI();
    setupStyling();
    createInterface();
    applyModernStyling();
    
    // Setup animation timer
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, &QtResourcePackGUI::updateAnimation);
    animationTimer_->start(16); // ~60 FPS
    
    loadResourcePacks();
}

QtResourcePackGUI::~QtResourcePackGUI()
{
    if (animationTimer_) {
        animationTimer_->stop();
    }
}

void QtResourcePackGUI::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtResourcePackGUI::setupUI()
{
    resize(900, 600);
    
    // Center on screen
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void QtResourcePackGUI::setupStyling()
{
    setStyleSheet(R"(
        QDialog {
            background-color: rgba(25, 30, 45, 250);
            border: 3px solid rgba(100, 150, 200, 255);
            border-radius: 15px;
        }
    )");
}

void QtResourcePackGUI::createInterface()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(15);
    
    // Title bar
    QHBoxLayout* titleLayout = new QHBoxLayout();
    
    titleLabel_ = new QLabel("🎨 Resource Packs");
    titleLabel_->setStyleSheet(R"(
        QLabel {
            color: rgb(200, 220, 255);
            font-size: 28px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
    )");
    
    closeButton_ = new QPushButton("✕");
    closeButton_->setFixedSize(40, 40);
    closeButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(180, 50, 50);
            color: white;
            border: none;
            border-radius: 20px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(220, 70, 70);
        }
        QPushButton:pressed {
            background-color: rgb(160, 40, 40);
        }
    )");
    connect(closeButton_, &QPushButton::clicked, this, &QtResourcePackGUI::onCloseClicked);
    
    titleLayout->addWidget(titleLabel_);
    titleLayout->addStretch();
    titleLayout->addWidget(closeButton_);
    
    mainLayout_->addLayout(titleLayout);
    
    // Content area
    contentLayout_ = new QHBoxLayout();
    contentLayout_->setSpacing(20);
    
    createPackList();
    createPreviewPanel();
    
    contentLayout_->addWidget(packList_, 1);
    contentLayout_->addWidget(previewPanel_, 1);
    
    mainLayout_->addLayout(contentLayout_);
}

void QtResourcePackGUI::createPackList()
{
    packList_ = new QListWidget();
    packList_->setStyleSheet(R"(
        QListWidget {
            background-color: rgb(15, 20, 35);
            border: 2px solid rgb(80, 100, 140);
            border-radius: 10px;
            color: white;
            font-size: 14px;
            padding: 10px;
        }
        QListWidget::item {
            background-color: rgb(60, 80, 120);
            border: 1px solid rgb(100, 120, 160);
            border-radius: 8px;
            padding: 15px;
            margin: 5px;
        }
        QListWidget::item:hover {
            background-color: rgb(80, 100, 140);
        }
        QListWidget::item:selected {
            background-color: rgb(100, 150, 200);
        }
    )");
    
    connect(packList_, &QListWidget::itemSelectionChanged, this, &QtResourcePackGUI::onPackSelected);
    
    // Add shadow effect
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    shadowEffect->setOffset(5, 5);
    packList_->setGraphicsEffect(shadowEffect);
}

void QtResourcePackGUI::createPreviewPanel()
{
    previewPanel_ = new QFrame();
    previewPanel_->setStyleSheet(R"(
        QFrame {
            background-color: rgb(20, 25, 40);
            border: 2px solid rgb(80, 100, 140);
            border-radius: 10px;
        }
    )");
    
    previewLayout_ = new QVBoxLayout(previewPanel_);
    previewLayout_->setContentsMargins(15, 15, 15, 15);
    previewLayout_->setSpacing(10);
    
    // Preview title
    QLabel* previewTitle = new QLabel("Theme Preview");
    previewTitle->setStyleSheet(R"(
        QLabel {
            color: rgb(200, 220, 255);
            font-size: 20px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
    )");
    previewLayout_->addWidget(previewTitle);
    
    // Scroll area for preview content
    previewScrollArea_ = new QScrollArea();
    previewScrollArea_->setStyleSheet(R"(
        QScrollArea {
            background: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background-color: rgb(40, 50, 70);
            border: none;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: rgb(100, 120, 160);
            border-radius: 6px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: rgb(120, 140, 180);
        }
    )");
    previewScrollArea_->setWidgetResizable(true);
    previewScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    previewScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    previewLayout_->addWidget(previewScrollArea_);
    
    // Apply button
    applyButton_ = new QPushButton("Apply Theme");
    applyButton_->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(50, 150, 50);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: bold;
            padding: 12px;
        }
        QPushButton:hover {
            background-color: rgb(70, 170, 70);
        }
        QPushButton:pressed {
            background-color: rgb(40, 130, 40);
        }
    )");
    connect(applyButton_, &QPushButton::clicked, this, &QtResourcePackGUI::onApplyTheme);
    
    previewLayout_->addWidget(applyButton_);
    
    // Add shadow effect
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    shadowEffect->setOffset(5, 5);
    previewPanel_->setGraphicsEffect(shadowEffect);
}

void QtResourcePackGUI::applyModernStyling()
{
    // Add subtle glow effect to the entire dialog
    QGraphicsDropShadowEffect* glowEffect = new QGraphicsDropShadowEffect();
    glowEffect->setBlurRadius(30);
    glowEffect->setColor(QColor(100, 150, 200, 150));
    glowEffect->setOffset(0, 0);
    setGraphicsEffect(glowEffect);
}

void QtResourcePackGUI::loadResourcePacks()
{
    availablePacks_.clear();
    packList_->clear();
    
    QString packsDir = "assets/resource_packs";
    QDir dir(packsDir);
    
    if (dir.exists()) {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        
        for (const QString& subdir : subdirs) {
            QString packPath = dir.absoluteFilePath(subdir);
            QDir packDir(packPath);
            
            // Check if pack has required files
            if (packDir.exists("theme.json") || packDir.exists("pack.json")) {
                availablePacks_.append(subdir);
                
                // Create list item with icon if available
                QListWidgetItem* item = new QListWidgetItem(subdir);
                
                // Try to load pack icon
                QString iconPath = packDir.absoluteFilePath("icon.png");
                if (QFileInfo::exists(iconPath)) {
                    QPixmap pixmap(iconPath);
                    if (!pixmap.isNull()) {
                        item->setIcon(QIcon(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
                    }
                }
                
                packList_->addItem(item);
            }
        }
    }
    
    // Add default theme
    if (!availablePacks_.contains("default")) {
        availablePacks_.prepend("default");
        QListWidgetItem* defaultItem = new QListWidgetItem("🎭 Default Theme");
        packList_->insertItem(0, defaultItem);
    }
}

void QtResourcePackGUI::refreshPacks()
{
    loadResourcePacks();
}

void QtResourcePackGUI::onPackSelected()
{
    QListWidgetItem* currentItem = packList_->currentItem();
    if (!currentItem) return;
    
    QString packName = currentItem->text();
    if (packName.startsWith("🎭 ")) {
        packName = "default";
    }
    
    currentPack_ = packName;
    createPackPreview(packName);
}

void QtResourcePackGUI::createPackPreview(const QString& packName)
{
    // Clear existing preview
    QWidget* oldWidget = previewScrollArea_->widget();
    if (oldWidget) {
        oldWidget->deleteLater();
    }
    
    QWidget* previewWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(previewWidget);
    layout->setSpacing(15);
    
    // Pack name
    QLabel* packLabel = new QLabel("🎭 " + packName);
    packLabel->setStyleSheet(R"(
        QLabel {
            color: rgb(200, 220, 255);
            font-size: 18px;
            font-weight: bold;
            background: transparent;
            border: none;
        }
    )");
    layout->addWidget(packLabel);
    
    // Load pack information
    loadPackInfo(packName);
    
    // Description
    QLabel* descLabel = new QLabel("Theme Preview");
    descLabel->setStyleSheet(R"(
        QLabel {
            color: rgb(150, 170, 200);
            font-size: 14px;
            background: transparent;
            border: none;
        }
    )");
    layout->addWidget(descLabel);
    
    // Color swatches
    if (!themeColors_.isEmpty()) {
        for (auto it = themeColors_.begin(); it != themeColors_.end(); ++it) {
            QString colorName = it.key();
            QColor color = it.value();
            
            QFrame* colorFrame = new QFrame();
            colorFrame->setFixedHeight(40);
            colorFrame->setStyleSheet(QString(R"(
                QFrame {
                    background-color: rgb(%1, %2, %3);
                    border-radius: 5px;
                    border: 1px solid rgb(100, 120, 160);
                }
            )").arg(color.red()).arg(color.green()).arg(color.blue()));
            
            QLabel* colorLabel = new QLabel(colorName.toUpper() + " COLOR");
            colorLabel->setStyleSheet(R"(
                QLabel {
                    color: white;
                    font-size: 12px;
                    font-weight: bold;
                    background: rgba(0, 0, 0, 100);
                    border-radius: 3px;
                    padding: 4px 8px;
                }
            )");
            
            QVBoxLayout* colorLayout = new QVBoxLayout(colorFrame);
            colorLayout->addWidget(colorLabel);
            colorLayout->setAlignment(Qt::AlignCenter);
            
            layout->addWidget(colorFrame);
        }
    }
    
    layout->addStretch();
    previewScrollArea_->setWidget(previewWidget);
}

void QtResourcePackGUI::loadPackInfo(const QString& packName)
{
    themeColors_.clear();
    
    if (packName == "default") {
        // Default theme colors
        themeColors_["primary"] = QColor(255, 255, 255);
        themeColors_["accent"] = QColor(100, 200, 255);
        themeColors_["highlight"] = QColor(255, 215, 0);
        return;
    }
    
    QString packPath = QString("assets/resource_packs/%1").arg(packName);
    QDir packDir(packPath);
    
    // Try to load theme.json
    QString themeFile = packDir.absoluteFilePath("theme.json");
    if (QFileInfo::exists(themeFile)) {
        QFile file(themeFile);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject themeObj = doc.object();
            
            if (themeObj.contains("colors")) {
                QJsonObject colorsObj = themeObj["colors"].toObject();
                
                for (auto it = colorsObj.begin(); it != colorsObj.end(); ++it) {
                    QString colorName = it.key();
                    QJsonValue colorValue = it.value();
                    
                    if (colorValue.isArray()) {
                        QJsonArray colorArray = colorValue.toArray();
                        if (colorArray.size() >= 3) {
                            int r = colorArray[0].toInt();
                            int g = colorArray[1].toInt();
                            int b = colorArray[2].toInt();
                            themeColors_[colorName] = QColor(r, g, b);
                        }
                    } else if (colorValue.isString()) {
                        QString colorString = colorValue.toString();
                        QColor color(colorString);
                        if (color.isValid()) {
                            themeColors_[colorName] = color;
                        }
                    }
                }
            }
        }
    }
    
    // Fallback colors if none loaded
    if (themeColors_.isEmpty()) {
        if (packName == "neon") {
            themeColors_["primary"] = QColor(0, 255, 255);
            themeColors_["accent"] = QColor(255, 0, 255);
            themeColors_["highlight"] = QColor(255, 255, 0);
        } else if (packName == "retro") {
            themeColors_["primary"] = QColor(255, 100, 150);
            themeColors_["accent"] = QColor(100, 255, 200);
            themeColors_["highlight"] = QColor(255, 200, 100);
        } else {
            themeColors_["primary"] = QColor(255, 255, 255);
            themeColors_["accent"] = QColor(100, 200, 255);
            themeColors_["highlight"] = QColor(255, 215, 0);
        }
    }
}

void QtResourcePackGUI::onApplyTheme()
{
    if (currentPack_.isEmpty()) return;
    
    if (application_) {
        // Apply theme through application
        // application_->setResourcePack(currentPack_.toStdString());
    }
    
    QMessageBox::information(this, "Theme Applied", 
                           QString("Applied theme: %1").arg(currentPack_));
    
    hideWithAnimation();
}

void QtResourcePackGUI::onCloseClicked()
{
    hideWithAnimation();
}

void QtResourcePackGUI::show()
{
    isVisible_ = true;
    QDialog::show();
    showWithAnimation();
    loadResourcePacks(); // Refresh packs when showing
}

void QtResourcePackGUI::hide()
{
    hideWithAnimation();
}

void QtResourcePackGUI::toggle()
{
    if (isVisible_) {
        hide();
    } else {
        show();
    }
}

void QtResourcePackGUI::showWithAnimation()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    
    // Setup animations
    slideAnimation_ = new QPropertyAnimation(this, "pos");
    slideAnimation_->setDuration(400);
    slideAnimation_->setStartValue(QPoint(x(), -height()));
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

void QtResourcePackGUI::hideWithAnimation()
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

void QtResourcePackGUI::updateAnimation()
{
    animationTime_ += 0.016f; // Assume 60 FPS
    
    // Subtle pulsing effect for the main panel
    if (isVisible_ && !isAnimating_) {
        float pulse = 0.95f + 0.05f * std::sin(animationTime_ * 2.0f);
        // Could add subtle scaling or opacity effects here
        setWindowOpacity(pulse * 0.98f + 0.02f);
    }
}