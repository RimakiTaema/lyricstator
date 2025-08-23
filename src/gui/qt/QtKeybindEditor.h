#ifndef QTKEYBINDEDITOR_H
#define QTKEYBINDEDITOR_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QScrollArea>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QKeyEvent>
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

#include "core/Application.h"

struct KeybindAction {
    QString name;
    QString description;
    QString category;
    QString defaultKey;
    QString currentKey;
    bool enabled;
    bool editable;
};

struct KeybindProfile {
    QString name;
    QString description;
    QMap<QString, KeybindAction> actions;
    bool isDefault;
};

class QtKeybindEditor : public QDialog
{
    Q_OBJECT

public:
    explicit QtKeybindEditor(QWidget *parent = nullptr);
    ~QtKeybindEditor();

    void setApplication(Lyricstator::Application* app);
    void setKeybind(const QString& actionName, const QString& key);
    QString getKeybind(const QString& actionName) const;
    void resetToDefault();
    void loadProfile(const QString& profileName);
    void saveProfile(const QString& profileName, const QString& description = "");
    void deleteProfile(const QString& profileName);
    QStringList getAvailableProfiles() const;
    void exportKeybinds(const QString& filePath);
    void importKeybinds(const QString& filePath);

public slots:
    void show();
    void hide();
    void toggle();

private slots:
    void onCloseClicked();
    void onResetClicked();
    void onExportClicked();
    void onImportClicked();
    void onProfileSelected();
    void onSaveProfileClicked();
    void onDeleteProfileClicked();
    void onActionItemClicked(QTreeWidgetItem* item, int column);
    void onActionItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onSearchTextChanged();
    void onCategoryFilterChanged();
    void onKeyCaptureStarted();
    void onKeyCaptureFinished();
    void updateAnimation();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void setupUI();
    void createMainPanel();
    void createControlPanel();
    void createKeybindList();
    void createActionTree();
    void setupStyling();
    void setupCallbacks();
    void loadKeybindSettings();
    void saveKeybindSettings();
    void initializeDefaultProfiles();
    void refreshProfileList();
    void refreshActionList();
    void applyProfile(const KeybindProfile& profile);
    void showWithAnimation();
    void hideWithAnimation();
    void startKeyCapture(QTreeWidgetItem* item);
    void stopKeyCapture();
    QString keyEventToString(QKeyEvent* event) const;
    QString keyToString(int key, Qt::KeyboardModifiers modifiers) const;
    void updateActionItem(QTreeWidgetItem* item, const KeybindAction& action);
    void createActionItem(const KeybindAction& action);
    void filterActions();
    void loadProfilesFromFile();
    void saveProfilesToFile();
    void applyModernStyling();

    Lyricstator::Application* application_;
    
    // UI Components
    QWidget* mainPanel_;
    QLabel* titleLabel_;
    QPushButton* closeButton_;
    QPushButton* resetButton_;
    QPushButton* exportButton_;
    QPushButton* importButton_;
    QComboBox* profileComboBox_;
    QPushButton* saveProfileButton_;
    QPushButton* deleteProfileButton_;
    QLineEdit* searchBox_;
    QComboBox* categoryFilter_;
    QTreeWidget* actionTree_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QHBoxLayout* profileLayout_;
    QHBoxLayout* filterLayout_;
    
    // Data
    QMap<QString, KeybindProfile> profiles_;
    QString currentProfile_;
    QVector<KeybindAction> allActions_;
    QVector<KeybindAction> filteredActions_;
    QString currentFilter_;
    
    // Key capture state
    bool isCapturingKey_;
    QTreeWidgetItem* capturingItem_;
    QString capturedKey_;
    
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
    static const QStringList DEFAULT_CATEGORIES;
    static const QMap<QString, QString> DEFAULT_ACTIONS;
};

#endif // QTKEYBINDEDITOR_H