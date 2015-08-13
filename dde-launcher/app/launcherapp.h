#ifndef LAUNCHERAPP_H
#define LAUNCHERAPP_H

#include <QObject>

class LauncherFrame;

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
};

#endif // LAUNCHERAPP_H
