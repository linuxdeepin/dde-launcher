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

#include "applistview.h"

#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>

AppListView::AppListView(QWidget *parent)
    : QListView(parent)
{
    horizontalScrollBar()->setEnabled(false);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(SingleSelection);
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
}

void AppListView::mousePressEvent(QMouseEvent *e)
{
    QListView::mousePressEvent(e);

    const QModelIndex &index = indexAt(e->pos());
    if (!index.isValid())
        e->ignore();

    if (e->buttons() == Qt::RightButton) {
        const QPoint rightClickPoint = mapToGlobal(e->pos());

        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());
        if (clickedIndex.isValid())
            emit popupMenuRequested(rightClickPoint, clickedIndex);
    }
}

void AppListView::leaveEvent(QEvent *e)
{
    QListView::leaveEvent(e);

    emit entered(QModelIndex());
}
