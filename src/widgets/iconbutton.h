#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QPushButton>

class IconButton : public QPushButton
{
    Q_OBJECT
public:
    IconButton(QWidget *parent = Q_NULLPTR);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
//    void paintEvent(QPaintEvent *event);

private:
    bool m_pressed; // press: true ,release : false
    bool m_entered; // entered: true, leaved: false
};

#endif // ICONBUTTON_H
