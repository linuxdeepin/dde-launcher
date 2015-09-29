#include "app/launcherapp.h"
#include "app/define.h"
#include "app/global.h"
#include "widgets/themeappicon.h"
#include "dbusinterface/dde_launcher_interface.h"
#include "dbusinterface/launcher_interface.h"
#include "controller/dbusworker.h"

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
		QTranslator translator;
		translator.load("/usr/share/dde-launcher/translations/dde-launcher_" + QLocale::system().name() + ".qm");
		a.installTranslator(&translator);


        debug_log_console_on();
        RegisterDdeSession();
        Singleton<ThemeAppIcon>::instance()->gtkInit();
        LauncherApp launcher;
        launcher.show();

        DBusWorker worker;
        QThread dbusThread;
        worker.moveToThread(&dbusThread);
        dbusThread.start();
        emit signalManager->requestData();

        qDebug() << "Starting the launcher application";
        int reslut = a.exec();
        qDebug() << "exits " << a.applicationName() << reslut;
        return reslut;
    }else{
        DDeLauncherInterface ddeLauncherInterface(LauncherServiceName, LauncherPathName, conn);
        ddeLauncherInterface.Toggle();
        return 0;
    }
}
