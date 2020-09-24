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

#include "src/widgets/gradientlabel.h"
#include "src/widgets/searchwidget.h"
#include "src/widgets/navigationwidget.h"
#include "src/widgets/categorytitlewidget.h"
#include "launcherinterface.h"
#include "delegate/appitemdelegate.h"
#include "global_util/util.h"
#include "global_util/calculate_util.h"
#include "global_util/constants.h"
#include "model/appsmanager.h"
#include "model/appslistmodel.h"
#include "view/appgridview.h"
#include "worker/menuworker.h"
#include "widgets/applistarea.h"
#include "boxframe/boxframe.h"
#include "widgets/blurboxwidget.h"
#include "view/multipagesview.h"
#include "widgets/scrollwidgetagent.h"

#include <memory>

#include <QWidget>
#include <QFrame>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTimer>
#include <QDebug>

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class BackgroundManager;
class DBusLauncherService;
class SharedEventFilter;
class FullScreenFrame : public BoxFrame, public LauncherInterface
{
    Q_OBJECT
    Q_PROPERTY(int dockPosition READ dockPosition DESIGNABLE true)
public:
    enum tabFocus {FirstItem, SearchEdit, CategoryChangeBtn, CategoryTital};
    explicit FullScreenFrame(QWidget *parent = nullptr);
    ~FullScreenFrame() override;

    void exit();
    void showByMode(const qlonglong mode);
    int dockPosition();
    void updateDisplayMode(const int mode);
    void nextTabWidget(int key);

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
    void reflashPageView(const AppsListModel::AppCategory category);
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
    //鼠标单击位置
    QPoint m_mouse_move_pos;
    QPoint m_mouse_press_pos;
    qint64 m_mouse_press_time;
    bool m_mouse_press;

    AppsListModel::AppCategory m_currentCategory;

    std::unique_ptr<MenuWorker> m_menuWorker;
    SharedEventFilter *m_eventFilter;

    CalculateUtil *m_calcUtil;
    AppsManager *m_appsManager;

    QTimer *m_delayHideTimer;
    QTimer *m_clearCacheTimer;

    NavigationWidget *m_navigationWidget;
    SearchWidget *m_searchWidget;

    QFrame *m_contentFrame;
    DHBoxWidget *m_appsIconBox;
    DHBoxWidget *m_appsItemBox;    // app 分组
    MaskQWidget *m_appsItemSeizeBox; // app 分组点位
    QHBoxLayout *m_iconHLayout;

    QLabel *m_tipsLabel;

    AppItemDelegate *m_appItemDelegate;
    MultiPagesView *m_multiPagesView;

    BlurBoxWidget *m_internetBoxWidget;
    BlurBoxWidget *m_chatBoxWidget;
    BlurBoxWidget *m_musicBoxWidget;
    BlurBoxWidget *m_videoBoxWidget;
    BlurBoxWidget *m_graphicsBoxWidget;
    BlurBoxWidget *m_gameBoxWidget;
    BlurBoxWidget *m_officeBoxWidget;
    BlurBoxWidget *m_readingBoxWidget;
    BlurBoxWidget *m_developmentBoxWidget;
    BlurBoxWidget *m_systemBoxWidget;
    BlurBoxWidget *m_othersBoxWidget;

    QFrame *m_topSpacing;
    QFrame *m_bottomSpacing;
    QLayout *m_pHBoxLayout = nullptr;
    QVBoxLayout *m_mainLayout;

    QList<ScrollWidgetAgent *> m_widgetAgentList;
    ScrollParallelAnimationGroup *m_animationGroup;
    DBusDisplay *m_displayInter;
    //只有窗口在完全显示出来后，才允许自动调整各部件位置
    bool m_canResizeDockPosition = false;
};
#endif // MAINFRAME_H
