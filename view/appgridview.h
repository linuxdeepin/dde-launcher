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

#ifndef APPGRIDVIEW_H
#define APPGRIDVIEW_H

#include <QListView>
#include <QSize>
#include <QLabel>

#include "model/appsmanager.h"

class CalculateUtil;
class AppsListModel;
class AppGridView : public QListView
{
    Q_OBJECT

public:
    explicit AppGridView(QWidget *parent = 0);

    using QListView::indexAt;
    const QModelIndex indexAt(const int index) const;
    int indexYOffset(const QModelIndex &index) const;
    void setContainerBox(const QWidget *container);
    void updateItemHiDPIFixHook(const QModelIndex &index);

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
    void requestScrollUp() const;
    void requestScrollDown() const;
    void requestScrollStop() const;

protected:
    void startDrag(const QModelIndex &index);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void enterEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void fitToContent();
    void prepareDropSwap();
    void createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni = false);
    void dropSwap();

private:
    const QRect indexRect(const QModelIndex &index) const;

private:
    int m_dropToPos;
    bool m_enableDropInside = false;
    QPoint m_dragStartPos;

    const QWidget *m_containerBox = nullptr;
    QTimer *m_dropThresholdTimer;
    QPropertyAnimation *m_lastFakeAni = nullptr;

    static QPointer<AppsManager> m_appManager;
    static QPointer<CalculateUtil> m_calcUtil;
};

#endif // APPGRIDVIEW_H
