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

#ifndef NEWFRAME_H
#define NEWFRAME_H

#include "launcherinterface.h"
#include "dbusinterface/dbusdock.h"
#include "view/applistview.h"
#include "model/appslistmodel.h"
#include "delegate/applistdelegate.h"

#include <dblureffectwidget.h>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>

DWIDGET_USE_NAMESPACE

class NewFrame : public DBlurEffectWidget, public LauncherInterface
{
    Q_OBJECT

public:
    explicit NewFrame(QWidget *parent = nullptr);

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
    bool windowDeactiveEvent() Q_DECL_OVERRIDE;

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;

private slots:
    const QPoint scaledPosition(const QPoint &xpos);
    void adjustPosition();

private:
    DBusDock *m_dockInter;
    DPlatformWindowHandle m_windowHandle;
    DWindowManagerHelper *m_wmHelper;

    AppListView *m_appsView;
    AppsListModel *m_appsModel;
};

#endif // NEWFRAME_H
