/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

    bool visible();
    void showLauncher();
    void hideLauncher();
    void uninstallApp(const QString &appKey);

private slots:
    void displayModeChanged();

private:
    LauncherInterface *m_launcherInter;
    DBusLauncher *m_dbusLauncherInter;
};

#endif // LAUNCHERSYS_H
