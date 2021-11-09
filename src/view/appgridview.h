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

#include <com_deepin_daemon_gesture.h>

#include "appsmanager.h"

using Gesture = com::deepin::daemon::Gesture;

class DragPageDelegate
{
public:
    virtual void mousePress(QMouseEvent *e) = 0;
    virtual void mouseMove(QMouseEvent *e) = 0;
    virtual void mouseRelease(QMouseEvent *e) = 0;
};

class CalculateUtil;
class AppsListModel;
class FullScreenFrame;
class AppGridView : public QListView
{
    Q_OBJECT

public:
    explicit AppGridView(QWidget *parent = nullptr);

    using QListView::indexAt;
    const QModelIndex indexAt(const int index) const;
    int indexYOffset(const QModelIndex &index) const;
    void setContainerBox(const QWidget *container);
    void updateItemHiDPIFixHook(const QModelIndex &index);

    void setDelegate(DragPageDelegate *pDelegate);
    DragPageDelegate* getDelegate();

    void dragOut(int pos);
    void dragIn(const QModelIndex &index);
    void setDropAndLastPos(const QPoint& itemPos);
    void flashDrag();
    QPixmap creatSrcPix(const QModelIndex &index, const QString &appKey);

    QRect appIconRect(const QModelIndex &index);
    const QRect indexRect(const QModelIndex &index) const;

    void createLabel();

    FullScreenFrame *fullscreen();

public slots:
    void setDragAnimationEnable() {m_enableAnimation = true;}
signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
    void requestScrollUp() const;
    void requestScrollDown() const;
    void requestScrollStop() const;
    void requestScrollLeft(const QModelIndex &index) const;
    void requestScrollRight(const QModelIndex &index) const;
    void dragEnd();
    void requestMouseRelease() const;

protected:
    void startDrag(const QModelIndex &index);
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private slots:
    void dropSwap();
    void fitToContent();
    void prepareDropSwap();
    void createFakeAnimation(const int pos, const bool moveNext, const int xb, const bool isLastAni = false);

private:
    int m_dropToPos;
    bool m_enableDropInside = false;                     // 拖拽释放后鼠标所在位置是否在listview范围内的标识
    bool m_enableAnimation = true;                       // app交换动画执行标识
    bool m_moveGridView = false;                         // GridView的mouseMove事件是否结束
    bool m_mousePress;                                   // 鼠标按下状态

    const QWidget *m_containerBox = nullptr;
    QTimer *m_dropThresholdTimer;                        // 推拽过程中app交互动画定时器对象
    QPropertyAnimation *m_lastFakeAni = nullptr;         // 推拽过程中app交换动画对象
    static Gesture *m_gestureInter;
    DragPageDelegate *m_pDelegate;

    static QPointer<AppsManager> m_appManager;
    static QPointer<CalculateUtil> m_calcUtil;
    static bool m_longPressed;                           // 保存触控屏是否可拖拽状态

    QTime m_dragLastTime;                                // 拖拽开始到结束的持续时间(ms)
    QPoint m_dropPoint;                                  // 过度动画的终点坐标
    QPoint m_dragStartPos;                               // 拖拽起点坐标

    QLabel *m_pixLabel;
    QList<QLabel *>m_floatLabels;
};

typedef QList<AppGridView *> AppGridViewList;

#endif // APPGRIDVIEW_H
