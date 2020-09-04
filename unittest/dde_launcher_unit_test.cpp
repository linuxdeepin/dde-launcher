/*
 * Copyright (C) 2018 ~ 2028 Uniontech Technology Co., Ltd.
 *
 * Author:     chenjun <chenjun@uniontech.com>
 *
 * Maintainer: chenjun <chenjun@uniontech.com>
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

#include "dde_launcher_unit_test.h"
#include <QtTest/QtTest>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDBusArgument>

#include "src/dbusinterface/dbuslauncherframe.h"
#include "src/dbusinterface/dbusdisplay.h"
#include "src/dbusinterface/dbusfileinfo.h"
#include "src/dbusinterface/dbusmenu.h"
#include "src/dbusinterface/dbusmenumanager.h"
#include "src/dbusinterface/dbustartmanager.h"
#include "src/dbusinterface/monitorinterface.h"
#include "src/dbusinterface/dbusdock.h"
#include "src/dbusinterface/dbuslauncher.h"

LauncherUnitTest::LauncherUnitTest(QObject *parent)
    : QObject(parent)
{

}

LauncherUnitTest::~LauncherUnitTest()
{

}

void LauncherUnitTest::initTestCase()
{

}

void LauncherUnitTest::cleanupTestCase()
{

}

void LauncherUnitTest::case1_test()
{
    QBENCHMARK_ONCE;

}

void LauncherUnitTest::case2_testDisplayDBus()
{
    QBENCHMARK_ONCE;

    DBusDisplay displayInterface(this);
    QVERIFY(displayInterface.isValid());

    BrightnessMap map = displayInterface.brightness();
    qDebug() << QString("Test displayInterface brightness : %1").arg(map.count());

    QDBusObjectPath objectPath = displayInterface.builtinOutput();
    qDebug() << QString("Test displayInterface builtinOutput:%1").arg(objectPath.path());

    int displayMode = displayInterface.displayMode();
    qDebug() << QString("Test displayInterface displayMode:%1").arg(displayMode);

    bool hasChanged = displayInterface.hasChanged();
    qDebug() << QString("Test displayInterface hasChanged:%1").arg(hasChanged);

    QList<QDBusObjectPath> monitors = displayInterface.monitors();
    qDebug() << QString("Test displayInterface monitors:%1").arg(monitors.count());

    QString primary = displayInterface.primary();
    qDebug() << QString("Test displayInterface primary:%1").arg(primary);

    QRect primaryRect = displayInterface.primaryRect();
    qDebug() << QString("Test displayInterface primary:(%1,%2,%3,%4").arg(primaryRect.left()).arg(primaryRect.top()).arg(primaryRect.right()).arg(primaryRect.bottom());

    int screenHeight = displayInterface.screenHeight();
    qDebug() << QString("Test displayInterface screenHeight:%1").arg(screenHeight);

    int screenWidth = displayInterface.screenWidth();
    qDebug() << QString("Test displayInterface ScreenWidth:%1").arg(screenWidth);
}

void LauncherUnitTest::case3_testDockDBus()
{
    QBENCHMARK_ONCE;

    DBusDock dockInterface(this);
    QVERIFY(dockInterface.isValid());

    uint activeWindow = dockInterface.activeWindow();
    qDebug() << QString("Test dockInterface activeWindow:%1").arg(activeWindow);

    QList<QDBusObjectPath> entries = dockInterface.entries();
    qDebug() << QString("Test dockInterface entrie count:%1").arg(entries.count());

    int displayMode = dockInterface.displayMode();
    qDebug() << QString("Test dockInterface displayMode:%1").arg(displayMode);

    int hideMode = dockInterface.hideMode();
    qDebug() << QString("Test dockInterface hideMode:%1").arg(hideMode);

    int hideState = dockInterface.hideState();
    qDebug() << QString("Test dockInterface hideState:%1").arg(hideState);

    int position = dockInterface.position();
    qDebug() << QString("Test dockInterface position:%1").arg(position);

    quint32 iconSize = dockInterface.iconSize();
    qDebug() << QString("Test dockInterface iconSize:%1").arg(iconSize);

    QRect frontendRect = dockInterface.frontendRect();
    qDebug() << QString("Test dockInterface frontendRect:(%1,%2,%3,%4").arg(frontendRect.left()).arg(frontendRect.top()).arg(frontendRect.right()).arg(frontendRect.bottom());
}

void LauncherUnitTest::case4_testFileInfoDBus()
{
    DBusFileInfo fileInfoInteface(this);
    QVERIFY(fileInfoInteface.isValid());

    uint driveStartStopTypeMultidisk = fileInfoInteface.driveStartStopTypeMultidisk();
    qDebug() << QString("Test fileInfoInteface driveStartStopTypeMultidisk:%1").arg(driveStartStopTypeMultidisk);

    uint driveStartStopTypeNetwork = fileInfoInteface.driveStartStopTypeNetwork();
    qDebug() << QString("Test fileInfoInteface driveStartStopTypeMultidisk:%1").arg(driveStartStopTypeNetwork);

    uint driveStartStopTypePassword = fileInfoInteface.driveStartStopTypePassword();
    qDebug() << QString("Test fileInfoInteface driveStartStopTypeMultidisk:%1").arg(driveStartStopTypePassword);

    uint driveStartStopTypeShutdown = fileInfoInteface.driveStartStopTypeShutdown();
    qDebug() << QString("Test fileInfoInteface driveStartStopTypeShutdown:%1").arg(driveStartStopTypeShutdown);

    uint driveStartStopTypeUnknown = fileInfoInteface.driveStartStopTypeUnknown();
    qDebug() << QString("Test fileInfoInteface driveStartStopTypeUnknown:%1").arg(driveStartStopTypeUnknown);

    QString fileAttributeAccessCanDelete = fileInfoInteface.fileAttributeAccessCanDelete();
    qDebug() << QString("Test fileInfoInteface fileAttributeAccessCanDelete:%1").arg(fileAttributeAccessCanDelete);

    QString fileAttributeAccessCanExecute = fileInfoInteface.fileAttributeAccessCanExecute();
    qDebug() << QString("Test fileInfoInteface fileAttributeAccessCanDelete:%1").arg(fileAttributeAccessCanExecute);

    QString fileAttributeAccessCanRead = fileInfoInteface.fileAttributeAccessCanRead();
    qDebug() << QString("Test fileInfoInteface fileAttributeAccessCanRead:%1").arg(fileAttributeAccessCanRead);

    QString fileAttributeAccessCanRename = fileInfoInteface.fileAttributeAccessCanRename();
    qDebug() << QString("Test fileInfoInteface fileAttributeAccessCanRename:%1").arg(fileAttributeAccessCanRename);

    QString fileAttributeAccessCanTrash = fileInfoInteface.fileAttributeAccessCanTrash();
    qDebug() << QString("Test fileInfoInteface fileAttributeAccessCanTrash:%1").arg(fileAttributeAccessCanTrash);

    QString fileAttributeAccessCanWrite = fileInfoInteface.fileAttributeAccessCanWrite();
    qDebug() << QString("Test fileInfoInteface fileAttributeAccessCanWrite:%1").arg(fileAttributeAccessCanWrite);

    QString fileAttributeDosIsArchive = fileInfoInteface.fileAttributeDosIsArchive();
    qDebug() << QString("Test fileInfoInteface fileAttributeDosIsArchive:%1").arg(fileAttributeDosIsArchive);

    QString fileAttributeDosIsSystem = fileInfoInteface.fileAttributeDosIsSystem();
    qDebug() << QString("Test fileInfoInteface fileAttributeDosIsSystem:%1").arg(fileAttributeDosIsSystem);

    QString fileAttributeEtagValue = fileInfoInteface.fileAttributeEtagValue();
    qDebug() << QString("Test fileInfoInteface fileAttributeEtagValue:%1").arg(fileAttributeEtagValue);

    //to-do
}

void LauncherUnitTest::case5_testLauncherDBus()
{
    DBusLauncher launcherInterface(this);
    QVERIFY(launcherInterface.isValid());

    bool fullscreen = launcherInterface.fullscreen();
    qDebug() << QString("Test launcherInterface fullscreen:%1").arg(fullscreen);

    int displaymode = launcherInterface.displaymode();
    qDebug() << QString("Test launcherInterface displaymode:%1").arg(displaymode);
}

void LauncherUnitTest::case6_testMenuDBus()
{
    DBusMenu menuInterface("com.deepin.menu.Menu","/com/deepin/menu/Menu",QDBusConnection::sessionBus(),this);
    QVERIFY(menuInterface.isValid());
}

void LauncherUnitTest::case7_testMenuManagerDBus()
{
    DBusMenuManager menuManagerInterface(this);
    QVERIFY(menuManagerInterface.isValid());
}

void LauncherUnitTest::case8_testStartManagerDBus()
{
    DBusStartManager startManagerInterface(this);
    QVERIFY(startManagerInterface.isValid());
}

void LauncherUnitTest::case9_testMonitorInterface()
{
    MonitorInterface monitorInterface("/com/deepin/daemon/Display",this);
    QVERIFY(monitorInterface.isValid());

    MonitorMode bestMode = monitorInterface.bestMode();
    qDebug() << QString("Test monitorInterface Test monitorInterface bestMode(ID:%1,width:%2,height:%3,rate:%4)").arg(bestMode.id).arg(bestMode.width).arg(bestMode.height).arg(bestMode.rate);

    MonitorMode currentMode = monitorInterface.currentMode();
    qDebug() << QString("Test monitorInterface currentMode(ID:%1,width:%2,height:%3,rate:%4)").arg(currentMode.id).arg(currentMode.width).arg(currentMode.height).arg(currentMode.rate);

    QString fullName = monitorInterface.fullName();
    qDebug() << QString("Test monitorInterface fullName:%1").arg(fullName);

    ushort height = monitorInterface.height();
    qDebug() << QString("Test monitorInterface height:%1").arg(height);

    bool isComposited = monitorInterface.isComposited();
    qDebug() << QString("Test monitorInterface isComposited:%1").arg(isComposited);

    QString name = monitorInterface.name();
    qDebug() << QString("Test monitorInterface name:%1").arg(name);

    bool opened = monitorInterface.opened();
    qDebug() << QString("Test monitorInterface opened:%1").arg(opened);

    QStringList outputs = monitorInterface.outputs();
    qDebug() << QString("Test monitorInterface output count:%1").arg(outputs.count());

    ushort reflect = monitorInterface.reflect();
    qDebug() << QString("Test monitorInterface reflect:%1").arg(reflect);

    ushort rotation = monitorInterface.rotation();
    qDebug() << QString("Test monitorInterface rotation:%1").arg(rotation);

    ushort width = monitorInterface.width();
    qDebug() << QString("Test monitorInterface width:%1").arg(width);

    short x = monitorInterface.x();
    qDebug() << QString("Test monitorInterface x:%1").arg(x);

    short y = monitorInterface.x();
    qDebug() << QString("Test monitorInterface y:%1").arg(y);
}

/**
 * @brief LauncherUnitTest::checkDbusStartUp
 * 检查启动器通过dbus能否直接启动
 * 过程先杀掉启动器进程，然后dbus命令显示启动器，再根据进程执行返回值判断是否启动成功
 * 可以测出问题 dde-launcher 5.3.0.2-1 杀掉启动器进程之后需要按两次super才能打开启动器
 * bug:https://pms.uniontech.com/zentao/bug-view-41679.html
 */
void LauncherUnitTest::checkDbusStartUp()
{
    int killCode = QProcess::execute("killall", QStringList() << "dde-launcher");

    int dbusCode = QProcess::execute("dbus-send", QStringList() << "--print-reply"
                      << "--dest=com.deepin.dde.Launcher"
                      << "/com/deepin/dde/Launcher" << "com.deepin.dde.Launcher.Toggle");

    qDebug() << "kill code:" << killCode << ", dbus code:" << dbusCode;

    QCOMPARE(dbusCode, 0);
}

QTEST_MAIN(LauncherUnitTest)
