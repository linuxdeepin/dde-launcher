#ifndef GLOBAL
#define GLOBAL

#include "widgets/singleton.h"
#include "signalmanager.h"
#include "controller/dbuscontroller.h"
#include "views/appitemmanager.h"

#define signalManager  Singleton<SignalManager>::instance()
#define dbusController Singleton<DBusController>::instance()
#define appItemManager Singleton<AppItemManager>::instance()

const int leftMargin = 50;

#endif // GLOBAL

