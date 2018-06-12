#ifndef MINIFRAMEBUTTON_H
#define MINIFRAMEBUTTON_H

#include <QPushButton>

class MiniFrameButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MiniFrameButton(const QString &text, QWidget *parent = nullptr);
    ~MiniFrameButton();
};

#endif
