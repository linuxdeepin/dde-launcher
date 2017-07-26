
#include <gtk/gtk.h>

#include "fullscreenframe.h"
#include "dbuslauncherframe.h"
#include "model/appsmanager.h"
#include "dbusservices/dbuslauncherservice.h"

#include <QCommandLineParser>
#include <QTranslator>
#include <unistd.h>
#include <dapplication.h>
#include <DLog>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

#define PROP_GTK_ICON_THEME_NAME     "gtk-icon-theme-name"

void iconThemeChanged(GtkSettings *gsettings, GParamSpec *pspec, gpointer udata)
{
    Q_UNUSED(gsettings)
    Q_UNUSED(udata)
    Q_ASSERT(!strcmp(g_param_spec_get_name(pspec), PROP_GTK_ICON_THEME_NAME));
}

int main(int argv, char *args[])
{
    DApplication::loadDXcbPlugin();
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
    LauncherSys launcher;
    DBusLauncherService service(&launcher);
    Q_UNUSED(service);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.dde.Launcher") ||
        !connection.registerObject("/com/deepin/dde/Launcher", &launcher))
        qWarning() << "register dbus service failed";

#ifndef QT_DEBUG
    if (/*!positionArgs.isEmpty() && */cmdParser.isSet(showOption))
#endif
        QTimer::singleShot(1, &launcher, &LauncherSys::showLauncher);

    // monitor gtk icon theme changed
    GtkSettings *gs = gtk_settings_get_default();
    g_signal_connect(gs, "notify::" PROP_GTK_ICON_THEME_NAME, G_CALLBACK(iconThemeChanged), NULL);

    return app.exec();
}
