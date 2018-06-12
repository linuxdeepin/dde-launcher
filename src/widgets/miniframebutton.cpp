#include "miniframebutton.h"

MiniFrameButton::MiniFrameButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("MiniFrameButton");
}

MiniFrameButton::~MiniFrameButton()
{
}
