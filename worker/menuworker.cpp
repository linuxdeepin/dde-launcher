/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "menuworker.h"

AppsManager *MenuWorker::m_appManager = nullptr;

MenuWorker::MenuWorker(QObject *parent) : QObject(parent)
{
    qDebug() << "MenuWorker";
    m_menuManagerInterface = new DBusMenuManager(this);

    m_dockAppManagerInterface = new DBusDock(this);
    m_startManagerInterface = new DBusStartManager(this);
    m_launcherInterface = new DBusLauncher(this);
    m_menuInterface = NULL;
    if (!m_appManager)
        m_appManager = AppsManager::instance();

    initConnect();

}


void MenuWorker::initConnect(){

}

MenuWorker::~MenuWorker()
{
}


void MenuWorker::showMenuByAppItem(const QModelIndex &index, QPoint pos){
    setCurrentModelIndex(index);
    m_appKey = m_currentModelIndex.data(AppsListModel::AppKeyRole).toString();
    m_appDesktop = m_currentModelIndex.data(AppsListModel::AppDesktopRole).toString();
    qDebug() << "appKey" << m_appKey;
    QString menuContent = createMenuContent(/*m_appKey*/);
    QString menuJsonContent = JsonToQString(pos, menuContent);
    QString menuDBusObjectpath = registerMenu();
    qDebug() << "dbus objectpath:" << menuDBusObjectpath;
    if (menuDBusObjectpath.length() > 0){
        showMenu(menuDBusObjectpath, menuJsonContent);
        m_currentMenuObjectPath = menuDBusObjectpath;
        m_menuObjectPaths.insert(m_appKey, menuDBusObjectpath);
    }else{
        qCritical() << "register menu fail!";
    }
}

QString MenuWorker::createMenuContent(/*QString appKey*/){
    m_isItemOnDesktop = m_currentModelIndex.data(AppsListModel::AppIsOnDesktopRole).toBool();
    m_isItemOnDock = m_currentModelIndex.data(AppsListModel::AppIsOnDockRole).toBool();
    m_isItemStartup = m_currentModelIndex.data(AppsListModel::AppAutoStartRole).toBool();
    m_isRemovable = m_currentModelIndex.data(AppsListModel::AppIsRemovableRole).toBool();

    QJsonObject openObj = createMenuItem(0, tr("Open(_O)"));
    QJsonObject seperatorObj1 = createSeperator();
    QJsonObject desktopObj;
    if (m_isItemOnDesktop){
        desktopObj = createMenuItem(1, tr("Remove from desktop"));
    }else{
        desktopObj = createMenuItem(1, tr("Send to desktop(_E)"));
    }
    QJsonObject dockObj;
    if (m_isItemOnDock){
        dockObj = createMenuItem(2, tr("Remove from dock"));
    }else{
        dockObj = createMenuItem(2, tr("Send to dock(_C)"));
    }
    QJsonObject seperatorObj2 = createSeperator();
    QJsonObject startupObj;
    if (m_isItemStartup){
        startupObj = createMenuItem(3, tr("Remove from startup(_R)"));
    }else{
        startupObj = createMenuItem(3, tr("Add to startup(_A)"));
    }
    QJsonObject uninstallObj = createMenuItem(4, tr("Uninstall"), m_isRemovable);

    QJsonArray items;
    items.append(openObj);
    items.append(seperatorObj1);
    items.append(desktopObj);
    items.append(dockObj);
    items.append(seperatorObj2);
    items.append(startupObj);
#ifndef WITHOUT_UNINSTALL_APP
    items.append(uninstallObj);
#endif

    QJsonObject menuObj;
    menuObj["checkableMenu"] = false;
    menuObj["singleCheck"] = false;
    menuObj["items"] = items;

    return QString(QJsonDocument(menuObj).toJson());
}

QJsonObject MenuWorker::createMenuItem(int itemId, QString itemText, bool isActive){
    QJsonObject itemObj;
    itemObj["itemId"] = QString::number(itemId);
    itemObj["itemText"] = itemText;
    itemObj["isActive"] = isActive;
    itemObj["isCheckable"] = false;
    itemObj["checked"] = false;
    itemObj["itemIcon"] = "";
    itemObj["itemIconHover"] = "";
    itemObj["itemIconInactive"] = "";
    itemObj["showCheckMark"] = false;
    QJsonObject subMenuObj;
    subMenuObj["checkableMenu"] = false;
    subMenuObj["singleCheck"] = false;
    subMenuObj["items"] = QJsonArray();
    itemObj["itemSubMenu"] = subMenuObj;
    return itemObj;
}

QJsonObject MenuWorker::createSeperator(){
    return createMenuItem(-100, "");
}

QString MenuWorker::JsonToQString(QPoint pos, QString menucontent) {
    QJsonObject menuObj;
    menuObj["x"] = pos.x();
    menuObj["y"] = pos.y();
    menuObj["isDockMenu"] = false;
    menuObj["menuJsonContent"] = menucontent;
    return QString(QJsonDocument(menuObj).toJson());
}

QString MenuWorker::registerMenu() {
    QDBusPendingReply<QDBusObjectPath> reply = m_menuManagerInterface->RegisterMenu();
    reply.waitForFinished();
    if (!reply.isError()) {
        return reply.value().path();
    } else {
        qDebug() << "reply:" << reply.error().message();
        return "";
    }
}

void MenuWorker::showMenu(QString menuDBusObjectPath, QString menuContent) {
    qDebug() << menuDBusObjectPath;
    m_menuInterface = new DBusMenu(MenuManager_service, menuDBusObjectPath, QDBusConnection::sessionBus(), this);
    m_menuInterface->ShowMenu(menuContent);
    connect(m_menuInterface, SIGNAL(ItemInvoked(QString, bool)),this, SLOT(menuItemInvoked(QString,bool)));
    connect(m_menuInterface, SIGNAL(MenuUnregistered()), this, SLOT(handleMenuClosed()));
    connect(m_menuInterface, SIGNAL(MenuUnregistered()), m_menuInterface, SLOT(deleteLater()));

    m_menuIsShown = true;
}

void MenuWorker::hideMenu(const QString &menuDBusObjectPath)
{
    m_menuManagerInterface->UnregisterMenu(menuDBusObjectPath);
}

void MenuWorker::hideMenuByAppKey(const QString &appKey)
{
    if (m_menuObjectPaths.contains(appKey)){
        hideMenu(m_menuObjectPaths.value(appKey));
    }
}

void MenuWorker::menuItemInvoked(QString itemId, bool flag){
    Q_UNUSED(flag)
    int id = itemId.toInt();
    qDebug() << "menuItemInvoked" << itemId;
    switch (id) {
    case 0:
        handleOpen();
        break;
    case 1:
        handleToDesktop();
        break;
    case 2:
        handleToDock();
        break;
    case 3:
        handleToStartup();
        break;
    case 4:
        emit  unInstallApp(m_currentModelIndex);
        break;
    default:
        break;
    }
}


void MenuWorker::handleOpen(){
    m_appManager->launchApp(m_currentModelIndex);

    emit quitLauncher();
}

void MenuWorker::handleMenuClosed(){
//    emit signalManager->rightClickedChanged(false);
    m_menuIsShown = false;
}

void MenuWorker::setCurrentModelIndex(const QModelIndex &index) {
    m_currentModelIndex = index;
}

const QModelIndex MenuWorker::getCurrentModelIndex() {
    return m_currentModelIndex;
}

void MenuWorker::handleToDesktop(){
    qDebug() << "handleToDesktop" << m_appKey;
    if (m_isItemOnDesktop){
        QDBusPendingReply<bool> reply = m_launcherInterface->RequestRemoveFromDesktop(m_appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from desktop:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply = m_launcherInterface->RequestSendToDesktop(m_appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "send to desktop:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }
}

void MenuWorker::handleToDock(){
    qDebug() << "handleToDock" << m_appKey;
    if (m_isItemOnDock){
        QDBusPendingReply<bool> reply = m_dockAppManagerInterface->RequestUndock(m_appDesktop);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from dock:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply =  m_dockAppManagerInterface->RequestDock(m_appDesktop, -1);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "send to dock:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }
}

void MenuWorker::handleToStartup(){
    QString desktopUrl = m_currentModelIndex.data(AppsListModel::AppDesktopRole).toString();
    if (m_isItemStartup){
        QDBusPendingReply<bool> reply = m_startManagerInterface->RemoveAutostart(desktopUrl);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from startup:" << ret;
            if (ret) {
//                emit signalManager->hideAutoStartLabel(appKey);
            }
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply =  m_startManagerInterface->AddAutostart(desktopUrl);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "add to startup:" << ret;
            if (ret){
//                emit signalManager->showAutoStartLabel(appKey);
            }
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }
}

