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

#include "menuworker.h"

static QString ChainsProxy_path = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
        + "/deepin/proxychains.conf";

MenuWorker::MenuWorker(QObject *parent) : QObject(parent)
{
    m_xsettings = new QGSettings("com.deepin.xsettings", QByteArray(), this);
    m_menuManagerInterface = new DBusMenuManager(this);
    m_dockAppManagerInterface = new DBusDock(this);
    m_startManagerInterface = new DBusStartManager(this);
    m_launcherInterface = new DBusLauncher(this);
    m_menuInterface = NULL;
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
    m_isItemProxy = m_currentModelIndex.data(AppsListModel::AppIsProxyRole).toBool();
    m_isItemEnableScaling = m_currentModelIndex.data(AppsListModel::AppEnableScalingRole).toBool();

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

    QJsonObject proxyObj;
    proxyObj = createMenuItem(4, tr("Open by proxy"));
    proxyObj["isCheckable"] = true;
    proxyObj["checked"] = m_isItemProxy;

    QJsonObject scalingObj;
    scalingObj = createMenuItem(6, tr("Disable display scaling"));
    scalingObj["isCheckable"] = true;
    scalingObj["checked"] = !m_isItemEnableScaling;

    QJsonObject uninstallObj = createMenuItem(5, tr("Uninstall"), m_isRemovable);

    QJsonArray items;
    items.append(openObj);
    items.append(seperatorObj1);
    items.append(desktopObj);
    items.append(dockObj);
    items.append(seperatorObj2);
    items.append(startupObj);

    const double scale_ratio = m_xsettings->get("scale-factor").toDouble();
    if (!qFuzzyCompare(1.0, scale_ratio))
        items.append(scalingObj);

    if (QFile::exists(ChainsProxy_path))
        items.append(proxyObj);

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
        handleToProxy();
        break;
    case 5:
        emit  unInstallApp(m_currentModelIndex);
        break;
    case 6:
        handleSwitchScaling();
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
    emit menuAccepted();
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

void MenuWorker::handleToProxy()
{
    m_launcherInterface->SetUseProxy(m_appKey, !m_isItemProxy);
}

void MenuWorker::handleSwitchScaling()
{
    m_launcherInterface->SetDisableScaling(m_appKey, m_isItemEnableScaling);
}
