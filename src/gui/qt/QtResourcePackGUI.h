#ifndef QTRESOURCEPACKGUI_H
#define QTRESOURCEPACKGUI_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QScrollArea>
#include <QFrame>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QColor>

#include "core/Application.h"

class QtResourcePackGUI : public QDialog
{
    Q_OBJECT

public:
    explicit QtResourcePackGUI(QWidget *parent = nullptr);
    ~QtResourcePackGUI();

    void setApplication(Lyricstator::Application* app);
    void loadResourcePacks();
    void refreshPacks();

public slots:
    void show();
    void hide();
    void toggle();

private slots:
    void onPackSelected();
    void onApplyTheme();
    void onCloseClicked();
    void updateAnimation();

private:
    void setupUI();
    void setupStyling();
    void createInterface();
    void createPackList();
    void createPreviewPanel();
    void createPackPreview(const QString& packName);
    void loadPackInfo(const QString& packName);
    void applyModernStyling();
    void showWithAnimation();
    void hideWithAnimation();

    Lyricstator::Application* application_;
    
    // UI Components
    QWidget* mainPanel_;
    QLabel* titleLabel_;
    QPushButton* closeButton_;
    QListWidget* packList_;
    QFrame* previewPanel_;
    QScrollArea* previewScrollArea_;
    QPushButton* applyButton_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* contentLayout_;
    QVBoxLayout* packListLayout_;
    QVBoxLayout* previewLayout_;
    
    // Data
    QStringList availablePacks_;
    QString currentPack_;
    QMap<QString, QColor> themeColors_;
    
    // Animation
    QPropertyAnimation* slideAnimation_;
    QPropertyAnimation* fadeAnimation_;
    QParallelAnimationGroup* animationGroup_;
    QTimer* animationTimer_;
    float animationTime_;
    bool isVisible_;
    bool isAnimating_;
};

#endif // QTRESOURCEPACKGUI_H