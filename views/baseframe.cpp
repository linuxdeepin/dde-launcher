#include "baseframe.h"
#include <QEvent>

BaseFrame::BaseFrame(QWidget *parent) : QFrame(parent)
{

}

void BaseFrame::enterEvent(QEvent *event){
    emit mouseEnterToggled(true);
    QFrame::enterEvent(event);
}

void BaseFrame::leaveEvent(QEvent *event){
    emit mouseEnterToggled(false);
    QFrame::leaveEvent(event);
}

BaseFrame::~BaseFrame()
{

}

