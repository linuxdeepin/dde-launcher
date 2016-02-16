/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H


#include <QtCore>

class MenumanagerInterface;
class MenuInterface;
class DBusDockedAppManager;
class QJsonObject;
class NotificationInterface;

#define MenuManager_service "com.deepin.menu"
#define MenuManager_path "/com/deepin/menu"


class MenuController : public QObject
{
    Q_OBJECT

public:

    explicit MenuController(QObject *parent = 0);
    ~MenuController();

    void initConnect();
    bool isItemOnDock(QString appKey);
    bool isItemOnDesktop(QString appKey);
    bool isItemStartup(QString appKey);

    QJsonObject createMenuItem(int itemId, QString itemText);
    QJsonObject createSeperator();

    QString createMenuContent(QString appKey);
    QString registerMenu();
    QString JsonToQString(QPoint pos, QString menucontent);

signals:

public slots:
    void showMenuByAppItem(QString appKey, QPoint pos);
    void menuItemInvoked(QString itemId, bool flag);

    void showMenu(QString menuDBusObjectPath, QString menuContent);
    void hideMenu(const QString& menuDBusObjectPath);
    void hideMenuByAppKey(const QString& appKey);

    void handleOpen(QString appKey);
    void handleToDesktop(QString appkey);
    void handleToDock(QString appKey);
    void handleToStartup(QString appKey);

    void handleUninstallAction(QString appKey, int id);
    void startUnistall(QString appKey);
    void handleUninstallSuccess(const QString& appKey);
    void handleUninstallFail(const QString& appKey, const QString& message);

    void handleMenuClosed();

private:
    MenumanagerInterface* m_menuManagerInterface;
    MenuInterface* m_menuInterface;
    DBusDockedAppManager* m_dockAppManagerInterface;
    NotificationInterface* m_notifcationInterface;

    QString m_appKeyRightClicked;
    QString m_currentMenuObjectPath;
    QMap<QString, QString> m_menuObjectPaths;
    bool m_isItemOnDesktop;
    bool m_isItemOnDock;
    bool m_isItemStartup;
};

#endif // MENUCONTROLLER_H
