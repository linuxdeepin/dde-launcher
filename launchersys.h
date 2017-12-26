/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
