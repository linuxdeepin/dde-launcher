// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applistarea.h"

#include <QDebug>
#include <QWheelEvent>
#include <QScroller>
#include <QTimer>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

AppListArea::AppListArea(QWidget *parent)
    : QScrollArea(parent)
{
}

void AppListArea::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers().testFlag(Qt::ControlModifier)) {
        e->accept();

        if (e->delta() > 0)
            emit increaseIcon();
        else
            emit decreaseIcon();
    }
}
