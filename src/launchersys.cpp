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

#include "launchersys.h"
#include "launcherinterface.h"
#include "fullscreenframe.h"
#include "windowedframe.h"
#include "model/appsmanager.h"

#include <QGSettings>

#define FULL_SCREEN     0
#define MINI_FRAME      1

LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent)

    , m_launcherInter(nullptr)
    , m_dbusLauncherInter(new DBusLauncher(this))
    , m_windowLauncher(new WindowedFrame)
    , m_fullLauncher(new FullScreenFrame)
    , m_regionMonitor(new DRegionMonitor(this))
    , m_autoExitTimer(new QTimer(this))
    , m_ignoreRepeatVisibleChangeTimer(new QTimer(this))
{
    m_regionMonitor->setCoordinateType(DRegionMonitor::Original);

    m_fullLauncher->installEventFilter(this);
    m_windowLauncher->installEventFilter(this);

    m_autoExitTimer->setInterval(60 * 1000);
    m_autoExitTimer->setSingleShot(true);

    m_ignoreRepeatVisibleChangeTimer->setInterval(200);
    m_ignoreRepeatVisibleChangeTimer->setSingleShot(true);

    displayModeChanged();

    AppsManager::instance();

    connect(m_dbusLauncherInter, &DBusLauncher::FullscreenChanged, this, &LauncherSys::displayModeChanged, Qt::QueuedConnection);
    connect(m_autoExitTimer, &QTimer::timeout, this, &LauncherSys::onAutoExitTimeout, Qt::QueuedConnection);
    connect(m_windowLauncher, &WindowedFrame::visibleChanged, this, &LauncherSys::visibleChanged);
    connect(m_windowLauncher, &WindowedFrame::visibleChanged, m_ignoreRepeatVisibleChangeTimer, static_cast<void (QTimer::*)()>(&QTimer::start), Qt::DirectConnection);
    connect(m_fullLauncher, &FullScreenFrame::visibleChanged, this, &LauncherSys::visibleChanged);
    connect(m_fullLauncher, &FullScreenFrame::visibleChanged, m_ignoreRepeatVisibleChangeTimer, static_cast<void (QTimer::*)()>(&QTimer::start), Qt::DirectConnection);

    m_autoExitTimer->start();
}

void LauncherSys::showLauncher()
{
    if (m_ignoreRepeatVisibleChangeTimer->isActive())
        return;
    m_ignoreRepeatVisibleChangeTimer->start();

    m_regionMonitorConnect = connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, [=] (const QPoint &p) {
        m_launcherInter->regionMonitorPoint(p);
    });

    if (!m_regionMonitor->registered()) {
        m_regionMonitor->registerRegion();
    }

    qApp->processEvents();

    m_autoExitTimer->stop();
    m_launcherInter->showLauncher();
}

void LauncherSys::hideLauncher()
{
    if (m_ignoreRepeatVisibleChangeTimer->isActive())
        return;
    m_ignoreRepeatVisibleChangeTimer->start();

    m_regionMonitor->unregisterRegion();

    disconnect(m_regionMonitorConnect);

    m_autoExitTimer->start();
    m_launcherInter->hideLauncher();
}

void LauncherSys::uninstallApp(const QString &appKey)
{
    m_launcherInter->uninstallApp(appKey);
}

bool LauncherSys::visible()
{
    return m_launcherInter->visible();
}

void LauncherSys::displayModeChanged()
{
    const bool visible = m_launcherInter && m_launcherInter->visible();

    if (m_launcherInter) {
        m_launcherInter->hideLauncher();
    }

    m_launcherInter = m_dbusLauncherInter->fullscreen() ?
                static_cast<LauncherInterface*>(m_fullLauncher) :
                static_cast<LauncherInterface*>(m_windowLauncher);

    if (visible)
        m_launcherInter->showLauncher();
    else
        m_launcherInter->hideLauncher();
}

void LauncherSys::onAutoExitTimeout()
{
    if (visible())
        return m_autoExitTimer->start();

    QGSettings gsettings("com.deepin.dde.launcher", "/com/deepin/dde/launcher/");
    if (gsettings.keys().contains("autoExit") && gsettings.get("auto-exit").toBool())
    {
        qWarning() << "Exit Timer timeout, may quitting...";
        qApp->quit();
    }
}

bool LauncherSys::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Hide && (watched == m_fullLauncher || watched == m_windowLauncher)) {
        m_regionMonitor->unregisterRegion();
        disconnect(m_regionMonitorConnect);
        m_autoExitTimer->start();
    }

    return QObject::eventFilter(watched, event);
}
