#ifndef LAUNCHERSYS_H
#define LAUNCHERSYS_H

#include "dbusinterface/dbuslauncher.h"

#include <QObject>

class LauncherInterface;
class LauncherSys : public QObject
{
    Q_OBJECT

public:
    explicit LauncherSys(QObject *parent = 0);

    void showLauncher();
    void hideLauncher();
    bool visible();

private slots:
    void displayModeChanged();

private:
    LauncherInterface *m_launcherInter;
    DBusLauncher *m_dbusLauncherInter;
};

#endif // LAUNCHERSYS_H
