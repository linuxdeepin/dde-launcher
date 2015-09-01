#include "app/launcherapp.h"
#include "app/define.h"
#include "app/global.h"
#include "widgets/themeappicon.h"
#include "dbusinterface/dde_launcher_interface.h"
#include "dbusinterface/launcher_interface.h"
#include <QApplication>
#include <QDBusConnection>
#include <QDBusInterface>

int main(int argc, char *argv[])
{
//    debug_daemon_off();
    QApplication a(argc, argv);
    QDBusConnection conn = QDBusConnection::sessionBus();
    if(conn.registerService(LauncherServiceName)){
        debug_log_console_on();
        RegisterDdeSession();
        Singleton<ThemeAppIcon>::instance()->gtkInit();
        LauncherApp launcher;
        dbusController->init();
        launcher.show();
        LOG_INFO() << "Starting the launcher application";
        int reslut = a.exec();
        LOG_INFO() << "exits " << a.applicationName() << reslut;
        return reslut;
    }else{
        DDeLauncherInterface ddeLauncherInterface(LauncherServiceName, LauncherPathName, conn);
        ddeLauncherInterface.Toggle();
        return 0;
    }
}
