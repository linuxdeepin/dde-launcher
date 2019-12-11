/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     wangleiyong <wangleiyong_cm@deepin.com>
 *
 * Maintainer: wangleiyong <wangleiyong_cm@deepin.com>
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

#include <DFloatingButton>
#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

#define     ICON_SPACE      10
#define     VIEW_SPACE      20

typedef QList<DFloatingButton *> FloatBtnList;

class MultiPagesView : public QWidget
{
    Q_OBJECT
public:
    explicit MultiPagesView(AppsListModel::AppCategory categoryModel = AppsListModel::All, QWidget *parent = nullptr);

    void updatePageCount(int pageCount);
    void showCurrentPage(int currentPage);
    QModelIndex selectApp(const int key);
    AppGridView* pageView(int pageIndex);
    AppsListModel* pageModel(int pageIndex);

    QModelIndex getAppItem(int index);
    void setDataDelegate(QAbstractItemDelegate* delegate);
    void setSearchModel(AppsListModel* appMode, bool bSearch);
signals:

private slots:
    void clickIconBtn();
    void layoutChanged();

protected:
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void Init();

private:
    int m_pageCount;
    int m_pageIndex;
    AppsListModel::AppCategory m_appModel;

    AppsManager *m_appsManager;
    CalculateUtil *m_calcUtil;
    AppListArea *m_appListArea;
    AppGridViewList m_appGridViewList;
    pageAppsModelist m_pageAppsModelList;
    FloatBtnList m_floatBtnList;

    DHBoxWidget *m_pHBoxLayout;
    QHBoxLayout *m_iconLayout;

    QPropertyAnimation *pageSwitchAnimation;

    QIcon m_iconViewActive;
    QIcon m_iconView;

    QAbstractItemDelegate *m_delegate = nullptr;
};

#endif // MULTIPAGESVIEW_H
