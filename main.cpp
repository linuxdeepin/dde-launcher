#include "app/launcherapp.h"
#include "app/define.h"
#include "app/global.h"
#include "widgets/themeappicon.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    debug_daemon_off();
    QApplication a(argc, argv);
    debug_log_console_on();
    Singleton<ThemeAppIcon>::instance()->gtkInit();
    LauncherApp launcher;
    dbusController->init();
    launcher.show();

    LOG_INFO() << "Starting the launcher application";
    int reslut = a.exec();
    LOG_INFO() << "exits " << a.applicationName() << reslut;
}
