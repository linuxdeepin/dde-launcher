#ifndef GLOBAL
#define GLOBAL

#include "widgets/singleton.h"
#include "signalmanager.h"
#include "controller/dbuscontroller.h"

#define signalManager  Singleton<SignalManager>::instance()
#define dbusController Singleton<DBusController>::instance()

const int leftMargin = 50;

#endif // GLOBAL

