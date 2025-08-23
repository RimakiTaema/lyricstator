#ifndef QTEQUALIZER_H
#define QTEQUALIZER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>
#include <QSettings>
#include <cmath>

#include "core/Application.h"

struct EqualizerBand {
    int index;
    float frequency;
    float gain;
    float qFactor;
    bool enabled;
};

struct EqualizerPreset {
    QString name;
    QString description;
    QVector<float> bandGains;
    QMap<QString, float> parameters;
};

class QtEqualizer : public QDialog
{
    Q_OBJECT

public:
    explicit QtEqualizer(QWidget *parent = nullptr);
    ~QtEqualizer();

    void setApplication(Lyricstator::Application* app);
    void setBandCount(int bandCount);
    void setBandGain(int bandIndex, float gain);
    float getBandGain(int bandIndex) const;
    void setEqualizerEnabled(bool enabled);
    bool isEqualizerEnabled() const;
    void resetToDefault();
    void loadPreset(const QString& presetName);
    void savePreset(const QString& presetName, const QString& description = "");
    void deletePreset(const QString& presetName);
    QStringList getAvailablePresets() const;
    void updateSpectrumData(const QVector<float>& spectrumData);

public slots:
    void show();
    void hide();
    void toggle();

private slots:
    void onEnableToggled();
    void onResetClicked();
    void onPresetSelected();
    void onSavePresetClicked();
    void onCloseClicked();
    void onBandGainChanged(int bandIndex, double value);
    void onSearchTextChanged();
    void updateAnimation();
    void updateSpectrumVisualization();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void createMainPanel();
    void createControlPanel();
    void createSpectrumVisualization();
    void createBandControls();
    void createBandControl(int bandIndex, float frequency);
    void setupStyling();
    void setupCallbacks();
    void loadEqualizerSettings();
    void saveEqualizerSettings();
    void initializeDefaultPresets();
    void refreshPresetList();
    void applyPreset(const EqualizerPreset& preset);
    void showWithAnimation();
    void hideWithAnimation();
    void updateBandColors();
    void updateSpectrumData();
    float getFrequencyForBand(int bandIndex, int totalBands) const;
    QString formatFrequency(float frequency) const;
    QColor getBandColor(int bandIndex, int totalBands) const;
    void applyModernStyling();

    Lyricstator::Application* application_;
    
    // UI Components
    QWidget* mainPanel_;
    QLabel* titleLabel_;
    QPushButton* closeButton_;
    QPushButton* enableButton_;
    QPushButton* resetButton_;
    QComboBox* presetComboBox_;
    QPushButton* savePresetButton_;
    QLineEdit* searchBox_;
    
    // Spectrum visualization
    QFrame* spectrumCanvas_;
    QVector<float> spectrumData_;
    QVector<float> smoothedSpectrum_;
    QTimer* spectrumUpdateTimer_;
    
    // Band controls
    QWidget* bandsContainer_;
    QVector<EqualizerBand> bands_;
    QVector<QWidget*> bandControlWidgets_;
    QVector<QSlider*> bandSliders_;
    QVector<QLabel*> bandLabels_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QGridLayout* bandsLayout_;
    
    // Data
    bool equalizerEnabled_;
    bool showSpectrum_;
    int bandCount_;
    QMap<QString, EqualizerPreset> presets_;
    QString currentPreset_;
    
    // Animation
    QPropertyAnimation* slideAnimation_;
    QPropertyAnimation* fadeAnimation_;
    QParallelAnimationGroup* animationGroup_;
    QTimer* animationTimer_;
    float animationTime_;
    bool isVisible_;
    bool isAnimating_;
    
    // Settings
    QSettings* settings_;
    
    // Constants
    static const int DEFAULT_BAND_COUNT = 10;
    static const float MIN_GAIN = -20.0f;
    static const float MAX_GAIN = 20.0f;
    static const float DEFAULT_Q_FACTOR = 1.0f;
};

#endif // QTEQUALIZER_H