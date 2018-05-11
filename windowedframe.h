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
#include "widgets/searchwidget.h"
#include "widgets/miniframerightbar.h"
#include "widgets/miniframeswitchbtn.h"
#include "global_util/constants.h"

#include <dblureffectwidget.h>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>

#include <QLabel>
#include <memory>

DWIDGET_USE_NAMESPACE

class WindowedFrame : public DBlurEffectWidget, public LauncherInterface
{
    Q_OBJECT

public:
    explicit WindowedFrame(QWidget *parent = nullptr);
    ~WindowedFrame();

    enum DisplayMode
    {
        Used,
        All
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

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;

private slots:
    void adjustPosition();
    void onToggleFullScreen();
    void onSwitchBtnClicked();
    void onJumpBtnClicked();
    void onWMCompositeChanged();
    void searchText(const QString &text);
    void showTips(const QString &text);
    void hideTips();
    void prepareHideLauncher();

private:
    DBusDock *m_dockInter;
    std::unique_ptr<MenuWorker> m_menuWorker;
    SharedEventFilter *m_eventFilter;
    DPlatformWindowHandle m_windowHandle;
    DWindowManagerHelper *m_wmHelper;

    AppsManager *m_appsManager;
    AppListView *m_appsView;
    AppsListModel *m_appsModel;
    AppsListModel *m_usedModel;
    AppsListModel *m_searchModel;

    SearchWidget *m_searchWidget;
    MiniFrameRightBar *m_rightBar;
    MiniFrameSwitchBtn *m_switchBtn;
    QLabel *m_tipsLabel;
    QTimer *m_delayHideTimer;
    QTimer *m_autoScrollTimer;

    DisplayMode m_displayMode;

    int m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
};

#endif // WINDOWEDFRAME_H
