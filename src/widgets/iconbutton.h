#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QPushButton>

class IconButton : public QPushButton
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = Q_NULLPTR);

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // ICONBUTTON_H
