#include "iconbutton.h"

#include <QPainter>

IconButton::IconButton(QWidget *parent)
    : QPushButton (parent)
{
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

//void IconButton::paintEvent(QPaintEvent *event)
//{
//    QPainter painter(this);
//    QPen pen ;
//    pen.setColor(Qt::transparent);
//    painter.setPen(pen);

//    if (m_entered) {

//    } else {

//    }
//    if (m_pressed) {

//    } else {

//    }

//    if (!isChecked()) {
//        painter.setBrush(Qt::transparent);
//    }

//    painter.drawRoundedRect(this->rect(), 9, 9);
//}
