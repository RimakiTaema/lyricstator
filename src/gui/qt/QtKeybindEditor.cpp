#include "QtKeybindEditor.h"

QtKeybindEditor::QtKeybindEditor(QWidget *parent)
    : QDialog(parent)
    , application_(nullptr)
{
    setWindowTitle("Keybind Editor");
    setModal(false);
    setupUI();
}

QtKeybindEditor::~QtKeybindEditor()
{
}

void QtKeybindEditor::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtKeybindEditor::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    QLabel* label = new QLabel("Keybind Editor - To be implemented");
    mainLayout_->addWidget(label);
}