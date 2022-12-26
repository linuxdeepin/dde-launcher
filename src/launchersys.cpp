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
#include "appsmanager.h"
#include "util.h"
#include "constants.h"
#include "amdbuslauncherinterface.h"
#include "amdbusdockinterface.h"

#define SessionManagerService "org.deepin.dde.SessionManager1"
#define SessionManagerPath "/org/deepin/dde/SessionManager1"

/**
 * @brief LauncherSys::LauncherSys 启动器界面实现及逻辑处理类
 * @param parent
 */
LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent)
    , m_appManager(AppsManager::instance())
    , m_launcherInter(nullptr)
    , m_sessionManagerInter(new SessionManager(SessionManagerService, SessionManagerPath, QDBusConnection::sessionBus(), this))
    , m_windowLauncher(nullptr)
    , m_fullLauncher(nullptr)
    , m_regionMonitor(new DRegionMonitor(this))
    , m_autoExitTimer(new QTimer(this))
    , m_ignoreRepeatVisibleChangeTimer(new QTimer(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_amDbusLauncher(new AMDBusLauncherInter(this))
    , m_amDbusDockInter(new AMDBusDockInter(this))
    , m_launcherPlugin(new LauncherPluginController(this))
{
    m_regionMonitor->setCoordinateType(DRegionMonitor::Original);
    displayModeChanged();

    m_autoExitTimer->setInterval(60 * 1000);
    m_autoExitTimer->setSingleShot(true);

    m_ignoreRepeatVisibleChangeTimer->setInterval(200);
    m_ignoreRepeatVisibleChangeTimer->setSingleShot(true);
    m_autoExitTimer->start();

    // 插件加载
    m_launcherPlugin->startLoader();

    connect(m_amDbusLauncher, &AMDBusLauncherInter::FullscreenChanged, this, &LauncherSys::displayModeChanged, Qt::QueuedConnection);
    connect(m_amDbusLauncher, &AMDBusLauncherInter::DisplayModeChanged, this, &LauncherSys::onDisplayModeChanged, Qt::QueuedConnection);

    connect(m_amDbusDockInter, &AMDBusDockInter::FrontendWindowRectChanged, this, &LauncherSys::onFrontendRectChanged);

    connect(m_autoExitTimer, &QTimer::timeout, this, &LauncherSys::onAutoExitTimeout, Qt::QueuedConnection);
    connect(ConfigWorker::instance(), &DConfig::valueChanged, this, &LauncherSys::onValueChanged);
}

LauncherSys::~LauncherSys()
{
    if (m_windowLauncher)
        delete m_windowLauncher;

    if (m_fullLauncher)
        delete m_fullLauncher;
}

/** dbus服务调用显示
 * @brief LauncherSys::showLauncher
 */
void LauncherSys::showLauncher()
{
    if (m_sessionManagerInter->locked()) {
        return;
    }

    if (m_ignoreRepeatVisibleChangeTimer->isActive())
        return;

    m_ignoreRepeatVisibleChangeTimer->start();

    m_autoExitTimer->stop();
    registerRegion();
    qApp->processEvents();
    m_launcherInter->showLauncher();
}

/**dbus调用隐藏
 * @brief LauncherSys::hideLauncher
 */
void LauncherSys::hideLauncher()
{
    if (m_ignoreRepeatVisibleChangeTimer->isActive())
        return;

    m_ignoreRepeatVisibleChangeTimer->start();

    unRegisterRegion();

    m_autoExitTimer->start();
    m_launcherInter->hideLauncher();
}

void LauncherSys::uninstallApp(const QString &desktopPath)
{
    m_launcherInter->uninstallApp(desktopPath);
}

bool LauncherSys::visible()
{
    return m_launcherInter->visible();
}

void LauncherSys::displayModeChanged()
{
    LauncherInterface *lastLauncher = m_launcherInter;
    if (m_launcherInter)
        m_calcUtil->setFullScreen(m_amDbusLauncher->fullscreen());

    if (m_calcUtil->fullscreen()) {
        if (!m_fullLauncher) {
            m_fullLauncher = new FullScreenFrame;
            m_fullLauncher->installEventFilter(this);
            connect(m_fullLauncher, &FullScreenFrame::visibleChanged, this, &LauncherSys::onVisibleChanged);
            connect(m_fullLauncher, &FullScreenFrame::visibleChanged, m_ignoreRepeatVisibleChangeTimer, static_cast<void (QTimer::*)()>(&QTimer::start), Qt::DirectConnection);
            connect(m_fullLauncher, &FullScreenFrame::searchApp, m_launcherPlugin, &LauncherPluginController::onSearchedTextChanged, Qt::QueuedConnection);
        }

        m_launcherInter = static_cast<LauncherInterface*>(m_fullLauncher);

    } else {
        if (!m_windowLauncher) {
            m_windowLauncher = new WindowedFrame;
            m_windowLauncher->installEventFilter(this);
            connect(m_windowLauncher, &WindowedFrame::visibleChanged, this, &LauncherSys::onVisibleChanged);
            connect(m_windowLauncher, &WindowedFrame::visibleChanged, m_ignoreRepeatVisibleChangeTimer, static_cast<void (QTimer::*)()>(&QTimer::start), Qt::DirectConnection);
            connect(m_windowLauncher, &WindowedFrame::searchApp, m_launcherPlugin, &LauncherPluginController::onSearchedTextChanged, Qt::QueuedConnection);
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

    QTimer::singleShot(0, this, [ = ] {
            registerRegion();
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

void LauncherSys::registerRegion()
{
    m_regionMonitorConnect = connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, &LauncherSys::onButtonPress);

    if (!m_regionMonitor->registered())
        m_regionMonitor->registerRegion();
}

void LauncherSys::unRegisterRegion()
{
    m_regionMonitor->unregisterRegion();
    disconnect(m_regionMonitorConnect);
}

void LauncherSys::onDisplayModeChanged()
{
    if (m_fullLauncher) {
        m_fullLauncher->updateDisplayMode(m_amDbusLauncher->displaymode());
    }
}

/** 启动器跟随任务栏进行显示
 * @brief LauncherSys::onFrontendRectChanged
 */
void LauncherSys::onFrontendRectChanged()
{
    if (m_launcherInter && m_launcherInter->visible())
        m_launcherInter->showLauncher();
}

void LauncherSys::onButtonPress(const QPoint &p, const int flag)
{
    m_launcherInter->regionMonitorPoint(p, flag);
}

void LauncherSys::onValueChanged()
{
    if (m_windowLauncher)
        m_windowLauncher->update();

    if (m_fullLauncher)
        m_fullLauncher->update();
}
