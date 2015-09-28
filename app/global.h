#ifndef GLOBAL
#define GLOBAL

#include "widgets/singleton.h"
#include "signalmanager.h"
#include "controller/dbuscontroller.h"
#include <QString>
#include <QStringList>
#include <QObject>

#define signalManager  Singleton<SignalManager>::instance()
#define dbusController Singleton<DBusController>::instance()

const int NavgationBarLeftMargin = 50;
const QString LauncherServiceName="com.deepin.dde.Launcher";
const QString LauncherPathName="/com/deepin/dde/Launcher";

const QStringList CategroyKeys{
    "internet" ,
    "multimedia",
    "games",
    "graphics",
    "productivity",
    "industry",
    "education",
    "development",
    "system",
    "utilities",
    "others"
};

const QStringList CategoryNames{ QObject::tr("internet"),
                                 QObject::tr("multimedia"),
                                 QObject::tr("games"),
                                 QObject::tr("graphics"),
                                 QObject::tr("productivity"),
                                 QObject::tr("industry"),
                                 QObject::tr("education"),
                                 QObject::tr("development"),
                                 QObject::tr("system"),
                                 QObject::tr("utilities"),
                                 QObject::tr("others")
                               };
#endif // GLOBAL
