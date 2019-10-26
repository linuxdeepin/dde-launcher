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
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool event(QEvent *event) override;

private:
    void updateFont();

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif
