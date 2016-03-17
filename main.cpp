

#include "mainframe.h"
#include "dbus/dbuslauncherframe.h"
#include "dbusservices/dbuslauncherservice.h"

#include <QCommandLineParser>

#include <unistd.h>
#include <dapplication.h>

DWIDGET_USE_NAMESPACE

int main(int argv, char *args[])
{
    DApplication app(argv, args);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-launcher");
    app.setApplicationVersion("3.0");

    const bool quit = !app.setSingleInstance(QString("dde-launcher_%1").arg(getuid()));

    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("DDE Launcher");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.addPositionalArgument("mode", "show and toogle to <mode>");
    cmdParser.process(app);

    if (quit)
    {
        DBusLauncherFrame launcherFrame;
        if (launcherFrame.isValid())
            launcherFrame.Toggle();

        return 0;
    }

    MainFrame launcher;
    DBusLauncherService service(&launcher);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.dde.Launcher") ||
        !connection.registerObject("/com/deepin/dde/Launcher", &launcher))
        qWarning() << "register dbus service failed";

    launcher.show();

    return app.exec();
}
