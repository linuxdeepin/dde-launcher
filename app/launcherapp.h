#ifndef LAUNCHERAPP_H
#define LAUNCHERAPP_H

#include <QObject>

class LauncherFrame;
class DBusWorker;

class LauncherApp : public QObject
{
    Q_OBJECT
public:
    explicit LauncherApp(QObject *parent = 0);
    ~LauncherApp();

signals:

public slots:
    void show();

private:
    LauncherFrame* m_launcherFrame;
    DBusWorker* m_dbusWorker;
    QThread* m_dbusThread;
};

#endif // LAUNCHERAPP_H
