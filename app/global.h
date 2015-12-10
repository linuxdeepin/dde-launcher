#ifndef GLOBAL
#define GLOBAL

#include "widgets/singleton.h"
#include "signalmanager.h"
#include "controller/dbuscontroller.h"
#include "views/appitemmanager.h"
#include <QString>
#include <QStringList>
#include <QObject>

#define signalManager  Singleton<SignalManager>::instance()
#define dbusController Singleton<DBusController>::instance()
#define appItemManager Singleton<AppItemManager>::instance()

const int NavgationBarLeftMargin = 50;
const int TopBottomGradientHeight = 60;
const QString LauncherServiceName="com.deepin.dde.Launcher";
const QString LauncherPathName="/com/deepin/dde/Launcher";

const QStringList CategroyKeys{
    "Internet",
    "Office",
    "Development",
    "Reading",
    "Graphics",
    "Game",
    "Music",
    "System",
    "Video",
    "Chat",
    "Others"
};

const QStringList CategoryNames{ QObject::tr("Internet"),
                                 QObject::tr("Office"),
                                 QObject::tr("Development"),
                                 QObject::tr("Reading"),
                                 QObject::tr("Graphics"),
                                 QObject::tr("Game"),
                                 QObject::tr("Music"),
                                 QObject::tr("System"),
                                 QObject::tr("Video"),
                                 QObject::tr("Chat"),
                                 QObject::tr("Others")
                               };

#endif // GLOBAL
