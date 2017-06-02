#ifndef LAUNCHERINTERFACE_H
#define LAUNCHERINTERFACE_H

class QModelIndex;
class LauncherInterface
{
public:
    virtual ~LauncherInterface() {}
    virtual void showLauncher() = 0;
    virtual void hideLauncher() = 0;
    virtual bool visible() = 0;

    virtual void launchCurrentApp() = 0;
    virtual void appendToSearchEdit(const char ch) = 0;
    virtual void moveCurrentSelectApp(const int key) = 0;

    virtual bool windowDeactiveEvent() = 0;

    virtual void uninstallApp(const QModelIndex &context) = 0;
};


#endif // LAUNCHERINTERFACE_H
