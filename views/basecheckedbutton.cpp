/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "basecheckedbutton.h"
#include <QEvent>


BaseCheckedButton::BaseCheckedButton(QWidget *parent) : QPushButton(parent)
{
    setCheckable(true);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);
}


BaseCheckedButton::BaseCheckedButton(QString text, QWidget *parent):QPushButton(text, parent){
    setCheckable(true);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);
}

void BaseCheckedButton::enterEvent(QEvent *event){
    emit mouseEnterToggled(true);
    QPushButton::enterEvent(event);
}

void BaseCheckedButton::leaveEvent(QEvent *event){
    emit mouseEnterToggled(false);
    QPushButton::leaveEvent(event);
}

BaseCheckedButton::~BaseCheckedButton()
{

}
