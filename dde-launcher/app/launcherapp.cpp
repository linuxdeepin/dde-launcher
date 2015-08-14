#include "launcherapp.h"
#include "views/launcherframe.h"
#include "Logger.h"

LauncherApp::LauncherApp(QObject *parent) : QObject(parent)
{
    m_launcherFrame = new LauncherFrame;
}

void LauncherApp::show(){
    m_launcherFrame->show();
    LOG_INFO() << "LauncherApp show";
}

LauncherApp::~LauncherApp()
{

}
