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
#include "miniframe.h"
#include "newframe.h"
#include "model/appsmanager.h"

#include <QGSettings>

#define FULL_SCREEN     0
#define MINI_FRAME      1

LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent),

      m_launcherInter(nullptr),
      m_dbusLauncherInter(new DBusLauncher(this)),

      m_autoExitTimer(new QTimer(this))
{
    m_autoExitTimer->setInterval(60 * 1000);
    m_autoExitTimer->setSingleShot(true);

    displayModeChanged();

    AppsManager::instance();

    connect(m_dbusLauncherInter, &DBusLauncher::FullscreenChanged, this, &LauncherSys::displayModeChanged, Qt::QueuedConnection);
    connect(m_autoExitTimer, &QTimer::timeout, this, &LauncherSys::onAutoExitTimeout, Qt::QueuedConnection);

    m_autoExitTimer->start();
}

void LauncherSys::showLauncher()
{
    qApp->processEvents();

    m_autoExitTimer->stop();
    m_launcherInter->showLauncher();
}

void LauncherSys::hideLauncher()
{
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

    delete m_launcherInter;

    if (!m_dbusLauncherInter->fullscreen())
    {
        NewFrame *newFrame = new NewFrame;
        connect(newFrame, &NewFrame::visibleChanged, this, &LauncherSys::visibleChanged);
        m_launcherInter = newFrame;
    } else {
        FullScreenFrame *frame = new FullScreenFrame;
        connect(frame, &FullScreenFrame::visibleChanged, this, &LauncherSys::visibleChanged);
        m_launcherInter = frame;
    }

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

