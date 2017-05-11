#ifndef LAUNCHERSYS_H
#define LAUNCHERSYS_H

#include <QObject>

class LauncherInterface;
class LauncherSys : public QObject
{
    Q_OBJECT

public:
    explicit LauncherSys(QObject *parent = 0);

    void showLauncher();
    void hideLauncher();

private:
    LauncherInterface *m_launcherInter;
};

#endif // LAUNCHERSYS_H
