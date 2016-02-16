/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

    static QMap<QString, QString> UnistallAppNames;
    static void addCommandOptions();
signals:

public slots:
    void show();
    void handleUninstall(QString appKey);
    void handleButtonClicked(int buttonId);

private:
    LauncherFrame* m_launcherFrame;
    DBusWorker* m_dbusWorker;
    QThread* m_dbusThread;
};

#endif // LAUNCHERAPP_H
