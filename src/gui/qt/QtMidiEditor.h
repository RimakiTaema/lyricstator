#ifndef QTMIDIEDITOR_H
#define QTMIDIIDITOR_H

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
#include <QScrollArea>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
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

#include "core/Application.h"

struct MidiNote {
    int note;
    int velocity;
    qint64 startTime;
    qint64 duration;
    int channel;
    QString noteName;
    bool isSelected;
    bool isVisible;
};

struct MidiTrack {
    QString name;
    int channel;
    bool isMuted;
    bool isSolo;
    int volume;
    int pan;
    QVector<MidiNote> notes;
    QColor color;
    bool isVisible;
};

struct MidiEvent {
    qint64 time;
    int type; // Note On/Off, Control Change, etc.
    int channel;
    int data1;
    int data2;
    QString description;
};

class QtMidiEditor : public QDialog
{
    Q_OBJECT

public:
    explicit QtMidiEditor(QWidget *parent = nullptr);
    ~QtMidiEditor();

    void setApplication(Lyricstator::Application* app);
    void loadMidiFile(const QString& filePath);
    void saveMidiFile(const QString& filePath);
    void setTempo(int bpm);
    int getTempo() const;
    void setTimeSignature(int numerator, int denominator);
    void getTimeSignature(int& numerator, int& denominator) const;
    void setQuantization(int ticks);
    int getQuantization() const;
    void addNote(int note, int velocity, qint64 startTime, qint64 duration, int channel);
    void removeNote(int noteIndex, int channel);
    void modifyNote(int noteIndex, int channel, const MidiNote& newNote);
    void selectNote(int noteIndex, int channel);
    void selectNotesInRange(qint64 startTime, qint64 endTime, int minNote, int maxNote);
    void clearSelection();
    void copySelection();
    void pasteSelection(qint64 timeOffset);
    void deleteSelection();
    void quantizeSelection();
    void transposeSelection(int semitones);
    void setTrackMuted(int trackIndex, bool muted);
    void setTrackSolo(int trackIndex, bool solo);
    void setTrackVolume(int trackIndex, int volume);
    void setTrackPan(int trackIndex, int pan);
    void addTrack(const QString& name, int channel);
    void removeTrack(int trackIndex);
    void renameTrack(int trackIndex, const QString& name);
    void exportMidi(const QString& filePath);
    void importMidi(const QString& filePath);

public slots:
    void show();
    void hide();
    void toggle();
    void play();
    void pause();
    void stop();
    void record();

private slots:
    void onCloseClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onExportClicked();
    void onImportClicked();
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onRecordClicked();
    void onTempoChanged();
    void onTimeSignatureChanged();
    void onQuantizationChanged();
    void onTrackSelectionChanged();
    void onNoteSelectionChanged();
    void onZoomChanged();
    void onGridToggled();
    void onSnapToggled();
    void onShowVelocityToggled();
    void onShowPitchBendToggled();
    void onShowControlChangesToggled();
    void onTrackMuteToggled();
    void onTrackSoloToggled();
    void onTrackVolumeChanged();
    void onTrackPanChanged();
    void onAddTrackClicked();
    void onRemoveTrackClicked();
    void onRenameTrackClicked();
    void onQuantizeClicked();
    void onTransposeClicked();
    void onCopyClicked();
    void onPasteClicked();
    void onDeleteClicked();
    void onUndoClicked();
    void onRedoClicked();
    void updateAnimation();
    void updatePlaybackPosition();

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
    void createTrackList();
    void createPianoRoll();
    void createControlPanel();
    void createTimeline();
    void createTransportControls();
    void setupStyling();
    void setupCallbacks();
    void loadMidiSettings();
    void saveMidiSettings();
    void initializeDefaultSettings();
    void setupGraphicsScene();
    void setupTimeline();
    void setupPianoRoll();
    void updateTimeline();
    void updatePianoRoll();
    void drawGrid();
    void drawNotes();
    void drawTracks();
    void drawTimeline();
    void drawPianoKeys();
    void showWithAnimation();
    void hideWithAnimation();
    void applyModernStyling();
    void updatePlaybackDisplay();
    void handleNoteClick(const QPointF& pos);
    void handleNoteDrag(const QPointF& pos);
    void handleNoteRelease();
    void updateNoteSelection();
    void updateTrackSelection();
    void refreshDisplay();
    void createContextMenu(const QPoint& pos);

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
    QAction* playAction_;
    QAction* pauseAction_;
    QAction* stopAction_;
    QAction* recordAction_;
    QAction* undoAction_;
    QAction* redoAction_;
    
    // Transport controls
    QGroupBox* transportGroup_;
    QPushButton* playButton_;
    QPushButton* pauseButton_;
    QPushButton* stopButton_;
    QPushButton* recordButton_;
    QSlider* positionSlider_;
    QLabel* positionLabel_;
    QLabel* timeLabel_;
    QLabel* tempoLabel_;
    QSpinBox* tempoSpinBox_;
    QLabel* timeSignatureLabel_;
    QSpinBox* numeratorSpinBox_;
    QSpinBox* denominatorSpinBox_;
    
    // Track list
    QGroupBox* trackGroup_;
    QListWidget* trackList_;
    QPushButton* addTrackButton_;
    QPushButton* removeTrackButton_;
    QPushButton* renameTrackButton_;
    
    // Piano roll
    QGroupBox* pianoRollGroup_;
    QGraphicsView* pianoRollView_;
    QGraphicsScene* pianoRollScene_;
    QScrollArea* pianoRollScroll_;
    
    // Control panel
    QGroupBox* controlGroup_;
    QCheckBox* gridCheckBox_;
    QCheckBox* snapCheckBox_;
    QCheckBox* showVelocityCheckBox_;
    QCheckBox* showPitchBendCheckBox_;
    QCheckBox* showControlChangesCheckBox_;
    QSlider* zoomSlider_;
    QLabel* zoomLabel_;
    QSpinBox* quantizationSpinBox_;
    QLabel* quantizationLabel_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QGridLayout* transportLayout_;
    QGridLayout* trackLayout_;
    QGridLayout* pianoRollLayout_;
    QGridLayout* controlPanelLayout_;
    
    // Data
    QVector<MidiTrack> tracks_;
    QVector<MidiEvent> events_;
    QVector<MidiNote> selectedNotes_;
    QVector<int> selectedTracks_;
    QString midiFilePath_;
    int tempo_;
    int timeSignatureNumerator_;
    int timeSignatureDenominator_;
    int quantization_;
    qint64 totalDuration_;
    qint64 currentPosition_;
    bool isPlaying_;
    bool isRecording_;
    bool showGrid_;
    bool snapToGrid_;
    bool showVelocity_;
    bool showPitchBend_;
    bool showControlChanges_;
    float zoomLevel_;
    qint64 viewStartTime_;
    qint64 viewEndTime_;
    int viewStartNote_;
    int viewEndNote_;
    
    // Graphics items
    QGraphicsRectItem* timelineItem_;
    QGraphicsRectItem* pianoKeysItem_;
    QVector<QGraphicsRectItem*> noteItems_;
    QVector<QGraphicsTextItem*> textItems_;
    QVector<QGraphicsLineItem*> gridLines_;
    
    // Mouse interaction
    bool isDragging_;
    QPointF dragStartPos_;
    QPointF lastMousePos_;
    int selectedNoteIndex_;
    int selectedTrackIndex_;
    
    // Animation
    QPropertyAnimation* slideAnimation_;
    QPropertyAnimation* fadeAnimation_;
    QParallelAnimationGroup* animationGroup_;
    QTimer* animationTimer_;
    QTimer* playbackTimer_;
    float animationTime_;
    bool isVisible_;
    bool isAnimating_;
    
    // Settings
    QSettings* settings_;
    
    // Constants
    static const int DEFAULT_TEMPO = 120;
    static const int DEFAULT_NUMERATOR = 4;
    static const int DEFAULT_DENOMINATOR = 4;
    static const int DEFAULT_QUANTIZATION = 16;
    static const float MIN_ZOOM = 0.1f;
    static const float MAX_ZOOM = 10.0f;
    static const int PIANO_ROLL_HEIGHT = 600;
    static const int TIMELINE_HEIGHT = 100;
    static const int TRACK_LIST_WIDTH = 200;
};

#endif // QTMIDIEDITOR_H