/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "applistarea.h"

#include <QDebug>
#include <QWheelEvent>

AppListArea::AppListArea(QWidget *parent)
    : QScrollArea(parent)
{

}

void AppListArea::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers().testFlag(Qt::ControlModifier))
    {
        e->accept();

        if (e->delta() > 0)
            emit increaseIcon();
        else
            emit decreaseIcon();
    } else {
        QScrollArea::wheelEvent(e);
    }
}

void AppListArea::enterEvent(QEvent *e)
{
    QScrollArea::enterEvent(e);

    emit mouseEntered();
}
