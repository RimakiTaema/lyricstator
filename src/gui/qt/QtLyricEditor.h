#ifndef QTLYRICEDITOR_H
#define QTLYRICEDITOR_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QFontDialog>
#include <QColorDialog>

#include "core/Application.h"

struct LyricLine {
    QString text;
    qint64 startTime;
    qint64 endTime;
    int lineNumber;
    bool isSelected;
    bool isHighlighted;
    QString fontFamily;
    int fontSize;
    QColor textColor;
    QColor backgroundColor;
    QColor outlineColor;
    int outlineWidth;
    bool isBold;
    bool isItalic;
    bool isUnderlined;
    QString alignment;
    QPointF position;
    QSizeF size;
};

struct LyricStyle {
    QString name;
    QString description;
    QString fontFamily;
    int fontSize;
    QColor textColor;
    QColor backgroundColor;
    QColor outlineColor;
    int outlineWidth;
    bool isBold;
    bool isItalic;
    bool isUnderlined;
    QString alignment;
    QPointF position;
    QSizeF size;
    bool isDefault;
};

struct LyricTiming {
    qint64 time;
    QString text;
    int lineNumber;
    bool isSynchronized;
    float confidence;
};

class QtLyricEditor : public QDialog
{
    Q_OBJECT

public:
    explicit QtLyricEditor(QWidget *parent = nullptr);
    ~QtLyricEditor();

    void setApplication(Lyricstator::Application* app);
    void loadLyricFile(const QString& filePath);
    void saveLyricFile(const QString& filePath);
    void exportLyricFile(const QString& filePath, const QString& format);
    void importLyricFile(const QString& filePath, const QString& format);
    void addLyricLine(const QString& text, qint64 startTime, qint64 endTime);
    void removeLyricLine(int lineIndex);
    void modifyLyricLine(int lineIndex, const LyricLine& newLine);
    void selectLyricLine(int lineIndex);
    void selectLyricLinesInRange(qint64 startTime, qint64 endTime);
    void clearSelection();
    void copySelection();
    void pasteSelection(qint64 timeOffset);
    void deleteSelection();
    void duplicateSelection();
    void moveSelection(qint64 timeOffset);
    void setLyricStyle(int lineIndex, const LyricStyle& style);
    void applyStyleToSelection(const LyricStyle& style);
    void createStyle(const QString& name, const LyricStyle& style);
    void deleteStyle(const QString& name);
    void renameStyle(const QString& oldName, const QString& newName);
    void synchronizeWithAudio(const QString& audioFilePath);
    void autoSyncLyrics();
    void manualSyncLyrics();
    void validateLyrics();
    void spellCheck();
    void findAndReplace(const QString& findText, const QString& replaceText, bool caseSensitive);
    void undo();
    void redo();
    void resetToDefault();

public slots:
    void show();
    void hide();
    void toggle();
    void previewLyrics();
    void playPreview();
    void stopPreview();

private slots:
    void onCloseClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onExportClicked();
    void onImportClicked();
    void onPreviewClicked();
    void onPlayPreviewClicked();
    void onStopPreviewClicked();
    void onAddLineClicked();
    void onRemoveLineClicked();
    void onDuplicateLineClicked();
    void onMoveLineUpClicked();
    void onMoveLineDownClicked();
    void onStyleChanged();
    void onFontChanged();
    void onColorChanged();
    void onAlignmentChanged();
    void onBoldToggled();
    void onItalicToggled();
    void onUnderlineToggled();
    void onOutlineToggled();
    void onTimeChanged();
    void onTextChanged();
    void onLineSelectionChanged();
    void onStyleSelectionChanged();
    void onCreateStyleClicked();
    void onDeleteStyleClicked();
    void onRenameStyleClicked();
    void onApplyStyleClicked();
    void onSynchronizeClicked();
    void onAutoSyncClicked();
    void onManualSyncClicked();
    void onValidateClicked();
    void onSpellCheckClicked();
    void onFindReplaceClicked();
    void onUndoClicked();
    void onRedoClicked();
    void onResetClicked();
    void onZoomChanged();
    void onGridToggled();
    void onSnapToggled();
    void onShowTimingToggled();
    void onShowStylesToggled();
    void updateAnimation();
    void updatePreview();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUI();
    void createMainPanel();
    void createToolbar();
    void createLyricEditor();
    void createStylePanel();
    void createTimingPanel();
    void createPreviewPanel();
    void createControlPanel();
    void setupStyling();
    void setupCallbacks();
    void loadLyricSettings();
    void saveLyricSettings();
    void initializeDefaultSettings();
    void setupLyricTable();
    void setupStyleList();
    void setupTimingView();
    void setupPreviewView();
    void updateLyricTable();
    void updateStyleList();
    void updateTimingView();
    void updatePreviewView();
    void drawLyricPreview();
    void drawTimingGrid();
    void showWithAnimation();
    void hideWithAnimation();
    void applyModernStyling();
    void updatePreviewDisplay();
    void handleLineClick(const QPoint& pos);
    void handleLineDrag(const QPoint& pos);
    void handleLineRelease();
    void updateLineSelection();
    void updateStyleSelection();
    void refreshDisplay();
    void createContextMenu(const QPoint& pos);
    void applyStyleToLine(int lineIndex, const LyricStyle& style);
    void updateLineStyle(int lineIndex);
    void validateLine(int lineIndex);
    void checkSpelling();
    void findText(const QString& text, bool caseSensitive);
    void replaceText(const QString& findText, const QString& replaceText, bool caseSensitive);

    Lyricstator::Application* application_;
    
    // UI Components
    QWidget* mainPanel_;
    QLabel* titleLabel_;
    QPushButton* closeButton_;
    QPushButton* saveButton_;
    QPushButton* loadButton_;
    QPushButton* exportButton_;
    QPushButton* importButton_;
    
    // Toolbar
    QToolBar* toolbar_;
    QAction* previewAction_;
    QAction* playPreviewAction_;
    QAction* stopPreviewAction_;
    QAction* undoAction_;
    QAction* redoAction_;
    
    // Main controls
    QPushButton* previewButton_;
    QPushButton* playPreviewButton_;
    QPushButton* stopPreviewButton_;
    QPushButton* addLineButton_;
    QPushButton* removeLineButton_;
    QPushButton* duplicateLineButton_;
    QPushButton* moveUpButton_;
    QPushButton* moveDownButton_;
    
    // Lyric editor
    QGroupBox* lyricGroup_;
    QTableWidget* lyricTable_;
    QTextEdit* lyricTextEdit_;
    QLineEdit* startTimeEdit_;
    QLineEdit* endTimeEdit_;
    QSpinBox* lineNumberSpinBox_;
    
    // Style panel
    QGroupBox* styleGroup_;
    QListWidget* styleList_;
    QPushButton* createStyleButton_;
    QPushButton* deleteStyleButton_;
    QPushButton* renameStyleButton_;
    QPushButton* applyStyleButton_;
    
    // Style controls
    QComboBox* fontComboBox_;
    QSpinBox* fontSizeSpinBox_;
    QPushButton* fontColorButton_;
    QPushButton* backgroundColorButton_;
    QPushButton* outlineColorButton_;
    QSpinBox* outlineWidthSpinBox_;
    QCheckBox* boldCheckBox_;
    QCheckBox* italicCheckBox_;
    QCheckBox* underlineCheckBox_;
    QCheckBox* outlineCheckBox_;
    QComboBox* alignmentComboBox_;
    
    // Timing panel
    QGroupBox* timingGroup_;
    QTableWidget* timingTable_;
    QPushButton* synchronizeButton_;
    QPushButton* autoSyncButton_;
    QPushButton* manualSyncButton_;
    QPushButton* validateButton_;
    
    // Preview panel
    QGroupBox* previewGroup_;
    QLabel* previewLabel_;
    QSlider* previewSlider_;
    QLabel* previewTimeLabel_;
    
    // Control panel
    QGroupBox* controlGroup_;
    QCheckBox* gridCheckBox_;
    QCheckBox* snapCheckBox_;
    QCheckBox* showTimingCheckBox_;
    QCheckBox* showStylesCheckBox_;
    QSlider* zoomSlider_;
    QLabel* zoomLabel_;
    QPushButton* spellCheckButton_;
    QPushButton* findReplaceButton_;
    QPushButton* resetButton_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QGridLayout* lyricLayout_;
    QGridLayout* styleLayout_;
    QGridLayout* timingLayout_;
    QGridLayout* previewLayout_;
    QGridLayout* controlPanelLayout_;
    
    // Data
    QVector<LyricLine> lyricLines_;
    QMap<QString, LyricStyle> styles_;
    QVector<LyricTiming> timings_;
    QString lyricFilePath_;
    QString currentStyle_;
    QVector<int> selectedLines_;
    QString findText_;
    QString replaceText_;
    bool caseSensitive_;
    bool showGrid_;
    bool snapToGrid_;
    bool showTiming_;
    bool showStyles_;
    float zoomLevel_;
    qint64 previewPosition_;
    bool isPreviewing_;
    bool isPlayingPreview_;
    
    // Text editing
    QFont currentFont_;
    QColor currentTextColor_;
    QColor currentBackgroundColor_;
    QColor currentOutlineColor_;
    int currentOutlineWidth_;
    bool currentBold_;
    bool currentItalic_;
    bool currentUnderline_;
    bool currentOutline_;
    QString currentAlignment_;
    
    // Mouse interaction
    bool isDragging_;
    QPoint dragStartPos_;
    QPoint lastMousePos_;
    int selectedLineIndex_;
    int selectedStyleIndex_;
    
    // Animation
    QPropertyAnimation* slideAnimation_;
    QPropertyAnimation* fadeAnimation_;
    QParallelAnimationGroup* animationGroup_;
    QTimer* animationTimer_;
    QTimer* previewTimer_;
    float animationTime_;
    bool isVisible_;
    bool isAnimating_;
    
    // Settings
    QSettings* settings_;
    
    // Constants
    static const QStringList SUPPORTED_FORMATS;
    static const QStringList SUPPORTED_FONTS;
    static const QVector<int> SUPPORTED_FONT_SIZES;
    static const QStringList SUPPORTED_ALIGNMENTS;
    static const float MIN_ZOOM = 0.1f;
    static const float MAX_ZOOM = 10.0f;
    static const int DEFAULT_FONT_SIZE = 24;
    static const int DEFAULT_OUTLINE_WIDTH = 2;
    static const int LYRIC_EDITOR_HEIGHT = 400;
    static const int STYLE_PANEL_WIDTH = 250;
    static const int TIMING_PANEL_WIDTH = 300;
    static const int PREVIEW_PANEL_HEIGHT = 200;
};

#endif // QTLYRICEDITOR_H