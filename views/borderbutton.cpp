#include "borderbutton.h"
#include <QMouseEvent>
#include <QEvent>
#include <QWidget>
#include <QDebug>
#include <QButtonGroup>
#include "app/global.h"
#include "launcherframe.h"

BorderButton::BorderButton(QWidget *parent) : QPushButton(parent)
{
    setCheckable(true);
    setObjectName("BorderButton");
}

void BorderButton::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton){
       emit rightClicked(mapToGlobal(event->pos()));
    }
    QPushButton::mousePressEvent(event);
}

void BorderButton::enterEvent(QEvent *event){
    setChecked(true);
    QPushButton::enterEvent(event);
}

void BorderButton::leaveEvent(QEvent *event){

    QPushButton::leaveEvent(event);
}

BorderButton::~BorderButton()
{

}

