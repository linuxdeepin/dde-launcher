// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIPAGESVIEW_H
#define MULTIPAGESVIEW_H

#include <QListView>
#include <QList>
#include <QScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QPropertyAnimation>

#include "../widgets/applistarea.h"
#include "appgridview.h"
#include "../model/appslistmodel.h"
#include "../model/appsmanager.h"
#include "../global_util/calculate_util.h"
#include "../widgets/gradientlabel.h"
#include "../boxframe/boxframe.h"
#include "pagecontrol.h"

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class MultiPagesView : public QWidget, public DragPageDelegate
{
    Q_OBJECT

public:
    enum Direction
    {
        Left,
        Right
    };

    explicit MultiPagesView(AppsListModel::AppCategory categoryModel = AppsListModel::All, QWidget *parent = nullptr);
    ~MultiPagesView();

    void updatePageCount(AppsListModel::AppCategory category = AppsListModel::All);
    void showCurrentPage(int currentPage);
    QModelIndex selectApp(const int key);
    AppGridView *pageView(int pageIndex);
    AppsListModel *pageModel(int pageIndex);
    int currentPage(){return m_pageIndex;}
    int pageCount() { return m_pageCount;}
    QModelIndex getAppItem(int index);
    void setDataDelegate(QAbstractItemDelegate *delegate);
    void setModel(AppsListModel::AppCategory category);
    void updatePosition(int mode = 0);

    void ShowPageView(AppsListModel::AppCategory category);

    void mousePress(QMouseEvent *e) override;
    void mouseMove(QMouseEvent *e) override;
    void mouseRelease(QMouseEvent *e) override;

    void setGradientVisible(bool visible);
    void updateGradient();
    void updateGradient(QPixmap& pixmap, QPoint startPoint, QPoint topRightImg);

    QPropertyAnimation::State getPageSwitchAnimationState();
    QWidget *getParentWidget();
    QPoint calculPadding(MultiPagesView::Direction dir);
    AppListArea *getListArea();
    AppGridViewList getAppGridViewList();
    AppsListModel::AppCategory getCategory();

    bool isScrolling();
signals:
    void connectViewEvent(AppGridView* pView);

private slots:
    void dragToLeft(const QModelIndex &index);
    void dragToRight(const QModelIndex &index);
    void dragStop();

protected:
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void InitUI();

private:
    GradientLabel *m_pLeftGradient;
    GradientLabel *m_pRightGradient;

    AppsManager *m_appsManager;                         // 应用管理类
    CalculateUtil *m_calcUtil;                          // 界面布局计算类
    AppListArea *m_appListArea;                         // 滑动区域控件
    AppGridViewList m_appGridViewList;                  // 多视图列表
    PageAppsModelist m_pageAppsModelList;               // 多视图模型列表

    DHBoxWidget *m_viewBox;

    QPropertyAnimation *m_pageSwitchAnimation;          // 分页切换动画

    QAbstractItemDelegate *m_delegate;                  // 视图代理基类
    PageControl *m_pageControl;                         // 分页控件

    int m_pageCount;
    int m_pageIndex;
    AppsListModel::AppCategory m_category;

    bool m_bDragStart;

    bool m_bMousePress;
    int m_nMousePos;
    int m_scrollValue;
    int m_scrollStart;
    QTime *m_changePageDelayTime;                      // 滚动延时，设定时间内只允许滚动一次
};

#endif // MULTIPAGESVIEW_H
