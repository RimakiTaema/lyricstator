#ifndef QTHELPSYSTEM_H
#define QTHELPSYSTEM_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QSplitter>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
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

struct HelpTopic {
    QString title;
    QString content;
    QString category;
    QString keywords;
    int priority;
    bool isVisible;
};

struct HelpCategory {
    QString name;
    QString description;
    QString icon;
    QVector<HelpTopic> topics;
    bool isExpanded;
};

class QtHelpSystem : public QDialog
{
    Q_OBJECT

public:
    explicit QtHelpSystem(QWidget *parent = nullptr);
    ~QtHelpSystem();

    void setApplication(Lyricstator::Application* app);
    void showTopic(const QString& topicTitle);
    void searchTopics(const QString& searchText);
    void showCategory(const QString& categoryName);
    void addTopic(const QString& title, const QString& content, const QString& category);
    void removeTopic(const QString& title);
    void updateTopic(const QString& title, const QString& newContent);
    void exportHelp(const QString& filePath);
    void importHelp(const QString& filePath);
    void checkForUpdates();
    void showAboutDialog();
    void showLicenseDialog();
    void showChangelog();

public slots:
    void show();
    void hide();
    void toggle();
    void refreshHelp();

private slots:
    void onCloseClicked();
    void onSearchClicked();
    void onHomeClicked();
    void onBackClicked();
    void onForwardClicked();
    void onPrintClicked();
    void onExportClicked();
    void onAboutClicked();
    void onLicenseClicked();
    void onChangelogClicked();
    void onTopicSelectionChanged();
    void onCategorySelectionChanged();
    void onSearchTextChanged();
    void onFilterChanged();
    void onHelpContentChanged();
    void updateAnimation();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void createMainPanel();
    void createToolbar();
    void createHelpBrowser();
    void createTopicTree();
    void createSearchPanel();
    void createControlPanel();
    void setupStyling();
    void setupCallbacks();
    void loadHelpContent();
    void saveHelpContent();
    void initializeDefaultHelp();
    void setupTopicTree();
    void updateTopicDisplay();
    void updateCategoryDisplay();
    void showWithAnimation();
    void hideWithAnimation();
    void applyModernStyling();
    void refreshHelpDisplay();
    void createContextMenu(const QPoint& pos);
    void loadHelpFromFile();
    void saveHelpToFile();
    void searchInTopics(const QString& searchText);
    void filterTopics(const QString& filterText, const QString& categoryFilter);

    Lyricstator::Application* application_;
    
    // UI Components
    QWidget* mainPanel_;
    QLabel* titleLabel_;
    QPushButton* closeButton_;
    QPushButton* searchButton_;
    QPushButton* homeButton_;
    QPushButton* backButton_;
    QPushButton* forwardButton_;
    QPushButton* printButton_;
    QPushButton* exportButton_;
    QPushButton* aboutButton_;
    QPushButton* licenseButton_;
    QPushButton* changelogButton_;
    
    // Help browser
    QSplitter* mainSplitter_;
    QTreeWidget* topicTree_;
    QTextBrowser* helpBrowser_;
    
    // Search panel
    QLineEdit* searchBox_;
    QComboBox* categoryFilter_;
    QPushButton* searchButton_;
    
    // Control panel
    QGroupBox* controlGroup_;
    QPushButton* refreshButton_;
    QPushButton* updateButton_;
    QPushButton* helpButton_;
    
    // Layouts
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QGridLayout* searchLayout_;
    QGridLayout* controlPanelLayout_;
    
    // Data
    QMap<QString, HelpTopic> allTopics_;
    QMap<QString, HelpCategory> categories_;
    QVector<QString> topicOrder_;
    QString currentTopic_;
    QString currentCategory_;
    QString currentFilter_;
    QString searchHistory_;
    QVector<QString> visitedTopics_;
    int currentHistoryIndex_;
    
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
    static const QStringList SUPPORTED_CATEGORIES;
    static const QStringList SUPPORTED_FORMATS;
    static const int DEFAULT_PRIORITY = 5;
    static const int HELP_WINDOW_WIDTH = 900;
    static const int HELP_WINDOW_HEIGHT = 700;
    static const int TOPIC_TREE_WIDTH = 250;
};

#endif // QTHELPSYSTEM_H