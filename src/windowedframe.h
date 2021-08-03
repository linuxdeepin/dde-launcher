/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * Maintainer: rekols <rekols@foxmail.com>
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

#ifndef WINDOWEDFRAME_H
#define WINDOWEDFRAME_H

#include "launcherinterface.h"
#include "sharedeventfilter.h"
#include "dbusinterface/dbusdock.h"
#include "view/applistview.h"
#include "model/appslistmodel.h"
#include "model/appsmanager.h"
#include "worker/menuworker.h"
#include "delegate/applistdelegate.h"
#include "widgets/searchlineedit.h"
#include "widgets/modetogglebutton.h"
#include "widgets/miniframerightbar.h"
#include "widgets/miniframeswitchbtn.h"
#include "global_util/constants.h"
#include "global_util/calculate_util.h"

#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
#include <dblureffectwidget.h>
#include <dregionmonitor.h>
#include <com_deepin_daemon_appearance.h>
#include <DSearchEdit>
#include <DToolButton>

#include <memory>

DWIDGET_USE_NAMESPACE

using Appearance = com::deepin::daemon::Appearance;
class DBusDisplay;
class QLabel;

class WindowedFrame : public DBlurEffectWidget, public LauncherInterface
{
    Q_OBJECT

public:
    explicit WindowedFrame(QWidget *parent = nullptr);

    enum DisplayMode
    {
        Category,
        All
    };

    enum AnchoredCornor
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Normal
    };

    enum FocusPosition {
        RightBottom, //所有分类
        Computer,
        Setting,
        Power,
        Search,
        Applist,
        Default      //确保首次切换都切换到所有分类
    };

signals:
    void visibleChanged(bool visible);

private:
    void showLauncher() Q_DECL_OVERRIDE;
    void hideLauncher() Q_DECL_OVERRIDE;
    bool visible() Q_DECL_OVERRIDE;

    void moveCurrentSelectApp(const int key) Q_DECL_OVERRIDE;
    void appendToSearchEdit(const char ch) Q_DECL_OVERRIDE;
    void launchCurrentApp() Q_DECL_OVERRIDE;

    void uninstallApp(const QString &appKey) Q_DECL_OVERRIDE;
    void uninstallApp(const QModelIndex &context);
    bool windowDeactiveEvent() Q_DECL_OVERRIDE;
    void switchToCategory(const QModelIndex &index);

    QPainterPath getCornerPath(AnchoredCornor direction);

    void resetWidgetStyle();

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void inputMethodEvent(QInputMethodEvent *e) Q_DECL_OVERRIDE;
    QVariant inputMethodQuery(Qt::InputMethodQuery prop) const Q_DECL_OVERRIDE;
    void regionMonitorPoint(const QPoint &point) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

private slots:
    void initAnchoredCornor();
    void adjustPosition();
    void onToggleFullScreen();
    void onSwitchBtnClicked();
    void onWMCompositeChanged();
    void searchText(const QString &text);
    void showTips(const QString &text);
    void hideTips();
    void prepareHideLauncher();
    void recoveryAll();
    void onOpacityChanged(const double value);
    void primaryScreenChanged();
    void updatePosition();

private:
    DBusDock *m_dockInter;
    std::unique_ptr<MenuWorker> m_menuWorker;
    SharedEventFilter *m_eventFilter;
    DPlatformWindowHandle m_windowHandle;
    DWindowManagerHelper *m_wmHelper;

    QWidget *m_maskBg;
    AppsManager *m_appsManager;
    AppListView *m_appsView;
    AppsListModel *m_appsModel;          // 应用列表
    AppsListModel *m_usedModel;
    AppsListModel *m_searchModel;        // 搜索后的应用列表

    QWidget *m_rightWidget;
    MiniFrameRightBar *m_leftBar;        // 左侧分类按钮组
    MiniFrameSwitchBtn *m_switchBtn;     // 底部所有分类和返回切换控件
    QLabel *m_tipsLabel;
    QTimer *m_delayHideTimer;
    QTimer *m_autoScrollTimer;
    Appearance *m_appearanceInter;
    DisplayMode m_displayMode;

    int m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
    int m_radius = 0;
    CalculateUtil *m_calcUtil;
    AnchoredCornor m_anchoredCornor = Normal;
    QPainterPath m_cornerPath;
    FocusPosition m_focusPos;
    ModeToggleButton *m_modeToggleBtn;   // 小窗口右上角窗口模式切换按钮
    DSearchEdit *m_searcherEdit;         // 搜索控件
    bool m_enterSearchEdit;
    DBusDisplay *m_displayInter;
};

#endif // WINDOWEDFRAME_H
