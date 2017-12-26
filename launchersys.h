#ifndef LAUNCHERSYS_H
#define LAUNCHERSYS_H

#include "dbusinterface/dbuslauncher.h"

#include <QObject>
#include <QTimer>

class LauncherInterface;
class LauncherSys : public QObject
{
    Q_OBJECT

public:
    explicit LauncherSys(QObject *parent = 0);

    bool visible();
    void showLauncher();
    void hideLauncher();
    void uninstallApp(const QString &appKey);

private slots:
    void displayModeChanged();
    void onAutoExitTimeout();

private:
    LauncherInterface *m_launcherInter;
    DBusLauncher *m_dbusLauncherInter;

    QTimer *m_autoExitTimer;
};

#endif // LAUNCHERSYS_H
