/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

