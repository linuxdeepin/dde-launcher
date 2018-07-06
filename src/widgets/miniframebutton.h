#ifndef MINIFRAMEBUTTON_H
#define MINIFRAMEBUTTON_H

#include <QPushButton>

class MiniFrameButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MiniFrameButton(const QString &text, QWidget *parent = nullptr);
    ~MiniFrameButton();

signals:
    void entered() const;

protected:
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
};

#endif
