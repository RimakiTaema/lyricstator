#ifndef QTEQUALIZER_H
#define QTEQUALIZER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

#include "core/Application.h"

class QtEqualizer : public QDialog
{
    Q_OBJECT

public:
    explicit QtEqualizer(QWidget *parent = nullptr);
    ~QtEqualizer();

    void setApplication(Lyricstator::Application* app);

private:
    void setupUI();
    
    Lyricstator::Application* application_;
    QVBoxLayout* mainLayout_;
};

#endif // QTEQUALIZER_H