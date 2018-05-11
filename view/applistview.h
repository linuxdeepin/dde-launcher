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

#ifndef APPLISTVIEW_H
#define APPLISTVIEW_H

#include <QListView>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#define DRAG_SCROLL_THRESHOLD 10

class AppListView : public QListView
{
    Q_OBJECT

public:
    explicit AppListView(QWidget *parent = 0);

    using QListView::indexAt;
    const QModelIndex indexAt(const int index) const;

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
    void requestScrollUp() const;
    void requestScrollDown() const;
    void requestScrollStop() const;

protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void startDrag(const QModelIndex &index);

private:
    void prepareDropSwap();
    void createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni = false);
    void dropSwap();

private:
    int m_dropToRow;
    int m_dragStartRow;
    QPoint m_dragStartPos;
    QTimer *m_dropThresholdTimer;
    bool m_enableDropInside = false;

    QPropertyAnimation *m_lastFakeAni = nullptr;
    QGraphicsOpacityEffect *m_opacityEffect;
};

#endif // APPLISTVIEW_H
