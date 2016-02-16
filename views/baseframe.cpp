/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

