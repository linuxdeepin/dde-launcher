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
#include "src/dbusinterface/dbusdisplay.h"
#include "widgets/blurboxwidget.h"
#include "view/multipagesview.h"

#include <memory>

#include <QFrame>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTimer>
#include <QDebug>

#include <dboxwidget.h>
#include <DFloatingButton>
typedef QList<DFloatingButton *>  DFloatBtnList;

#define CATEGORY_MAX 11

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
    explicit FullScreenFrame(QWidget *parent = 0);
    ~FullScreenFrame();

    enum FocusIndex {
        Applist,
        Category,
        Search,
        Default
    };

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
    void scrollToCategory(const AppsListModel::AppCategory &category, int nNext = 0);
    void scrollToBlurBoxWidget(BlurBoxWidget *category,int nNext = 0);
    void clickToCategory(const QModelIndex &index);
    void showTips(const QString &tips);
    void hideTips();
    void setCategoryIndex(AppsListModel::AppCategory &category, int nNext = 0);
    void addViewEvent(AppGridView *pView);
protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent*) override;

    // input method
    void inputMethodEvent(QInputMethodEvent *e) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery prop) const override;

private:
    void initUI();
    void initConnection();
    void initTimer();

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
    void checkCategoryVisible();
    void showPopupMenu(const QPoint &pos, const QModelIndex &context);
    void updateCurrentVisibleCategory();
    void updatePlaceholderSize();
    void updateDockPosition();

    AppsListModel *nextCategoryModel(const AppsListModel *currentModel);
    AppsListModel *prevCategoryModel(const AppsListModel *currentModel);
    AppsListModel::AppCategory nextCategoryModel(const AppsListModel::AppCategory category);
    AppsListModel::AppCategory prevCategoryModel(const AppsListModel::AppCategory category);

private slots:
    void layoutChanged();
    void searchTextChanged(const QString &keywords);
    void ensureScrollToDest(const QVariant &value);
    void ensureItemVisible(const QModelIndex &index);
    void refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums);
    void refreshTitleVisible();
    void reflashPageView(const AppsListModel::AppCategory category);
    void primaryScreenChanged();
private:
    CategoryTitleWidget *categoryTitle(const AppsListModel::AppCategory category) const;
    MultiPagesView *getCategoryGridViewList(const AppsListModel::AppCategory category);
    BlurBoxWidget  *categoryBoxWidget(const AppsListModel::AppCategory category) const;

    //根据光前cursor theme更新frame的光标
    void updateFrameCursor();

private:
    bool m_isConfirmDialogShown = false;
    bool m_refershCategoryTextVisible = false;
    int m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
    int m_displayMode = SEARCH;
    
    //Record the total number of apps
    int m_appNum;
    double rightMarginRation = 1;
    AppsListModel::AppCategory m_currentCategory = AppsListModel::All;
    std::unique_ptr<MenuWorker> m_menuWorker;
    SharedEventFilter *m_eventFilter;

    CalculateUtil *m_calcUtil;
    AppsManager *m_appsManager;
    QPropertyAnimation *m_scrollAnimation;

    QWidget *m_scrollDest;
    QTimer *m_delayHideTimer;
    QTimer *m_autoScrollTimer;
    QTimer *m_clearCacheTimer;

    NavigationWidget *m_navigationWidget;
    SearchWidget *m_searchWidget;
    AppListArea *m_appsArea;    // app 滚动区域
    DHBoxWidget *m_appsHbox;    // app 分组
    QHBoxLayout *m_iconHLayout;

    QWidget *m_viewListPlaceholder;
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

    AppsListModel *m_searchResultModel;
    AppsListModel *m_internetModel;
    AppsListModel *m_chatModel;
    AppsListModel *m_musicModel;
    AppsListModel *m_videoModel;
    AppsListModel *m_graphicsModel;
    AppsListModel *m_gameModel;
    AppsListModel *m_officeModel;
    AppsListModel *m_readingModel;
    AppsListModel *m_developmentModel;
    AppsListModel *m_systemModel;
    AppsListModel *m_othersModel;

    DFloatBtnList m_floatBtnList;
    QIcon m_iconViewActive;
    QIcon m_iconView;

    QFrame *m_topSpacing;
    QFrame *m_bottomSpacing;

    int m_padding;
    QLayout *m_pHBoxLayout = nullptr;
    QFrame *m_contentFrame;

    QVBoxLayout *m_mainLayout;
    FocusIndex m_nextFocusIndex;
    FocusIndex m_currentFocusIndex;
    bool m_firstStart{true};
    //总共的分类
    BlurBoxWidget *m_BoxWidget[CATEGORY_MAX]   =  {m_internetBoxWidget, m_chatBoxWidget, m_musicBoxWidget, m_videoBoxWidget, m_graphicsBoxWidget, m_gameBoxWidget
                                                   , m_officeBoxWidget, m_readingBoxWidget, m_developmentBoxWidget, m_systemBoxWidget, m_othersBoxWidget
                                                  };
    //当前处在第几个分类
    int m_boxWidgetPageCurrent[CATEGORY_MAX] = {0};
    int m_currentBox ;
    int m_focusIndex;

    //鼠标单击位置
    QPoint m_mousePos;
    qint64 m_mouse_press_time;
    int m_appsAreaHScrollBarValue;
    bool m_mouse_press;

    QRect m_primaryRawRect;
    int m_screenRawHeight;
    int m_screenRawWidth;
    DBusDisplay *m_displayInter;
};
#endif // MAINFRAME_H
