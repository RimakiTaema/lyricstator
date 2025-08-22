#ifndef QTKEYBINDEDITOR_H
#define QTKEYBINDEDITOR_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "core/Application.h"

class QtKeybindEditor : public QDialog
{
    Q_OBJECT

public:
    explicit QtKeybindEditor(QWidget *parent = nullptr);
    ~QtKeybindEditor();

    void setApplication(Lyricstator::Application* app);

private:
    void setupUI();
    
    Lyricstator::Application* application_;
    QVBoxLayout* mainLayout_;
};

#endif // QTKEYBINDEDITOR_H