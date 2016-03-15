

#include "mainframe.h"

#include <QLoggingCategory>

#include <unistd.h>
#include <dapplication.h>

DWIDGET_USE_NAMESPACE

int main(int argv, char *args[])
{
    DApplication app(argv, args);
    if (!app.setSingleInstance(QString("dde-launcher_%1").arg(getuid())))
    {
        qCritical() << "set single instance failed!!!";
        return 0;
    }

    QLoggingCategory startup("startup");
    qCDebug(startup) << "startup";

    MainFrame launcher;
    launcher.show();
    qCInfo(startup) << "mainframe shown";

    return app.exec();
}
