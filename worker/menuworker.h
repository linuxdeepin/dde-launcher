/*
 * Copyright (C) 2015 ~ 2017 Deepin Technology Co., Ltd.
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

#ifndef MENUWORKER_H
#define MENUWORKER_H

#include <QVariant>
#include <QProcess>
#include <QX11Info>
#include <QDBusObjectPath>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDBusPendingReply>
#include <QtCore>
#include <QModelIndex>
#include <QGSettings>

#include "dbusdock.h"
#include "dbuslauncher.h"
#include "dbustartmanager.h"
#include "model/appsmanager.h"
#include "model/appslistmodel.h"

class MenuWorker : public QObject
{
    Q_OBJECT

public:

    explicit MenuWorker(QObject *parent = 0);
    ~MenuWorker();

    enum MenuAction {
        Open = 1,
        Desktop = 2,
        Dock = 3,
        Startup = 4,
        Proxy = 5,
        SwitchScale = 6,
        Uninstall = 7
    };

    void initConnect();
    bool isMenuShown() const {return m_menuIsShown;}
    bool isItemOnDock(QString appKey);
    bool isItemOnDesktop(QString appKey);
    bool isItemStartup(QString appKey);

signals:
    void menuAccepted();
    void quitLauncher();
    void unInstallApp(const QModelIndex &index);

public slots:
    void showMenuByAppItem(const QModelIndex &index, QPoint pos);

    void handleOpen();
    void handleToDesktop();
    void handleToDock();
    void handleToStartup();
    void handleToProxy();
    void handleSwitchScaling();

    void handleMenuClosed();
    void setCurrentModelIndex(const QModelIndex &index);
    const QModelIndex getCurrentModelIndex();
    void handleMenuAction(int index);

private:
    QGSettings *m_xsettings;
    DBusDock* m_dockAppManagerInterface;
    DBusLauncher* m_launcherInterface;
    DBusStartManager* m_startManagerInterface;

    AppsManager *m_appManager;

    QModelIndex m_currentModelIndex = QModelIndex();
    QString m_appKey;
    QString m_appDesktop;
    QString m_currentMenuObjectPath;
    QMap<QString, QString> m_menuObjectPaths;
    bool m_isItemOnDesktop;
    bool m_isItemOnDock;
    bool m_isItemStartup;
    bool m_isRemovable;
    bool m_isItemProxy;
    bool m_isItemEnableScaling;

    bool m_menuIsShown = false;
};

#endif // MENUWORKER_H
