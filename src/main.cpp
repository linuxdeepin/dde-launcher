// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fullscreenframe.h"
#include "dbuslauncherframe.h"
#include "model/appsmanager.h"
#include "dbuslauncherservice.h"
#include "accessible.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>

#include <QCommandLineParser>
#include <QAccessible>
#include <QTranslator>
#include <QDebug>

#include <unistd.h>

DWIDGET_USE_NAMESPACE
#ifdef DCORE_NAMESPACE
DCORE_USE_NAMESPACE
#else
DUTIL_USE_NAMESPACE
#endif

void dump_user_apss_preset_order_list()
{
    AppsManager *appsManager = AppsManager::instance();
    const auto appsList = appsManager->appsInfoList(AppsListModel::All);

    QStringList buf;

    for (const auto &app : appsList)
        buf << QString("'%1'").arg(app.m_key);

    qInfo().noquote() << '[' << buf.join(", ") << ']';
}

int main(int argc, char *argv[])
{
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::ColorCompositing, true);
    DApplication *app = DApplication::globalApplication(argc, argv);
    app->setQuitOnLastWindowClosed(false);
    app->setOrganizationName("deepin");
    app->setApplicationName("dde-launcher");
    app->setApplicationVersion("3.0");
    app->loadTranslator();
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);

    DLogManager::registerConsoleAppender();
    // 加载 Accessible 插件 测试使用 用sniff查看效果
    QAccessible::installFactory(accessibleFactory);

#ifndef QT_DEBUG
    // 默认日志路径是 ~/.cache/dde-launcher/dde-launcher.log
    DLogManager::registerFileAppender();
#endif

    bool quit = !app->setSingleInstance(QString("dde-launcher_%1").arg(getuid()));

    QCommandLineOption showOption(QStringList() << "s" << "show", "show launcher(hide for default.)");
    QCommandLineOption toggleOption(QStringList() << "t" << "toggle", "toggle launcher visible.");
    QCommandLineOption dumpPresetOrder(QStringList() << "d" << "dump", "dump user-specificed preset order list and exit.");

    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("DDE Launcher");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.addOption(showOption);
    cmdParser.addOption(toggleOption);
    cmdParser.addOption(dumpPresetOrder);
    cmdParser.process(*app);

    if (cmdParser.isSet(dumpPresetOrder)) {
        quit = true;
        dump_user_apss_preset_order_list();
    }

    if (quit) {
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

    // 设置当前程序使用的本地化信息,当前为系统默认的设置
    setlocale(LC_ALL, "");

    LauncherSys launcher;
    DBusLauncherService service(&launcher);
    Q_UNUSED(service);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.dde.Launcher") ||
            !connection.registerObject("/com/deepin/dde/Launcher", &launcher)) {

        qWarning() << "register dbus service failed";
    }

#ifndef QT_DEBUG
    if (cmdParser.isSet(showOption))
#endif
        launcher.show();

    return app->exec();
}
