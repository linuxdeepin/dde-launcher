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
#include "menudialog.h"
#include "util.h"

#include <QSignalMapper>
#include <QWindow>

const bool IS_WAYLAND_DISPLAY = !qgetenv("WAYLAND_DISPLAY").isEmpty();

MenuWorker::MenuWorker(QObject *parent)
    : QObject(parent)
    , m_dockAppManagerInterface(new DBusDock(this))
    , m_launcherInterface(new DBusLauncher(this))
    , m_startManagerInterface(new DBusStartManager(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_appManager(AppsManager::instance())
    , m_isItemOnDesktop(false)
    , m_isItemOnDock(false)
    , m_isItemStartup(false)
    , m_isItemProxy(false)
    , m_isItemEnableScaling(false)
    , m_menu(new Menu)
{
    connect(m_menu, &Menu::notifyMenuDisplayState, this, &MenuWorker::notifyMenuDisplayState);
}

MenuWorker::~MenuWorker()
{
    delete m_menu;
}

void MenuWorker::creatMenuByAppItem(QMenu *menu, QSignalMapper *signalMapper)
{
    m_appKey = m_currentModelIndex.data(AppsListModel::AppKeyRole).toString();
    m_appDesktop = m_currentModelIndex.data(AppsListModel::AppDesktopRole).toString();
    m_isItemOnDesktop = m_currentModelIndex.data(AppsListModel::AppIsOnDesktopRole).toBool();
    m_isItemOnDock = m_currentModelIndex.data(AppsListModel::AppIsOnDockRole).toBool();
    m_isItemStartup = m_currentModelIndex.data(AppsListModel::AppAutoStartRole).toBool();
    m_isItemProxy = m_currentModelIndex.data(AppsListModel::AppIsProxyRole).toBool();
    m_isItemEnableScaling = m_currentModelIndex.data(AppsListModel::AppEnableScalingRole).toBool();

    const double scale_ratio = SettingValue("com.deepin.xsettings", QByteArray(), "scale-factor", 1.0).toDouble();;

    const bool isRemovable = m_currentModelIndex.data(AppsListModel::AppIsRemovableRole).toBool();
    const bool hideOpen = m_currentModelIndex.data(AppsListModel::AppHideOpenRole).toBool();
    const bool hideSendToDesktop = m_currentModelIndex.data(AppsListModel::AppHideSendToDesktopRole).toBool();
    const bool hideSendToDock = m_currentModelIndex.data(AppsListModel::AppHideSendToDockRole).toBool();
    const bool hideStartUp = m_currentModelIndex.data(AppsListModel::AppHideStartUpRole).toBool();
    const bool hideUninstall = m_currentModelIndex.data(AppsListModel::AppHideUninstallRole).toBool();
    const bool hideUseProxy = m_currentModelIndex.data(AppsListModel::AppHideUseProxyRole).toBool();
    const bool canOpen = m_currentModelIndex.data(AppsListModel::AppCanOpenRole).toBool();
    const bool canSendToDesktop = m_currentModelIndex.data(AppsListModel::AppCanSendToDesktopRole).toBool();
    const bool canSendToDock = m_currentModelIndex.data(AppsListModel::AppCanSendToDockRole).toBool();
    const bool canStartUp = m_currentModelIndex.data(AppsListModel::AppCanStartUpRole).toBool();
    const bool canDisableScale = m_calcUtil->IsServerSystem || qFuzzyCompare(1.0, scale_ratio);
    const bool canUseProxy = m_currentModelIndex.data(AppsListModel::AppCanOpenProxyRole).toBool();

    qDebug() << "appKey" << m_appKey;

    QAction *open;
    QAction *desktop;
    QAction *dock;
    QAction *startup;
    QAction *proxy;
    QAction *uninstall;

    open = new QAction(tr("Open"), menu);
    desktop = new QAction(m_isItemOnDesktop ? tr("Remove from desktop") : tr("Send to desktop"), menu);
    dock = new QAction(m_isItemOnDock ? tr("Remove from dock") : tr("Send to dock"), menu);
    startup = new QAction(m_isItemStartup ? tr("Remove from startup") : tr("Add to startup"), menu);
    uninstall = new QAction(tr("Uninstall"), menu);
    proxy = new QAction(tr("Use a proxy"), menu);

    // 分割线绘制的必要条件是，在打开功能之后，还有其他的功能选项
    if (!hideOpen) {
        menu->addAction(open);
#ifndef WITHOUT_UNINSTALL_APP
        if (!hideSendToDesktop || !hideSendToDock || !hideStartUp || !hideUseProxy || !hideUninstall || !canDisableScale)
#else
        if (!hideSendToDesktop || !hideSendToDock || !hideStartUp || !hideUseProxy || !canDisableScale)
#endif

            menu->addSeparator();
    }

    if (!hideSendToDesktop)
        menu->addAction(desktop);

    if (!hideSendToDock)
        menu->addAction(dock);

    // 分割线绘制的必要条件是，在发送到桌面或者发送到任务栏功能之后，还有其他的功能选项
    if (!hideOpen ||!hideSendToDesktop || !hideSendToDock) {
#ifndef WITHOUT_UNINSTALL_APP
        if (!hideStartUp || !hideUseProxy || !hideUninstall || !canDisableScale) {
#else
        if (!hideStartUp || !hideUseProxy || !canDisableScale) {
#endif
            menu->addSeparator();
        }
    }

    if (!hideStartUp)
        menu->addAction(startup);

    if (!hideUseProxy)
        menu->addAction(proxy);

    if (!canDisableScale) {
        QAction *scale = new QAction(tr("Disable display scaling"), menu);
        scale->setCheckable(true);
        scale->setChecked(!m_isItemEnableScaling);
        menu->addAction(scale);
        signalMapper->setMapping(scale, SwitchScale);        connect(scale, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    }

    open->setEnabled(canOpen);
    desktop->setEnabled(canSendToDesktop);
    dock->setEnabled(m_appKey != "dde-trash" && canSendToDock);
    startup->setEnabled(canStartUp);
    uninstall->setEnabled(isRemovable);
    proxy->setCheckable(true);
    proxy->setChecked(m_isItemProxy);
    proxy->setEnabled(canUseProxy);

#ifndef WITHOUT_UNINSTALL_APP
    if (!hideUninstall)
        menu->addAction(uninstall);
#endif

    if (!hideOpen)
        connect(open, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    if (!hideSendToDesktop)
        connect(desktop, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    if (!hideSendToDock)
        connect(dock, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    if (!hideStartUp)
        connect(startup, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
#ifndef WITHOUT_UNINSTALL_APP
    if (!hideUninstall)
        connect(uninstall, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
#endif
    if (!hideUseProxy)
        connect(proxy, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

    if (!hideOpen)
        signalMapper->setMapping(open, Open);
    if (!hideSendToDesktop)
        signalMapper->setMapping(desktop, Desktop);
    if (!hideSendToDock)
        signalMapper->setMapping(dock, Dock);
    if (!hideStartUp)
        signalMapper->setMapping(startup, Startup);
#ifndef WITHOUT_UNINSTALL_APP
    if (!hideUninstall)
        signalMapper->setMapping(uninstall, Uninstall);
#endif
    if (!hideUseProxy)
        signalMapper->setMapping(proxy, Proxy);
}

bool MenuWorker::isMenuVisible()
{
    if (m_menu)
        return m_menu->isVisible();
    else
        return false;
}

void MenuWorker::showMenuByAppItem(QPoint pos, const QModelIndex &index)
{
    setCurrentModelIndex(index);

    QSignalMapper *signalMapper = new QSignalMapper(m_menu);

    if (IS_WAYLAND_DISPLAY) {
        m_menu->setAttribute(Qt::WA_NativeWindow);
        m_menu->windowHandle()->setProperty("_d_dwayland_window-type", "session-shell");
    }

    m_menu->clear();
    creatMenuByAppItem(m_menu, signalMapper);

    connect(signalMapper, static_cast<void (QSignalMapper::*)(const int)>(&QSignalMapper::mapped), this, &MenuWorker::handleMenuAction);
    connect(m_menu, &QMenu::aboutToHide, this, &MenuWorker::handleMenuClosed);

    // 菜单超出当前屏幕范围时，菜单显示位置向上或者向左移动超出区域的差值
    qreal ratio = qApp->devicePixelRatio();
    QRect screenRect = m_appManager->instance()->currentScreen()->geometry();
    int widthToGlobal = screenRect.x() + screenRect.width();
    int heigthToGlobal = screenRect.y() + screenRect.height();
    int menuWidth = m_menu->sizeHint().width();
    int menuHeight = m_menu->sizeHint().height();

    if (heigthToGlobal <= pos.y() + menuHeight)
        pos = pos - QPoint(0, pos.y() + menuHeight - heigthToGlobal);
    else if (widthToGlobal <= pos.x() + menuWidth)
        pos = pos - QPoint(pos.x() + menuWidth - widthToGlobal, 0);
    else if ((heigthToGlobal <= pos.y() + menuHeight) || (widthToGlobal <= pos.x() + menuWidth))
        pos = pos - QPoint(pos.x() + menuWidth - widthToGlobal, pos.y() + menuHeight - heigthToGlobal);

    m_menu->move(pos);

    m_menu->show();
    m_menu->raise();

    // 保存右键菜单实际的物理大小(已将屏幕缩放考虑在内)
    m_menuIsShown = true;
    QPoint topLeft = (m_menu->geometry().topLeft() - screenRect.topLeft()) * ratio + screenRect.topLeft();
    m_menuGeometry = QRect(topLeft, m_menu->size() * ratio);
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
    // 隐藏右键菜单
    onHideMenu();

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

void MenuWorker::onHideMenu()
{
    if (m_menu && m_menu->isVisible())
        m_menu->hide();
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
