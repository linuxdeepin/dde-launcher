#include "iconbutton.h"

#include <QPainter>

IconButton::IconButton(QWidget *parent)
    : QPushButton (parent)
{
    setAccessibleName("modeSwitchButton");
    setCheckable(true);
    setChecked(false);
}

void IconButton::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    QPushButton::mousePressEvent(event);
}

void IconButton::mouseReaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    QPushButton::mouseReleaseEvent(event);
}

void IconButton::enterEvent(QEvent *event)
{
    m_entered = true;
    QPushButton::enterEvent(event);
}

void IconButton::leaveEvent(QEvent *event)
{
    m_entered = false;
    QPushButton::leaveEvent(event);
}
