
#include "mainframe.h"
#include "dbuslauncherframe.h"
#include "dbusservices/dbuslauncherservice.h"

#include <QCommandLineParser>
#include <QTranslator>
#include <unistd.h>
#include <dapplication.h>
#include <DLog>

DWIDGET_USE_NAMESPACE
DUTIL_USE_NAMESPACE

int main(int argv, char *args[])
{
    DApplication app(argv, args);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-launcher");
    app.setApplicationVersion("3.0");

#ifdef QT_DEBUG
    DLogManager::registerConsoleAppender();
#else
    DLogManager::registerFileAppender();
#endif

    const bool quit = !app.setSingleInstance(QString("dde-launcher_%1").arg(getuid()));

    QCommandLineOption showOption(QStringList() << "s" << "show", "show launcher(hide for default.)");
    QCommandLineOption toggleOption(QStringList() << "t" << "toggle", "toggle launcher visible.");

    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("DDE Launcher");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.addOption(showOption);
    cmdParser.addOption(toggleOption);
//    cmdParser.addPositionalArgument("mode", "show and toogle to <mode>");
    cmdParser.process(app);

//    QStringList positionArgs = cmdParser.positionalArguments();
    if (quit)
    {
        DBusLauncherFrame launcherFrame;

        do {
            if (!launcherFrame.isValid())
                break;

            if (cmdParser.isSet(toggleOption))
                launcherFrame.Toggle();
            else if (cmdParser.isSet(showOption))
                launcherFrame.Show();

        } while (false);

        return 0;
    }

    // INFO: what's this?
    setlocale(LC_ALL, "");

    QTranslator translator;
    translator.load("/usr/share/dde-launcher/translations/dde-launcher_" +
                    QLocale::system().name() + ".qm");
    app.installTranslator(&translator);
    MainFrame launcher;
    DBusLauncherService service(&launcher);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.dde.Launcher") ||
        !connection.registerObject("/com/deepin/dde/Launcher", &launcher))
        qWarning() << "register dbus service failed";


#ifndef QT_DEBUG
    if (/*!positionArgs.isEmpty() && */cmdParser.isSet(showOption))
#endif
        launcher.show();

    return app.exec();
}
