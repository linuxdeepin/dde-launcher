#include "launcherapp.h"
#include "views/launcherframe.h"
#include "Logger.h"
#include "dbusinterface/launcheradaptor.h"
#include "app/global.h"
#include "app/xcb_misc.h"
#include <QDBusConnection>

LauncherApp::LauncherApp(QObject *parent) : QObject(parent)
{
    m_launcherFrame = new LauncherFrame;
    new LauncherAdaptor(m_launcherFrame);
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.registerObject(LauncherPathName, m_launcherFrame);
}

void LauncherApp::show(){
    m_launcherFrame->Show();
    LOG_INFO() << "LauncherApp show";
}

LauncherApp::~LauncherApp()
{

}
