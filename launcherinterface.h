#ifndef LAUNCHERINTERFACE_H
#define LAUNCHERINTERFACE_H

class LauncherInterface
{
public:
    virtual ~LauncherInterface() {}
    virtual void showLauncher() = 0;
    virtual void hideLauncher() = 0;
    virtual bool visible() = 0;
};

#endif // LAUNCHERINTERFACE_H
