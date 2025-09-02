#include "QtKaraokeDisplay.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QFontMetrics>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <cmath>

QtKaraokeDisplay::QtKaraokeDisplay(QWidget *parent)
    : QWidget(parent)
    , application_(nullptr)
    , currentLyric_("Ready to sing!")
    , currentTimestamp_(0.0)
    , currentPitch_(0.0)
    , progress_(0.0)
    , totalDuration_(0.0)
    , resourcePackName_("default")
    , isPlaying_(false)
    , isDragging_(false)
    , zoomLevel_(1.0)
    , backgroundColor_(QColor(20, 25, 40, 200))
    , primaryTextColor_(QColor(255, 255, 255))
    , secondaryTextColor_(QColor(150, 170, 200))
    , accentColor_(QColor(100, 200, 255))
    , progressColor_(QColor(50, 150, 50))
    , pitchColor_(QColor(255, 215, 0))
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(800, 600);
    
    setupUI();
    setupTimers();
    updateFonts();
    updateColors();
}

QtKaraokeDisplay::~QtKaraokeDisplay()
{
    if (displayTimer_) {
        displayTimer_->stop();
    }
    if (pitchTimer_) {
        pitchTimer_->stop();
    }
}

void QtKaraokeDisplay::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtKaraokeDisplay::setCurrentLyric(const QString& lyric, double timestamp)
{
    if (currentLyric_ != lyric) {
        currentLyric_ = lyric;
        currentTimestamp_ = timestamp;
        
        // Trigger animation for new lyric
        QPropertyAnimation* animation = new QPropertyAnimation(this, "opacity");
        animation->setDuration(300);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        
        update();
    }
}

void QtKaraokeDisplay::setCurrentPitch(double pitch)
{
    currentPitch_ = pitch;
    update();
}

void QtKaraokeDisplay::setProgress(double progress)
{
    progress_ = std::clamp(progress, 0.0, 1.0);
    update();
}

void QtKaraokeDisplay::setTotalDuration(double duration)
{
    totalDuration_ = duration;
    update();
}

void QtKaraokeDisplay::setResourcePack(const QString& packName)
{
    resourcePackName_ = packName;
    applyTheme(packName);
    update();
}

void QtKaraokeDisplay::setupUI()
{
    // Set up layout rectangles (will be updated in resizeEvent)
    resizeEvent(nullptr);
}

void QtKaraokeDisplay::setupTimers()
{
    displayTimer_ = new QTimer(this);
    connect(displayTimer_, &QTimer::timeout, this, &QtKaraokeDisplay::updateDisplay);
    displayTimer_->start(16); // ~60 FPS
    
    pitchTimer_ = new QTimer(this);
    connect(pitchTimer_, &QTimer::timeout, this, &QtKaraokeDisplay::updatePitch);
    pitchTimer_->start(50); // 20 FPS for pitch updates
}

void QtKaraokeDisplay::updateFonts()
{
    titleFont_ = QFont("Arial", 32, QFont::Bold);
    lyricFont_ = QFont("Arial", 48, QFont::Normal);
    infoFont_ = QFont("Arial", 16, QFont::Normal);
    smallFont_ = QFont("Arial", 12, QFont::Normal);
}

void QtKaraokeDisplay::updateColors()
{
    // Apply resource pack colors if available
    if (resourcePackColors_.contains("primary")) {
        primaryTextColor_ = resourcePackColors_["primary"];
    }
    if (resourcePackColors_.contains("accent")) {
        accentColor_ = resourcePackColors_["accent"];
    }
    if (resourcePackColors_.contains("highlight")) {
        pitchColor_ = resourcePackColors_["highlight"];
    }
}

void QtKaraokeDisplay::applyTheme(const QString& themeName)
{
    resourcePackColors_.clear();
    
    if (themeName == "neon") {
        primaryTextColor_ = QColor(0, 255, 255);
        accentColor_ = QColor(255, 0, 255);
        pitchColor_ = QColor(255, 255, 0);
        backgroundColor_ = QColor(10, 0, 20, 200);
    } else if (themeName == "retro") {
        primaryTextColor_ = QColor(255, 100, 150);
        accentColor_ = QColor(100, 255, 200);
        pitchColor_ = QColor(255, 200, 100);
        backgroundColor_ = QColor(30, 20, 10, 200);
    } else {
        // Default theme
        primaryTextColor_ = QColor(255, 255, 255);
        accentColor_ = QColor(100, 200, 255);
        pitchColor_ = QColor(255, 215, 0);
        backgroundColor_ = QColor(20, 25, 40, 200);
    }
    
    secondaryTextColor_ = QColor(primaryTextColor_.red() * 0.7, 
                                primaryTextColor_.green() * 0.7, 
                                primaryTextColor_.blue() * 0.7);
}

void QtKaraokeDisplay::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    drawBackground(painter);
    drawLyrics(painter);
    drawProgressBar(painter);
    drawPitchVisualizer(painter);
    drawResourcePackInfo(painter);
    drawControls(painter);
}

void QtKaraokeDisplay::drawBackground(QPainter& painter)
{
    // Animated background with subtle pulsing
    static qint64 animationTime = 0;
    animationTime += 16; // Assume 60 FPS
    
    float pulseIntensity = 0.8f + 0.2f * std::sin(animationTime * 0.002f);
    QColor bgColor = backgroundColor_;
    bgColor.setRed(std::min(255, (int)(bgColor.red() * pulseIntensity)));
    bgColor.setGreen(std::min(255, (int)(bgColor.green() * pulseIntensity)));
    bgColor.setBlue(std::min(255, (int)(bgColor.blue() * pulseIntensity)));
    
    // Gradient background
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, bgColor);
    gradient.setColorAt(1, bgColor.darker(150));
    
    painter.fillRect(rect(), gradient);
    
    // Border
    QPen borderPen(accentColor_, 3);
    painter.setPen(borderPen);
    painter.drawRoundedRect(rect().adjusted(1, 1, -2, -2), 15, 15);
}

void QtKaraokeDisplay::drawLyrics(QPainter& painter)
{
    if (currentLyric_.isEmpty()) return;
    
    painter.setFont(lyricFont_);
    
    // Highlighted lyric effect
    static qint64 animationTime = 0;
    animationTime += 16;
    
    float highlightProgress = progress_; // Use progress for highlighting
    float intensity = 0.5f + 0.5f * std::sin(animationTime * 0.003f);
    
    QColor textColor = primaryTextColor_;
    if (highlightProgress > 0.1f) {
        // Create highlight color based on progress
        textColor = QColor(
            std::min(255, (int)(pitchColor_.red() * intensity)),
            std::min(255, (int)(pitchColor_.green() * intensity)),
            std::min(255, (int)(pitchColor_.blue() * intensity))
        );
    }
    
    painter.setPen(textColor);
    
    // Draw with shadow for better readability
    painter.setPen(QColor(0, 0, 0, 128));
    painter.drawText(lyricsRect_.adjusted(2, 2, 2, 2), Qt::AlignCenter, currentLyric_);
    
    painter.setPen(textColor);
    painter.drawText(lyricsRect_, Qt::AlignCenter, currentLyric_);
}

void QtKaraokeDisplay::drawProgressBar(QPainter& painter)
{
    QRect barRect = progressRect_;
    
    // Background
    painter.fillRect(barRect, QColor(50, 50, 50));
    
    // Progress
    QRect fillRect = barRect;
    fillRect.setWidth((int)(barRect.width() * progress_));
    
    QLinearGradient progressGradient(fillRect.topLeft(), fillRect.topRight());
    progressGradient.setColorAt(0, progressColor_);
    progressGradient.setColorAt(1, progressColor_.lighter(150));
    
    painter.fillRect(fillRect, progressGradient);
    
    // Border
    painter.setPen(accentColor_);
    painter.drawRect(barRect);
    
    // Time text
    painter.setFont(smallFont_);
    painter.setPen(primaryTextColor_);
    
    double currentTime = progress_ * totalDuration_;
    QString timeText = QString("%1:%2 / %3:%4")
                      .arg((int)currentTime / 60, 2, 10, QChar('0'))
                      .arg((int)currentTime % 60, 2, 10, QChar('0'))
                      .arg((int)totalDuration_ / 60, 2, 10, QChar('0'))
                      .arg((int)totalDuration_ % 60, 2, 10, QChar('0'));
    
    painter.drawText(barRect.adjusted(0, -25, 0, 0), Qt::AlignCenter, timeText);
}

void QtKaraokeDisplay::drawPitchVisualizer(QPainter& painter)
{
    QRect vizRect = pitchRect_;
    
    // Background
    painter.fillRect(vizRect, QColor(15, 20, 35));
    
    // Pitch value (0-100 scale)
    double confidence = std::clamp(currentPitch_ / 1000.0, 0.0, 1.0); // Normalize to 0-1
    int barWidth = (int)(vizRect.width() * confidence);
    
    QColor pitchBarColor;
    if (confidence > 0.8) {
        pitchBarColor = QColor(0, 255, 0); // Green for good pitch
    } else if (confidence > 0.5) {
        pitchBarColor = QColor(255, 255, 0); // Yellow for okay pitch
    } else {
        pitchBarColor = QColor(255, 0, 0); // Red for poor pitch
    }
    
    QRect pitchBar = vizRect;
    pitchBar.setWidth(barWidth);
    painter.fillRect(pitchBar, pitchBarColor);
    
    // Border
    painter.setPen(accentColor_);
    painter.drawRect(vizRect);
    
    // Pitch text
    painter.setFont(smallFont_);
    painter.setPen(primaryTextColor_);
    QString pitchText = QString("%1 Hz").arg((int)currentPitch_);
    painter.drawText(vizRect.adjusted(5, -20, -5, 0), Qt::AlignLeft | Qt::AlignBottom, pitchText);
}

void QtKaraokeDisplay::drawResourcePackInfo(QPainter& painter)
{
    if (resourcePackName_.isEmpty() || resourcePackName_ == "default") return;
    
    painter.setFont(smallFont_);
    painter.setPen(secondaryTextColor_);
    
    QRect infoRect(width() - 200, 10, 190, 30);
    QString packInfo = QString("Theme: %1").arg(resourcePackName_);
    painter.drawText(infoRect, Qt::AlignRight, packInfo);
}

void QtKaraokeDisplay::drawControls(QPainter& painter)
{
    painter.setFont(smallFont_);
    painter.setPen(secondaryTextColor_);
    
    QStringList controls = {
        "Space - Play/Pause",
        "Esc - Stop",
        "Mouse Wheel - Zoom",
        "Click - Seek"
    };
    
    int y = height() - 80;
    for (const QString& control : controls) {
        painter.drawText(10, y, control);
        y += 15;
    }
}

void QtKaraokeDisplay::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Space:
            handlePlayPause();
            break;
        case Qt::Key_Escape:
            handleStop();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

void QtKaraokeDisplay::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        lastMousePos_ = event->pos();
        isDragging_ = true;
        
        // Check if click is in progress bar for seeking
        if (progressRect_.contains(event->pos())) {
            double seekPosition = (double)(event->x() - progressRect_.x()) / progressRect_.width();
            handleSeek(seekPosition);
        }
    }
    QWidget::mousePressEvent(event);
}

void QtKaraokeDisplay::mouseMoveEvent(QMouseEvent* event)
{
    if (isDragging_) {
        QPoint delta = event->pos() - lastMousePos_;
        lastMousePos_ = event->pos();
        
        // Handle dragging for seeking if in progress bar
        if (progressRect_.contains(event->pos())) {
            double seekPosition = (double)(event->x() - progressRect_.x()) / progressRect_.width();
            handleSeek(seekPosition);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void QtKaraokeDisplay::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging_ = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void QtKaraokeDisplay::wheelEvent(QWheelEvent* event)
{
    // Zoom functionality
    double zoomFactor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    zoomLevel_ = std::clamp(zoomLevel_ * zoomFactor, 0.5, 3.0);
    
    // Update font sizes based on zoom
    lyricFont_.setPointSize((int)(48 * zoomLevel_));
    update();
    
    QWidget::wheelEvent(event);
}

void QtKaraokeDisplay::resizeEvent(QResizeEvent* event)
{
    // Update layout rectangles
    int w = width();
    int h = height();
    
    lyricsRect_ = QRect(w * 0.1, h * 0.4, w * 0.8, h * 0.2);
    progressRect_ = QRect(w * 0.2, h * 0.7, w * 0.6, 30);
    pitchRect_ = QRect(w * 0.2, h * 0.8, w * 0.6, 40);
    controlsRect_ = QRect(10, h - 100, 300, 90);
    
    QWidget::resizeEvent(event);
}

void QtKaraokeDisplay::handlePlayPause()
{
    if (application_) {
        // Toggle play/pause state
        isPlaying_ = !isPlaying_;
        // Call application play/pause method here
        update();
    }
}

void QtKaraokeDisplay::handleStop()
{
    if (application_) {
        isPlaying_ = false;
        progress_ = 0.0;
        // Call application stop method here
        update();
    }
}

void QtKaraokeDisplay::handleSeek(double position)
{
    if (application_ && totalDuration_ > 0) {
        position = std::clamp(position, 0.0, 1.0);
        progress_ = position;
        // Call application seek method here
        update();
    }
}

void QtKaraokeDisplay::updateDisplay()
{
    // Update animations and display state
    update();
}

void QtKaraokeDisplay::updatePitch()
{
    // This would typically get pitch data from the application
    if (application_) {
        // Get current pitch from application
        // currentPitch_ = application_->getCurrentPitch();
        update();
    }
}