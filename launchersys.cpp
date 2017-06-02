#include "launchersys.h"
#include "launcherinterface.h"
#include "fullscreenframe.h"
#include "miniframe.h"
#include "model/appsmanager.h"

#define FULL_SCREEN     0
#define MINI_FRAME      1

LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent),

      m_launcherInter(nullptr),
      m_dbusLauncherInter(new DBusLauncher(this))
{
    displayModeChanged();

    AppsManager::instance();

    connect(m_dbusLauncherInter, &DBusLauncher::FullscreenChanged, this, &LauncherSys::displayModeChanged, Qt::QueuedConnection);
}

void LauncherSys::showLauncher()
{
    m_launcherInter->showLauncher();
}

void LauncherSys::hideLauncher()
{
    m_launcherInter->hideLauncher();
}

void LauncherSys::uninstallApp(const QString &appKey)
{
    qDebug() << appKey;
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
        m_launcherInter = new MiniFrame;
    else
        m_launcherInter = new FullScreenFrame;

    if (visible)
        m_launcherInter->showLauncher();
    else
        m_launcherInter->hideLauncher();
}
