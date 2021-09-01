/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef LAUNCHERINTERFACE_H
#define LAUNCHERINTERFACE_H

class QString;
class QPoint;

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
    virtual void regionMonitorPoint(const QPoint &point) = 0;
    virtual bool windowDeactiveEvent() = 0;

    virtual void uninstallApp(const QString &appKey) = 0;
};


#endif // LAUNCHERINTERFACE_H
