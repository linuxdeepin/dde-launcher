#include "transparentframe.h"
#include "app/global.h"
#include <QMouseEvent>


TransparentFrame::TransparentFrame(QWidget *parent) : QFrame(parent)
{
//    setAttribute(Qt::WA_TranslucentBackground);
//    setAttribute(Qt::WA_NoMousePropagation, false);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}


void TransparentFrame::mouseReleaseEvent(QMouseEvent *event){
    emit signalManager->mouseReleased();
    qDebug() << "==========" << event;
//    event->ignore();
    QFrame::mouseReleaseEvent(event);
}


TransparentFrame::~TransparentFrame()
{

}

