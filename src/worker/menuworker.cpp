/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#include <QMenu>
#include <QSignalMapper>

static QString ChainsProxy_path = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
        + "/deepin/proxychains.conf";

MenuWorker::MenuWorker(QObject *parent) : QObject(parent)
{
    m_xsettings = new QGSettings("com.deepin.xsettings", QByteArray(), this);
    m_dockAppManagerInterface = new DBusDock(this);
    m_startManagerInterface = new DBusStartManager(this);
    m_launcherInterface = new DBusLauncher(this);
    m_appManager = AppsManager::instance();

    initConnect();
}


void MenuWorker::initConnect(){

}

MenuWorker::~MenuWorker()
{
}

void MenuWorker::showMenuByAppItem(QPoint pos, const QModelIndex &index) {
    setCurrentModelIndex(index);

    m_appKey = m_currentModelIndex.data(AppsListModel::AppKeyRole).toString();
    m_appDesktop = m_currentModelIndex.data(AppsListModel::AppDesktopRole).toString();
    m_isItemOnDesktop = m_currentModelIndex.data(AppsListModel::AppIsOnDesktopRole).toBool();
    m_isItemOnDock = m_currentModelIndex.data(AppsListModel::AppIsOnDockRole).toBool();
    m_isItemStartup = m_currentModelIndex.data(AppsListModel::AppAutoStartRole).toBool();
    m_isRemovable = m_currentModelIndex.data(AppsListModel::AppIsRemovableRole).toBool();
    m_isItemProxy = m_currentModelIndex.data(AppsListModel::AppIsProxyRole).toBool();
    m_isItemEnableScaling = m_currentModelIndex.data(AppsListModel::AppEnableScalingRole).toBool();

    qDebug() << "appKey" << m_appKey;

    QMenu *menu = new QMenu;

    QSignalMapper *signalMapper = new QSignalMapper(menu);

    QAction *open;
    QAction *desktop;
    QAction *dock;
    QAction *startup;
    QAction *proxy;
    QAction *scale;
    QAction *uninstall;

    open = new QAction(tr("Open"), menu);

    desktop = new QAction(m_isItemOnDesktop ?
                              tr("Remove from desktop") :
                              tr("Send to desktop"),
                          menu);

    dock = new QAction(m_isItemOnDock ?
                           tr("Remove from dock") :
                           tr("Send to dock"),
                       menu);

    startup = new QAction(m_isItemStartup ?
                              tr("Remove from startup") :
                              tr("Add to startup"),
                          menu);


    uninstall = new QAction(tr("Uninstall"), menu);

    menu->addAction(open);
    menu->addSeparator();
    menu->addAction(desktop);
    menu->addAction(dock);
    menu->addSeparator();
    menu->addAction(startup);

    if (QFile::exists(ChainsProxy_path)) {
        proxy = new QAction(tr("Use a proxy"), menu);
        proxy->setCheckable(true);
        proxy->setChecked(m_isItemProxy);
        menu->addAction(proxy);
        signalMapper->setMapping(proxy, Proxy);
        connect(proxy, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    }

    const double scale_ratio = m_xsettings->get("scale-factor").toDouble();
    if (!qFuzzyCompare(1.0, scale_ratio)) {
        scale = new QAction(tr("Disable display scaling"), menu);
        scale->setCheckable(true);
        scale->setChecked(!m_isItemEnableScaling);
        menu->addAction(scale);
        signalMapper->setMapping(scale, SwitchScale);
        connect(scale, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    }

    uninstall->setEnabled(m_isRemovable);

#ifndef WITHOUT_UNINSTALL_APP
    menu->addAction(uninstall);
#endif

    connect(open, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(desktop, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(dock, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(startup, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

    connect(uninstall, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

    signalMapper->setMapping(open, Open);
    signalMapper->setMapping(desktop, Desktop);
    signalMapper->setMapping(dock, Dock);
    signalMapper->setMapping(startup, Startup);
    signalMapper->setMapping(uninstall, Uninstall);

    connect(signalMapper, static_cast<void (QSignalMapper::*)(const int)>(&QSignalMapper::mapped), this, &MenuWorker::handleMenuAction);
    connect(menu, &QMenu::aboutToHide, this, &MenuWorker::handleMenuClosed);
    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);

    menu->move(pos);
    m_menuIsShown = true;
    m_menuGeometry = menu->geometry();
    menu->exec();
}

void MenuWorker::handleOpen()
{
    m_appManager->launchApp(m_currentModelIndex);

    emit appLaunched();
}

void MenuWorker::handleMenuClosed()
{
    emit menuAccepted();
    m_menuIsShown = false;
}

void MenuWorker::setCurrentModelIndex(const QModelIndex &index)
{
    m_currentModelIndex = index;
}

const QModelIndex MenuWorker::getCurrentModelIndex()
{
    return m_currentModelIndex;
}

void MenuWorker::handleMenuAction(int index)
{
    switch (index) {
    case Open:
        handleOpen();
        break;
    case Desktop:
        handleToDesktop();
        break;
    case Dock:
        handleToDock();
        break;
    case Startup:
        handleToStartup();
        break;
    case Proxy:
        handleToProxy();
        break;
    case SwitchScale:
        handleSwitchScaling();
        break;
    case Uninstall:
        emit unInstallApp(m_currentModelIndex);
        break;
    default:
        break;
    }
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
