/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#ifdef DCORE_NAMESPACE
DCORE_USE_NAMESPACE
#else
DUTIL_USE_NAMESPACE
#endif

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

    return app.exec();
}
