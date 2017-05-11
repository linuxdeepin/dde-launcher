#include "launchersys.h"
#include "launcherinterface.h"
#include "fullscreenframe.h"
#include "miniframe.h"

LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent),

#ifdef QT_DEBUG
      m_launcherInter(new MiniFrame)
#else
      m_launcherInter(new FullScreenFrame)
#endif
{
}

void LauncherSys::showLauncher()
{
    m_launcherInter->showLauncher();
}

void LauncherSys::hideLauncher()
{
    m_launcherInter->hideLauncher();
}

bool LauncherSys::visible()
{
    return m_launcherInter->visible();
}
