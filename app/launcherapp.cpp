#include "launcherapp.h"
#include "views/launcherframe.h"

LauncherApp::LauncherApp(QObject *parent) : QObject(parent)
{
    m_launcherFrame = new LauncherFrame;
}

void LauncherApp::show(){
    m_launcherFrame->show();
}

LauncherApp::~LauncherApp()
{

}
