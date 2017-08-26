/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "applistarea.h"

AppListArea::AppListArea(QWidget *parent)
    : QScrollArea(parent)
{

}

void AppListArea::enterEvent(QEvent *e)
{
    QScrollArea::enterEvent(e);

    emit mouseEntered();
}
