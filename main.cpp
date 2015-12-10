#include "app/launcherapp.h"
#include "app/define.h"
#include "app/global.h"
#include "widgets/themeappicon.h"
#include "dbusinterface/dde_launcher_interface.h"
#include "dbusinterface/launcher_interface.h"
#include "widgets/commandlinemanager.h"

#include <QApplication>
#include <QTranslator>
#include <QDBusConnection>
#include <QDBusInterface>

int main(int argc, char *argv[])
{
//    debug_daemon_off();
    QApplication a(argc, argv);

    QDBusConnection conn = QDBusConnection::sessionBus();
    if(conn.registerService(LauncherServiceName)){
        // setup translator
        qApp->setOrganizationName("deepin");
        qApp->setApplicationName("dde-launcher");
        qApp->setApplicationVersion("2015-1.0");

		QTranslator translator;
		translator.load("/usr/share/dde-launcher/translations/dde-launcher_" + QLocale::system().name() + ".qm");
		a.installTranslator(&translator);

        debug_log_console_on();

        RegisterDdeSession();
        Singleton<ThemeAppIcon>::instance()->gtkInit();
        LauncherApp launcher;
        launcher.show();

        gtk_init(NULL, NULL);
        gdk_error_trap_push();
        initGtkThemeWatcher();

        qDebug() << "Starting the launcher application";


        LauncherApp::addCommandOptions();
        qDebug() << "CommandLineManager::instance()" << CommandLineManager::instance();
        int reslut = a.exec();
        qDebug() << "exits " << a.applicationName() << reslut;
        return reslut;
    }else{
        LauncherApp::addCommandOptions();
        DDeLauncherInterface ddeLauncherInterface(LauncherServiceName, LauncherPathName, conn);
        if (CommandLineManager::instance()->isSet("mode")){
            qlonglong mode = CommandLineManager::instance()->value("mode").toLongLong();
            ddeLauncherInterface.ShowByMode(mode);
        }else{
            ddeLauncherInterface.Toggle();
        }
        return 0;
    }
}
