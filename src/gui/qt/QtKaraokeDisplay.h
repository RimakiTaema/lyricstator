#ifndef QTKARAOKEDISPLAY_H
#define QTKARAOKEDISPLAY_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFont>
#include <QColor>
#include <QString>
#include <QList>
#include <QMap>
#include <QTime>
#include <QElapsedTimer>

#include "core/Application.h"

class QtKaraokeDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit QtKaraokeDisplay(QWidget *parent = nullptr);
    ~QtKaraokeDisplay();

    void setApplication(Lyricstator::Application* app);
    void setCurrentLyric(const QString& lyric, double timestamp);
    void setCurrentPitch(double pitch);
    void setProgress(double progress);
    void setTotalDuration(double duration);
    void setResourcePack(const QString& packName);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateDisplay();
    void updatePitch();

private:
    void setupUI();
    void setupTimers();
    void connectSignals();
    void drawBackground(QPainter& painter);
    void drawLyrics(QPainter& painter);
    void drawProgressBar(QPainter& painter);
    void drawPitchVisualizer(QPainter& painter);
    void drawResourcePackInfo(QPainter& painter);
    void drawControls(QPainter& painter);
    void updateColors();
    void updateFonts();
    void handlePlayPause();
    void handleStop();
    void handleSeek(double position);

    Lyricstator::Application* application_;
    
    // Display state
    QString currentLyric_;
    double currentTimestamp_;
    double currentPitch_;
    double progress_;
    double totalDuration_;
    QString resourcePackName_;
    
    // UI state
    bool isPlaying_;
    bool isDragging_;
    QPoint lastMousePos_;
    double zoomLevel_;
    
    // Colors
    QColor backgroundColor_;
    QColor primaryTextColor_;
    QColor secondaryTextColor_;
    QColor accentColor_;
    QColor progressColor_;
    QColor pitchColor_;
    
    // Fonts
    QFont titleFont_;
    QFont lyricFont_;
    QFont infoFont_;
    QFont smallFont_;
    
    // Timers
    QTimer* displayTimer_;
    QTimer* pitchTimer_;
    
    // Layout
    QRect lyricsRect_;
    QRect progressRect_;
    QRect pitchRect_;
    QRect controlsRect_;
    
    // Resource pack colors (if available)
    QMap<QString, QColor> resourcePackColors_;
};

#endif // QTKARAOKEDISPLAY_H