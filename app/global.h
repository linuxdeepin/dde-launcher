#ifndef GLOBAL
#define GLOBAL

#include "widgets/singleton.h"
#include "signalmanager.h"
#include "controller/dbuscontroller.h"
#include <QString>

#define signalManager  Singleton<SignalManager>::instance()
#define dbusController Singleton<DBusController>::instance()

const int leftMargin = 50;
const QString LauncherServiceName="com.deepin.dde.Launcher";
const QString LauncherPathName="/com/deepin/dde/Launcher";

#endif // GLOBAL

