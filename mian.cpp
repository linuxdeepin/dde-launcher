
#include <dapplication.h>

#include "mainframe.h"

#include <QLoggingCategory>

DWIDGET_USE_NAMESPACE

int main(int argv, char *args[])
{
    DApplication app(argv, args);

    QLoggingCategory startup("startup");
    qCDebug(startup) << "startup";

    MainFrame launcher;
    launcher.show();
    qCInfo(startup) << "mainframe shown";

    return app.exec();
}
