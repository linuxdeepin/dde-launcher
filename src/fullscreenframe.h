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

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "gradientlabel.h"
#include "searchwidget.h"
#include "launcherinterface.h"
#include "appitemdelegate.h"
#include "util.h"
#include "calculate_util.h"
#include "constants.h"
#include "appsmanager.h"
#include "appslistmodel.h"
#include "appgridview.h"
#include "menuworker.h"
#include "applistarea.h"
#include "boxframe.h"
#include "multipagesview.h"
#include "appdrawerwidget.h"
#include "maskqwidget.h"
#include "searchmodewidget.h"
#include "sortfilterproxymodel.h"

#include <dboxwidget.h>

#include <QWidget>
#include <QFrame>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTimer>

#include <memory>

DWIDGET_USE_NAMESPACE

class BackgroundManager;
class SharedEventFilter;
class DBusLauncher;

class FullScreenFrame : public BoxFrame, public LauncherInterface
{
    Q_OBJECT

    Q_PROPERTY(int dockPosition READ dockPosition DESIGNABLE true)

public:
    enum TabFocus
    {
        FirstItem,
        SearchEdit,
        CategoryChangeBtn,
        CategoryTital
    };

    explicit FullScreenFrame(QWidget *parent = nullptr);
    ~FullScreenFrame() override;

    void exit();
    int dockPosition();
    void updateDisplayMode(const int mode);
    void nextTabWidget(int key);
    void mousePressDrag(QMouseEvent *e);
    void mouseMoveDrag(QMouseEvent *e);
    void mouseReleaseDrag(QMouseEvent *e);

signals:
    void visibleChanged(bool visible);
    void displayModeChanged(const int mode);
    void categoryAppNumsChanged(const AppsListModel::AppCategory category, const int appNums);
    void currentVisibleCategoryChanged(const AppsListModel::AppCategory currentVisibleCategory) const;
    void scrollChanged(const AppsListModel::AppCategory &category);
    void searchApp(const QString &keyword = QString());

public slots:
    void showTips(const QString &tips);
    void hideTips();
    void addViewEvent(AppGridView *pView);
    void onHideMenu();

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;

    // input method
    void inputMethodEvent(QInputMethodEvent *e) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery prop) const override;

private:
    void initUI();
    void initAppView();
    void initConnection();
    void initAccessibleName();

    void uninstallApp(const QString &appKey) override;
    void showLauncher() override;
    void hideLauncher() override;
    bool visible() override;
    void moveCurrentSelectApp(const int key) override;
    void appendToSearchEdit(const char ch) override;
    void launchCurrentApp() override;
    void regionMonitorPoint(const QPoint &point, int flag = 1) override;

    void updateGeometry();
    void showPopupMenu(const QPoint &pos, const QModelIndex &context);
    void updateDockPosition();

private slots:
    void layoutChanged();
    void searchTextChanged(const QString &keywords, bool enableUpdateMode);
    void refreshPageView(const AppsListModel::AppCategory category);
    void onScreenInfoChange();
    void onAppClicked(const QModelIndex &index);
    void onRequestMouseRelease();

private:
    int m_displayMode;
    int m_focusIndex;
    qint64 m_mousePressSeconds;
    bool m_mousePressState;
    bool m_isConfirmDialogShown;
    QPoint m_mouseMovePos;
    QPoint m_mousePressPos;
    QPoint m_startPoint;

    std::unique_ptr<MenuWorker> m_menuWorker;
    SharedEventFilter *m_eventFilter;                   // 事件过滤类

    CalculateUtil *m_calcUtil;
    AppsManager *m_appsManager;

    QTimer *m_delayHideTimer;
    SearchWidget *m_searchWidget;                       // 顶部水平方向控件 (全屏模式下搜索, 左上模式切换按钮,右上全屏和小窗口模式切换按钮)
    QFrame *m_contentFrame;
    DHBoxWidget *m_appsIconBox;
    QLabel *m_tipsLabel;
    AppItemDelegate *m_appItemDelegate;                 // 全屏模式下listview视图代理
    MultiPagesView *m_multiPagesView;                   // 全屏视图控件类

    SearchModeWidget *m_searchModeWidget;              // 搜索模式控件
    AppsListModel *m_allAppsModel;
    SortFilterProxyModel *m_filterModel;

    QFrame *m_topSpacing;
    QFrame *m_bottomSpacing;
    AppDrawerWidget *m_drawerWidget;                    // 应用抽屉展开页面
    const QScreen *m_curScreen;

    bool m_canResizeDockPosition = false;               // 只有窗口在完全显示出来后，才允许自动调整各部件位置
    bool m_bMousePress;                                 // 鼠标按下标识
    int m_nMousePos;                                    // 鼠标按住的起始坐标
    int m_scrollValue;                                  // 滑动区域当前停留的数值
    int m_scrollStart;                                  // 鼠标按下时滑动区域停留的数值
};
#endif // MAINFRAME_H
