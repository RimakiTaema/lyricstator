#include "QtEqualizer.h"

QtEqualizer::QtEqualizer(QWidget *parent)
    : QDialog(parent)
    , application_(nullptr)
{
    setWindowTitle("Audio Equalizer");
    setModal(false);
    setupUI();
}

QtEqualizer::~QtEqualizer()
{
}

void QtEqualizer::setApplication(Lyricstator::Application* app)
{
    application_ = app;
}

void QtEqualizer::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    QLabel* label = new QLabel("Equalizer - To be implemented");
    mainLayout_->addWidget(label);
}