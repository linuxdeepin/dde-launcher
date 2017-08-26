/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
    void requestScrollUp() const;
    void requestScrollDown() const;
    void requestScrollStop() const;

protected:
    using QListView::startDrag;
    void startDrag(const QModelIndex &index);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
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
