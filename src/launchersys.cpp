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
#include "global_util/util.h"
#include "iconfreshthread.h"

#define FULL_SCREEN     0
#define MINI_FRAME      1

#define MOUSE_LEFTBUTTON 1

#define SessionManagerService "com.deepin.SessionManager"
#define SessionManagerPath "/com/deepin/SessionManager"

/**
 * @brief LauncherSys::LauncherSys 启动器界面实现及逻辑处理类
 * @param parent
 */
LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent)
    , m_launcherInter(nullptr)
    , m_dbusLauncherInter(new DBusLauncher(this))
    , m_sessionManagerInter(new com::deepin::SessionManager(SessionManagerService, SessionManagerPath, QDBusConnection::sessionBus(), this))
    , m_windowLauncher(nullptr)
    , m_fullLauncher(nullptr)
    , m_regionMonitor(new DRegionMonitor(this))
    , m_autoExitTimer(new QTimer(this))
    , m_ignoreRepeatVisibleChangeTimer(new QTimer(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_appIconFreshThread(new IconFreshThread(this))
{
    m_regionMonitor->setCoordinateType(DRegionMonitor::Original);

    m_autoExitTimer->setInterval(60 * 1000);
    m_autoExitTimer->setSingleShot(true);

    m_ignoreRepeatVisibleChangeTimer->setInterval(200);
    m_ignoreRepeatVisibleChangeTimer->setSingleShot(true);

    AppsManager::instance();

    displayModeChanged();

    // 启动应用图标和应用名称缓存线程,减少系统加载应用时的开销
    m_appIconFreshThread->start();

    connect(m_dbusLauncherInter, &DBusLauncher::FullscreenChanged, this, &LauncherSys::displayModeChanged, Qt::QueuedConnection);
    connect(m_dbusLauncherInter, &DBusLauncher::DisplayModeChanged, this, &LauncherSys::onDisplayModeChanged, Qt::QueuedConnection);
    connect(m_autoExitTimer, &QTimer::timeout, this, &LauncherSys::onAutoExitTimeout, Qt::QueuedConnection);
    connect(this, &LauncherSys::destroyed, m_appIconFreshThread, &IconFreshThread::releaseThread);

    // 当刷新系统图标主题时,会对系统所有应用缓存进行清空,该线程也退出,这里重新开启,重新加载应用信息到缓存中
    connect(m_appIconFreshThread, &IconFreshThread::finished, [&]() {
        m_appIconFreshThread->deleteLater();

        if (m_appIconFreshThread->getThreadState()) {
            m_appIconFreshThread = new IconFreshThread(this);
            m_appIconFreshThread->start();
        }
    });

    m_autoExitTimer->start();
}

LauncherSys::~LauncherSys()
{
    if (m_windowLauncher)
        delete m_windowLauncher;

    if (m_fullLauncher)
        delete m_fullLauncher;
}

void LauncherSys::showLauncher()
{
    if (m_sessionManagerInter->locked()) {
        qDebug() << "session locked, can not show launcher";
        return;
    }

    if (m_ignoreRepeatVisibleChangeTimer->isActive())
        return;
    m_ignoreRepeatVisibleChangeTimer->start();

    registerRegion();

    qApp->processEvents();

    m_autoExitTimer->stop();
    m_launcherInter->showLauncher();
}

void LauncherSys::hideLauncher()
{
    if (m_ignoreRepeatVisibleChangeTimer->isActive())
        return;
    m_ignoreRepeatVisibleChangeTimer->start();

    unRegisterRegion();

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
    LauncherInterface* lastLauncher = m_launcherInter;

    // 后端启动器FullScreen变化,更新m_calcUtil
    if (m_launcherInter && m_dbusLauncherInter) {
        m_calcUtil->setFullScreen(m_dbusLauncherInter->fullscreen());
    }

    if (m_calcUtil->fullscreen()) {
        if (!m_fullLauncher) {
            m_fullLauncher = new FullScreenFrame;
            m_fullLauncher->installEventFilter(this);
            connect(m_fullLauncher, &FullScreenFrame::visibleChanged, this, &LauncherSys::onVisibleChanged);
            connect(m_fullLauncher, &FullScreenFrame::visibleChanged, m_ignoreRepeatVisibleChangeTimer, static_cast<void (QTimer::*)()>(&QTimer::start), Qt::DirectConnection);
        }

        m_launcherInter = static_cast<LauncherInterface*>(m_fullLauncher);
    } else {
        if (!m_windowLauncher) {
            m_windowLauncher = new WindowedFrame;
            m_windowLauncher->installEventFilter(this);
            connect(m_windowLauncher, &WindowedFrame::visibleChanged, this, &LauncherSys::onVisibleChanged);
            connect(m_windowLauncher, &WindowedFrame::visibleChanged, m_ignoreRepeatVisibleChangeTimer, static_cast<void (QTimer::*)()>(&QTimer::start), Qt::DirectConnection);
        }
        m_launcherInter = static_cast<LauncherInterface*>(m_windowLauncher);
    }

    lastLauncher = lastLauncher ? lastLauncher : m_launcherInter;

    if (lastLauncher->visible()) {

        // 先关闭小窗口,再show全屏窗口,规避全屏窗口出现后,小窗口有迟滞关闭的视觉体验问题
        if (lastLauncher != m_launcherInter)
            lastLauncher->hideLauncher();

        m_launcherInter->showLauncher();
    } else {
        m_launcherInter->hideLauncher();
    }

    QTimer::singleShot(0, this, [=] {
        if (m_launcherInter->visible()) {
            registerRegion();
        }
    });
}

void LauncherSys::onVisibleChanged()
{
    emit visibleChanged(m_launcherInter->visible());
}

void LauncherSys::onAutoExitTimeout()
{
    if (visible())
        return m_autoExitTimer->start();

    if (SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "auto-exit", false).toBool()) {
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

void LauncherSys::registerRegion() {
    m_regionMonitorConnect = connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, [=] (const QPoint &p, const int flag) {
        if (flag == MOUSE_LEFTBUTTON) {
            m_launcherInter->regionMonitorPoint(p);
        }
    });

    if (!m_regionMonitor->registered()) {
        m_regionMonitor->registerRegion();
    }
}

void LauncherSys::unRegisterRegion() {
    m_regionMonitor->unregisterRegion();
    disconnect(m_regionMonitorConnect);
}

void LauncherSys::onDisplayModeChanged()
{
    if (m_fullLauncher) {
        m_fullLauncher->updateDisplayMode(m_dbusLauncherInter->displaymode());
    }
}
