#ifndef QTSETTINGS_H
#define QTSETTINGS_H

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
#include <QTabWidget>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QTextEdit>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
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

struct ApplicationSetting {
    QString name;
    QString description;
    QString category;
    QString type; // "bool", "int", "string", "color", "font", "file"
    QVariant defaultValue;
    QVariant currentValue;
    bool isAdvanced;
    bool isHidden;
    QString group;
};

struct SettingsCategory {
    QString name;
    QString description;
    QString icon;
    QVector<ApplicationSetting> settings;
    bool isExpanded;
};

class QtSettings : public QDialog
{
    Q_OBJECT

public:
    explicit QtSettings(QWidget *parent = nullptr);
    ~QtSettings();

    void setApplication(Lyricstator::Application* app);
    void setSetting(const QString& settingName, const QVariant& value);
    QVariant getSetting(const QString& settingName) const;
    void resetSetting(const QString& settingName);
    void resetAllSettings();
    void resetCategory(const QString& categoryName);
    void exportSettings(const QString& filePath);
    void importSettings(const QString& filePath);
    void createSetting(const QString& name, const QString& description, const QString& category, 
                      const QString& type, const QVariant& defaultValue);
    void deleteSetting(const QString& settingName);
    void renameSetting(const QString& oldName, const QString& newName);
    void moveSetting(const QString& settingName, const QString& newCategory);
    void validateSettings();
    void applySettings();
    void saveSettings();
    void loadSettings();

public slots:
    void show();
    void hide();
    void toggle();
    void refreshSettings();

private slots:
    void onCloseClicked();
    void onApplyClicked();
    void onResetClicked();
    void onExportClicked();
    void onImportClicked();
    void onSaveClicked();
    void onCancelClicked();
    void onCategoryChanged();
    void onSettingChanged();
    void onBoolSettingToggled(bool checked);
    void onIntSettingChanged(int value);
    void onStringSettingChanged(const QString& text);
    void onColorSettingClicked();
    void onFontSettingClicked();
    void onFileSettingClicked();
    void onAdvancedToggled();
    void onSearchTextChanged();
    void onFilterChanged();
    void onResetAllClicked();
    void onResetCategoryClicked();
    void onCreateSettingClicked();
    void onDeleteSettingClicked();
    void onRenameSettingClicked();
    void onMoveSettingClicked();
    void onValidateClicked();
    void onHelpClicked();
    void onAboutClicked();
    void updateAnimation();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setupUI();
    void createMainPanel();
    void createTabWidget();
    void createGeneralTab();
    void createAudioTab();
    void createDisplayTab();
    void createInterfaceTab();
    void createAdvancedTab();
    void createControlPanel();
    void setupStyling();
    void setupCallbacks();
    void loadSettingsFromFile();
    void saveSettingsToFile();
    void initializeDefaultSettings();
    void setupSettingsTree();
    void setupSettingsTable();
    void updateSettingsDisplay();
    void updateCategoryDisplay();
    void showWithAnimation();
    void hideWithAnimation();
    void applyModernStyling();
    void refreshSettingsDisplay();
    void createContextMenu(const QPoint& pos);
    void applySettingToUI(const QString& settingName, const QVariant& value);
    void updateSettingValue(const QString& settingName, const QVariant& value);
    void validateSetting(const QString& settingName);
    void checkSettingConflicts();
    void createSettingWidget(const ApplicationSetting& setting);
    void updateSettingWidget(const QString& settingName);
    void filterSettings(const QString& filterText, const QString& categoryFilter);

    Lyricstator::Application* application_;
    
    // UI Components
    QWidget* mainPanel_;
    QLabel* titleLabel_;
    QPushButton* closeButton_;
    QPushButton* applyButton_;
    QPushButton* resetButton_;
    QPushButton* exportButton_;
    QPushButton* importButton_;
    QPushButton* saveButton_;
    QPushButton* cancelButton_;
    
    // Tab widget
    QTabWidget* tabWidget_;
    QWidget* generalTab_;
    QWidget* audioTab_;
    QWidget* displayTab_;
    QWidget* interfaceTab_;
    QWidget* advancedTab_;
    
    // General tab
    QGroupBox* generalGroup_;
    QCheckBox* autoSaveCheckBox_;
    QSpinBox* autoSaveIntervalSpinBox_;
    QCheckBox* checkUpdatesCheckBox_;
    QComboBox* languageComboBox_;
    QComboBox* themeComboBox_;
    QCheckBox* startMinimizedCheckBox_;
    QCheckBox* showSplashCheckBox_;
    
    // Audio tab
    QGroupBox* audioGroup_;
    QComboBox* audioDeviceComboBox_;
    QComboBox* sampleRateComboBox_;
    QComboBox* channelsComboBox_;
    QSpinBox* bufferSizeSpinBox_;
    QSlider* volumeSlider_;
    QLabel* volumeLabel_;
    QCheckBox* enableEqualizerCheckBox_;
    QCheckBox* enableEffectsCheckBox_;
    QCheckBox* enableSpatialAudioCheckBox_;
    
    // Display tab
    QGroupBox* displayGroup_;
    QComboBox* resolutionComboBox_;
    QComboBox* refreshRateComboBox_;
    QCheckBox* fullscreenCheckBox_;
    QCheckBox* vsyncCheckBox_;
    QCheckBox* antialiasingCheckBox_;
    QComboBox* antialiasingLevelComboBox_;
    QCheckBox* anisotropicFilteringCheckBox_;
    QSlider* brightnessSlider_;
    QLabel* brightnessLabel_;
    QSlider* contrastSlider_;
    QLabel* contrastLabel_;
    QSlider* saturationSlider_;
    QLabel* saturationLabel_;
    
    // Interface tab
    QGroupBox* interfaceGroup_;
    QCheckBox* showToolbarCheckBox_;
    QCheckBox* showStatusBarCheckBox_;
    QCheckBox* showMenuBarCheckBox_;
    QCheckBox* showContextMenusCheckBox_;
    QCheckBox* enableAnimationsCheckBox_;
    QCheckBox* enableTransparencyCheckBox_;
    QCheckBox* enableShadowsCheckBox_;
    QSpinBox* animationSpeedSpinBox_;
    QComboBox* iconThemeComboBox_;
    QComboBox* fontComboBox_;
    QSpinBox* fontSizeSpinBox_;
    QPushButton* fontColorButton_;
    QPushButton* backgroundColorButton_;
    QPushButton* accentColorButton_;
    
    // Advanced tab
    QGroupBox* advancedGroup_;
    QCheckBox* showAdvancedCheckBox_;
    QCheckBox* enableLoggingCheckBox_;
    QComboBox* logLevelComboBox_;
    QLineEdit* logFilePathEdit_;
    QPushButton* browseLogPathButton_;
    QCheckBox* enableDebugModeCheckBox_;
    QCheckBox* enableProfilingCheckBox_;
    QCheckBox* enableTelemetryCheckBox_;
    QSpinBox* maxMemoryUsageSpinBox_;
    QSpinBox* maxThreadsSpinBox_;
    QCheckBox* enableHardwareAccelerationCheckBox_;
    QComboBox* rendererComboBox_;
    
    // Settings tree/table
    QGroupBox* settingsGroup_;
    QTreeWidget* settingsTree_;
    QTableWidget* settingsTable_;
    QPushButton* createSettingButton_;
    QPushButton* deleteSettingButton_;
    QPushButton* renameSettingButton_;
    QPushButton* moveSettingButton_;
    QPushButton* validateButton_;
    
    // Search and filter
    QLineEdit* searchBox_;
    QComboBox* categoryFilter_;
    QCheckBox* showAdvancedSettingsCheckBox_;
    QCheckBox* showHiddenSettingsCheckBox_;
    
    // Control panel
    QGroupBox* controlGroup_;
    QPushButton* resetAllButton_;
    QPushButton* resetCategoryButton_;
    QPushButton* helpButton_;
    QPushButton* aboutButton_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QGridLayout* generalLayout_;
    QGridLayout* audioLayout_;
    QGridLayout* displayLayout_;
    QGridLayout* interfaceLayout_;
    QGridLayout* advancedLayout_;
    QGridLayout* settingsLayout_;
    QGridLayout* controlPanelLayout_;
    
    // Data
    QMap<QString, ApplicationSetting> allSettings_;
    QMap<QString, SettingsCategory> categories_;
    QVector<QString> settingOrder_;
    QString currentCategory_;
    QString currentFilter_;
    QVector<QString> modifiedSettings_;
    QVector<QString> conflictedSettings_;
    bool settingsModified_;
    bool showAdvancedSettings_;
    bool showHiddenSettings_;
    
    // Settings widgets mapping
    QMap<QString, QWidget*> settingWidgets_;
    QMap<QString, QVariant> originalValues_;
    
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
    static const QStringList SUPPORTED_LANGUAGES;
    static const QStringList SUPPORTED_THEMES;
    static const QStringList SUPPORTED_ICON_THEMES;
    static const QStringList SUPPORTED_FONTS;
    static const QVector<int> SUPPORTED_FONT_SIZES;
    static const QStringList SUPPORTED_RESOLUTIONS;
    static const QVector<int> SUPPORTED_REFRESH_RATES;
    static const QStringList SUPPORTED_ANTIALIASING_LEVELS;
    static const QStringList SUPPORTED_RENDERERS;
    static const QStringList SUPPORTED_LOG_LEVELS;
    static const int DEFAULT_FONT_SIZE = 12;
    static const int DEFAULT_ANIMATION_SPEED = 100;
    static const int DEFAULT_MAX_MEMORY_USAGE = 1024;
    static const int DEFAULT_MAX_THREADS = 4;
};

#endif // QTSETTINGS_H