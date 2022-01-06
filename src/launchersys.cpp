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
#include "constants.h"
#include "iconcachemanager.h"

#define SessionManagerService "com.deepin.SessionManager"
#define SessionManagerPath "/com/deepin/SessionManager"

/**
 * @brief LauncherSys::LauncherSys 启动器界面实现及逻辑处理类
 * @param parent
 */
LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent)
    , m_appManager(AppsManager::instance())
    , m_launcherInter(nullptr)
    , m_dbusLauncherInter(new DBusLauncher(this))
    , m_sessionManagerInter(new com::deepin::SessionManager(SessionManagerService, SessionManagerPath, QDBusConnection::sessionBus(), this))
    , m_windowLauncher(nullptr)
    , m_fullLauncher(nullptr)
    , m_regionMonitor(new DRegionMonitor(this))
    , m_autoExitTimer(new QTimer(this))
    , m_ignoreRepeatVisibleChangeTimer(new QTimer(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_dockInter(new DBusDock(this))
    , m_checkTimer(new QTimer(this))
{
    setClickState(false);
    m_checkTimer->setInterval(10);
    m_regionMonitor->setCoordinateType(DRegionMonitor::Original);
    displayModeChanged();

    m_autoExitTimer->setInterval(60 * 1000);
    m_autoExitTimer->setSingleShot(true);

    m_ignoreRepeatVisibleChangeTimer->setInterval(200);
    m_ignoreRepeatVisibleChangeTimer->setSingleShot(true);

    connect(m_dbusLauncherInter, &DBusLauncher::FullscreenChanged, this, &LauncherSys::displayModeChanged, Qt::QueuedConnection);
    connect(m_dbusLauncherInter, &DBusLauncher::DisplayModeChanged, this, &LauncherSys::onDisplayModeChanged, Qt::QueuedConnection);
    connect(m_autoExitTimer, &QTimer::timeout, this, &LauncherSys::onAutoExitTimeout, Qt::QueuedConnection);
    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &LauncherSys::onFrontendRectChanged);
    connect(IconCacheManager::instance(), &IconCacheManager::iconLoaded, this, &LauncherSys::aboutToShowLauncher, Qt::QueuedConnection);
    connect(m_checkTimer, &QTimer::timeout, this, &LauncherSys::aboutToShowLauncher);

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
        qInfo() << "session locked, can not show launcher";
        return;
    }

    if (m_checkTimer->isActive())
        m_ignoreRepeatVisibleChangeTimer->stop();

    if (m_ignoreRepeatVisibleChangeTimer->isActive())
        return;

    m_ignoreRepeatVisibleChangeTimer->start();

    if (IconCacheManager::iconLoadState()) {
        m_autoExitTimer->stop();
        registerRegion();
        qApp->processEvents();
        m_launcherInter->showLauncher();
    }
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

void LauncherSys::setClickState(bool state)
{
    m_clicked = state;
}

bool LauncherSys::clickState() const
{
    return m_clicked;
}

bool LauncherSys::visible()
{
    return m_launcherInter->visible();
}

void LauncherSys::displayModeChanged()
{
    LauncherInterface* lastLauncher = m_launcherInter;

    if (m_launcherInter && m_dbusLauncherInter)
        m_calcUtil->setFullScreen(m_dbusLauncherInter->fullscreen());

    if (m_calcUtil->fullscreen()) {
        if (!m_fullLauncher) {
            m_fullLauncher = new FullScreenFrame;
            m_fullLauncher->installEventFilter(this);
            connect(m_fullLauncher, &FullScreenFrame::visibleChanged, this, &LauncherSys::onVisibleChanged);
            connect(m_fullLauncher, &FullScreenFrame::visibleChanged, m_ignoreRepeatVisibleChangeTimer, static_cast<void (QTimer::*)()>(&QTimer::start), Qt::DirectConnection);
        }

        m_launcherInter = static_cast<LauncherInterface*>(m_fullLauncher);

        preloadIcon();
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

        setClickState(true);

        m_checkTimer->start();
        m_launcherInter->showLauncher();
    } else {
        m_launcherInter->hideLauncher();
    }

    QTimer::singleShot(0, this, [ = ] {
        if (m_launcherInter->visible())
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
    if (m_fullLauncher)
        m_fullLauncher->updateDisplayMode(m_dbusLauncherInter->displaymode());
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

void LauncherSys::aboutToShowLauncher()
{
    if (!m_checkTimer->isActive())
        m_checkTimer->start();

    if (m_launcherInter->visible()) {
        m_checkTimer->stop();
    }

    if (IconCacheManager::iconLoadState() && clickState()) {
        showLauncher();
        m_checkTimer->stop();
        setClickState(false);
    }
}

void LauncherSys::preloadIcon()
{
    if (!getDConfigValue("preload-apps-icon", true).toBool())
        return;

    // 全屏分类/自由模式，搜索或者导航栏的高度无法确定，确定后开始加载所有应用资源
    if (m_calcUtil->displayMode() == GROUP_BY_CATEGORY)
        emit m_appManager->smallToCategory();
    else
        emit m_appManager->smallToFullFree();
}

void LauncherSys::show()
{
    setClickState(true);
    aboutToShowLauncher();
}
