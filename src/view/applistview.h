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
#include <DWindowManagerHelper>

#define DRAG_SCROLL_THRESHOLD 25

DGUI_USE_NAMESPACE

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
    void requestSwitchToCategory(const QModelIndex &index) const;
    void requestEnter(bool enter) const;

protected:
    void wheelEvent(QWheelEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void startDrag(const QModelIndex &index);

private:
    void handleScrollValueChanged();
    void handleScrollFinished();
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
    QPropertyAnimation *m_scrollAni;
    QGraphicsOpacityEffect *m_opacityEffect;
    DWindowManagerHelper *m_wmHelper;
    double m_speedTime = 2.0;

    QTimer *m_updateEnableSelectionByMouseTimer;
    QPoint m_lastTouchBeginPos;
    int touchTapDistance = -1;
};

#endif // APPLISTVIEW_H
