#include "launchersys.h"
#include "launcherinterface.h"
#include "fullscreenframe.h"

LauncherSys::LauncherSys(QObject *parent)
    : QObject(parent),

      m_launcherInter(new FullScreenFrame)
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
