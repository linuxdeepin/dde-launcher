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
#include "navigationwidget.h"
#include "categorytitlewidget.h"
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
#include "blurboxwidget.h"
#include "multipagesview.h"
#include "scrollwidgetagent.h"

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
class DBusLauncherService;
class SharedEventFilter;
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

    void exit();
    int dockPosition();
    void updateDisplayMode(const int mode);
    void nextTabWidget(int key);
    void setBlurWidgetVisible(bool state = false);
    void mousePressDrag(QMouseEvent *e);
    void mouseMoveDrag(QMouseEvent *e);
    void mouseReleaseDrag(QMouseEvent *e);

signals:
    void visibleChanged(bool visible);
    void displayModeChanged(const int mode);
    void categoryAppNumsChanged(const AppsListModel::AppCategory category, const int appNums);
    void currentVisibleCategoryChanged(const AppsListModel::AppCategory currentVisibleCategory) const;
    void scrollChanged(const AppsListModel::AppCategory &category);

public slots:
    void scrollToCategory(const AppsListModel::AppCategory oldCategory, const AppsListModel::AppCategory newCategory);
    void blurBoxWidgetMaskClick(const AppsListModel::AppCategory appCategory);
    void scrollPrev();
    void scrollNext();
    void scrollCurrent();
    void clickToCategory(const QModelIndex &index);
    void showTips(const QString &tips);
    void hideTips();
    void addViewEvent(AppGridView *pView);
    void scrollBlurBoxWidget(ScrollWidgetAgent * widgetAgent);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;

    // input method
    void inputMethodEvent(QInputMethodEvent *e) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery prop) const override;

private:
    void initUI();
    void initConnection();

    void uninstallApp(const QString &appKey) Q_DECL_OVERRIDE;
    void uninstallApp(const QModelIndex &context);
    void showLauncher() Q_DECL_OVERRIDE;
    void hideLauncher() Q_DECL_OVERRIDE;
    bool visible() Q_DECL_OVERRIDE;
    void moveCurrentSelectApp(const int key) Q_DECL_OVERRIDE;
    void appendToSearchEdit(const char ch) Q_DECL_OVERRIDE;
    void launchCurrentApp() Q_DECL_OVERRIDE;
    bool windowDeactiveEvent() Q_DECL_OVERRIDE;
    void regionMonitorPoint(const QPoint &point) Q_DECL_OVERRIDE;

    void updateGeometry();
    void categoryListChanged();
    void showPopupMenu(const QPoint &pos, const QModelIndex &context);
    void updateDockPosition();

    AppsListModel::AppCategory nextCategoryType(const AppsListModel::AppCategory category);
    AppsListModel::AppCategory prevCategoryType(const AppsListModel::AppCategory category);

    virtual const QScreen *currentScreen() override;

private slots:
    void layoutChanged();
    void searchTextChanged(const QString &keywords);
    void refreshPageView(const AppsListModel::AppCategory category);
    void primaryScreenChanged();

private:
    CategoryTitleWidget *categoryTitle(const AppsListModel::AppCategory category) const;
    MultiPagesView *getCategoryGridViewList(const AppsListModel::AppCategory category);
    BlurBoxWidget  *getCategoryBoxWidget(const AppsListModel::AppCategory category) const;

    void checkCurrentCategoryVisible();
    void showCategoryBoxWidget(AppsListModel::AppCategory appCategory);
    void hideCategoryBoxWidget();
    void scrollToCategoryFinish();
    ScrollWidgetAgent *getScrollWidgetAgent(PosType type);
    BlurBoxWidget  *getCategoryBoxWidgetByPostType(PosType posType, AppsListModel::AppCategory appCategory);
    int nearestCategory(const AppsListModel::AppCategory oldCategory, const AppsListModel::AppCategory newCategory);

private:
    bool m_isConfirmDialogShown = false;
    int m_displayMode = SEARCH;
    int m_focusIndex;
    int m_currentIndex = 0;

    QPoint m_mouse_move_pos;
    QPoint m_mouse_press_pos;
    qint64 m_mouse_press_time;
    bool m_mouse_press;
    QPoint m_startPoint;                                // 鼠标拖动起点坐标

    AppsListModel::AppCategory m_currentCategory;

    std::unique_ptr<MenuWorker> m_menuWorker;
    SharedEventFilter *m_eventFilter;                   // 事件过滤类

    CalculateUtil *m_calcUtil;
    AppsManager *m_appsManager;

    QTimer *m_delayHideTimer;
    QTimer *m_clearCacheTimer;

    NavigationWidget *m_navigationWidget;               // 全屏模式下导航栏(搜索控件正下方)
    SearchWidget *m_searchWidget;                       // 顶部水平方向控件 (全屏模式下搜索, 左上模式切换按钮,右上全屏和小窗口模式切换按钮)

    QFrame *m_contentFrame;
    DHBoxWidget *m_appsIconBox;
    DHBoxWidget *m_appsItemBox;                         // 分类后容纳app列表的控件
    MaskQWidget *m_appsItemSeizeBox;                    // app 分组点位
    QHBoxLayout *m_iconHLayout;

    QLabel *m_tipsLabel;

    AppItemDelegate *m_appItemDelegate;                 // 全屏模式下listview视图代理
    MultiPagesView *m_multiPagesView;                   // 全屏视图控件类（listview + 分页控件））

    BlurBoxWidget *m_internetBoxWidget;                 // 全屏应用分类下网络应用控件
    BlurBoxWidget *m_chatBoxWidget;                     // 全屏应用分类下社交沟通控件
    BlurBoxWidget *m_musicBoxWidget;                    // 全屏应用分类下音乐欣赏控件
    BlurBoxWidget *m_videoBoxWidget;                    // 全屏应用分类下视频播放控件
    BlurBoxWidget *m_graphicsBoxWidget;                 // 全屏应用分类下图形图像控件
    BlurBoxWidget *m_gameBoxWidget;                     // 全屏应用分类下游戏娱乐控件
    BlurBoxWidget *m_officeBoxWidget;                   // 全屏应用分类下办公学习控件
    BlurBoxWidget *m_readingBoxWidget;                  // 全屏应用分类下阅读翻译控件
    BlurBoxWidget *m_developmentBoxWidget;              // 全屏应用分类下编程开发控件
    BlurBoxWidget *m_systemBoxWidget;                   // 全屏应用分类下系统管理控件
    BlurBoxWidget *m_othersBoxWidget;                   // 全屏应用分类下其他分类控件

    QFrame *m_topSpacing;
    QFrame *m_bottomSpacing;

    QList<ScrollWidgetAgent *> m_widgetAgentList;
    ScrollParallelAnimationGroup *m_animationGroup;
    DBusDisplay *m_displayInter;

    bool m_canResizeDockPosition = false;               // 只有窗口在完全显示出来后，才允许自动调整各部件位置

    bool m_bMousePress;                                 // 鼠标按下标识
    int m_nMousePos;                                    // 鼠标按住的起始坐标
    int m_scrollValue;                                  // 滑动区域当前停留的数值
    int m_scrollStart;                                  // 鼠标按下时滑动区域停留的数值
};
#endif // MAINFRAME_H
