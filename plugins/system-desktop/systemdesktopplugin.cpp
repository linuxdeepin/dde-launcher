#include "systemdesktopplugin.h"

#include <QtConcurrent>

SystemDesktopPlugin::SystemDesktopPlugin(QObject *parent) : QObject(parent)
{

}

void SystemDesktopPlugin::init()
{

}

void SystemDesktopPlugin::destory()
{

}

QString SystemDesktopPlugin::name()
{
    return "system-desktop";
}

QString SystemDesktopPlugin::description()
{
    return tr("load system desktop files");
}

int SystemDesktopPlugin::count()
{
    return 0;
}

QSharedPointer<AppData> SystemDesktopPlugin::data(int index)
{
    return nullptr;
}

QFuture<bool> SystemDesktopPlugin::run(QSharedPointer<AppData> app)
{
    return QtConcurrent::run([=] {
        return !app;
    });
}
