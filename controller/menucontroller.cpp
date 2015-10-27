#include "menucontroller.h"
#include "dbusinterface/menumanager_interface.h"
#include "dbusinterface/menu_interface.h"
#include "dbusinterface/launcher_interface.h"
#include "dbusinterface/dbusdockedappmanager.h"
#include "dbusinterface/startmanager_interface.h"
#include "app/global.h"
#include "dbusinterface/dbustype.h"
#include "Logger.h"
#include "dbuscontroller.h"
#include "dbusinterface/notification_interface.h"
#include "widgets/themeappicon.h"
#include "widgets/util.h"
#include <QDBusObjectPath>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDBusPendingReply>
#include <QVariant>
#include <QProcess>

MenuController::MenuController(QObject *parent) : QObject(parent)
{
    m_menuManagerInterface = new MenumanagerInterface(MenuManager_service, MenuManager_path, QDBusConnection::sessionBus(), this);
    m_dockAppManagerInterface = new DBusDockedAppManager(this);
    m_notifcationInterface = new NotificationInterface(NotificationInterface::staticServiceName(),
                                                       NotificationInterface::staticObjectPathName(),
                                                       QDBusConnection::sessionBus(),
                                                       this);
    m_menuInterface = NULL;
    initConnect();
}


void MenuController::initConnect(){
    connect(signalManager, SIGNAL(contextMenuShowed(QString,QPoint)),
            this, SLOT(showMenuByAppItem(QString,QPoint)));
    connect(signalManager, SIGNAL(appOpened(QString)), this, SLOT(handleOpen(QString)));
}

MenuController::~MenuController()
{
}


void MenuController::showMenuByAppItem(QString appKey, QPoint pos){
    m_appKeyRightClicked = appKey;
    QString menuContent = createMenuContent(appKey);
    QString menuJsonContent = JsonToQString(pos, menuContent);
    QString menuDBusObjectpath = registerMenu();
    if (menuDBusObjectpath.length() > 0){
        showMenu(menuDBusObjectpath, menuJsonContent);
    }else{
        qCritical() << "register menu fail!";
    }
}

QString MenuController::createMenuContent(QString appKey){
    m_isItemOnDesktop = isItemOnDesktop(appKey);
    m_isItemOnDock = isItemOnDock(appKey);
    m_isItemStartup = isItemStartup(appKey);

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
        startupObj = createMenuItem(3, tr("remove from startup(_R)"));
    }else{
        startupObj = createMenuItem(3, tr("Add to startup(_A)"));
    }
    QJsonObject uninstallObj = createMenuItem(4, tr("Uninstall"));

    QJsonArray items;
    items.append(openObj);
    items.append(seperatorObj1);
    items.append(desktopObj);
    items.append(dockObj);
    items.append(seperatorObj2);
    items.append(startupObj);
    items.append(uninstallObj);

    QJsonObject menuObj;
    menuObj["checkableMenu"] = false;
    menuObj["singleCheck"] = false;
    menuObj["items"] = items;

    return QString(QJsonDocument(menuObj).toJson());
}

QJsonObject MenuController::createMenuItem(int itemId, QString itemText){
    QJsonObject itemObj;
    itemObj["itemId"] = QString::number(itemId);
    itemObj["itemText"] = itemText;
    itemObj["isActive"] = true;
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


QJsonObject MenuController::createSeperator(){
    return createMenuItem(-100, "");
}


bool MenuController::isItemOnDesktop(QString appKey){
    bool flag = false;
    QDBusPendingReply<bool> reply = dbusController->getLauncherInterface()->IsItemOnDesktop(appKey);
    reply.waitForFinished();
    if (!reply.isError()){
        flag = reply.argumentAt(0).toBool();
    } else {
        qCritical() << reply.error().message();
    }
    qDebug() << appKey << flag;
    return flag;
}

bool MenuController::isItemOnDock(QString appKey){
    bool flag = false;
    QDBusPendingReply<bool> reply = m_dockAppManagerInterface->IsDocked(appKey);
    reply.waitForFinished();
    if (!reply.isError()){
        flag = reply.argumentAt(0).toBool();
    } else {
        qCritical() << reply.error().message();
    }
    return flag;
}

bool MenuController::isItemStartup(QString appKey){
    QString url = dbusController->getItemInfo(appKey).url;
    bool flag = false;
    QDBusPendingReply<bool> reply = dbusController->getStartManagerInterface()->IsAutostart(url);
    reply.waitForFinished();
    if (!reply.isError()){
        flag = reply.argumentAt(0).toBool();
    } else {
        qCritical() << reply.error().message();
    }
    return flag;
}


QString MenuController::JsonToQString(QPoint pos, QString menucontent) {
    QJsonObject menuObj;
    menuObj["x"] = pos.x();
    menuObj["y"] = pos.y();
    menuObj["isDockMenu"] = false;
    menuObj["menuJsonContent"] = menucontent;
    return QString(QJsonDocument(menuObj).toJson());
}

QString MenuController::registerMenu() {
    QDBusPendingReply<QDBusObjectPath> reply = m_menuManagerInterface->RegisterMenu();
    reply.waitForFinished();
    if (!reply.isError()) {
        return reply.value().path();
    } else {
        return "";
    }
}

void MenuController::showMenu(QString menuDBusObjectPath, QString menuContent) {
    m_menuInterface = new MenuInterface(MenuManager_service, menuDBusObjectPath, QDBusConnection::sessionBus(), this);
    m_menuInterface->ShowMenu(menuContent);
    connect(m_menuInterface, SIGNAL(ItemInvoked(QString, bool)),this, SLOT(menuItemInvoked(QString,bool)));
    connect(m_menuInterface, SIGNAL(MenuUnregistered()), this, SLOT(handleMenuClosed()));
    connect(m_menuInterface, SIGNAL(MenuUnregistered()), m_menuInterface, SLOT(deleteLater()));
    connect(signalManager, SIGNAL(uninstallActionChanged(int)), this, SLOT(handleUninstallAction(int)));
}

void MenuController::menuItemInvoked(QString itemId, bool flag){
    Q_UNUSED(flag)
    int id = itemId.toInt();
    qDebug() << "menuItemInvoked" << itemId;
    switch (id) {
    case 0:
        handleOpen(m_appKeyRightClicked);
        break;
    case 1:
        handleToDesktop(m_appKeyRightClicked);
        break;
    case 2:
        handleToDock(m_appKeyRightClicked);
        break;
    case 3:
        handleToStartup(m_appKeyRightClicked);
        break;
    case 4:
        emit signalManager->appUninstalled(m_appKeyRightClicked);
        break;
    default:
        break;
    }
}


void MenuController::handleOpen(QString appKey){
    emit signalManager->Hide();
    QString url = dbusController->getItemInfo(appKey).url;
    qDebug() << "handleOpen" << appKey << url;
    QDBusPendingReply<bool> reply = dbusController->getStartManagerInterface()->Launch(appKey);
    reply.waitForFinished();
    if (!reply.isError()) {
        bool ret = reply.argumentAt(0).toBool();
        qDebug() << "Launch app:" << ret;
        if (ret){
            dbusController->getLauncherInterface()->MarkLaunched(appKey);
            dbusController->getLauncherInterface()->RecordFrequency(appKey);
        }
    } else {
        qCritical() << reply.error().message();
    }
}

void MenuController::handleMenuClosed(){
    emit signalManager->rightClickedChanged(false);
}

void MenuController::handleToDesktop(QString appKey){
    qDebug() << "handleToDesktop" << appKey;
    if (m_isItemOnDesktop){
        QDBusPendingReply<bool> reply = dbusController->getLauncherInterface()->RequestRemoveFromDesktop(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from desktop:" << ret;
        } else {
            qCritical() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply = dbusController->getLauncherInterface()->RequestSendToDesktop(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "send to desktop:" << ret;
        } else {
            qCritical() << reply.error().message();
        }
    }
}

void MenuController::handleToDock(QString appKey){
    qDebug() << "handleToDock" << appKey;
    if (m_isItemOnDock){
        QDBusPendingReply<bool> reply = m_dockAppManagerInterface->RequestUndock(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from dock:" << ret;
        } else {
            qCritical() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply =  m_dockAppManagerInterface->ReqeustDock(appKey, "", "", "");
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "send to dock:" << ret;
        } else {
            qCritical() << reply.error().message();
        }
    }
}

void MenuController::handleToStartup(QString appKey){
    QString url = dbusController->getItemInfo(appKey).url;
    qDebug() << "handleToStartup" << appKey << url;

    if (m_isItemStartup){
        QDBusPendingReply<bool> reply = dbusController->getStartManagerInterface()->RemoveAutostart(url);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from startup:" << ret;
            if (ret) {
                emit signalManager->hideAutoStartLabel(appKey);
            }
        } else {
            qCritical() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply =  dbusController->getStartManagerInterface()->AddAutostart(url);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "add to startup:" << ret;
            if (ret){
                emit signalManager->showAutoStartLabel(appKey);
            }
        } else {
            qCritical() << reply.error().message();
        }
    }
}

void MenuController::handleUninstallAction(int id){
    switch (id) {
    case 0:
        break;
    case 1:
        startUnistall(m_appKeyRightClicked);
        break;
    default:
        break;
    }
}

void MenuController::startUnistall(QString appKey){
    QDBusPendingReply<> reply = dbusController->getLauncherInterface()->RequestUninstall(appKey, false);
    reply.waitForFinished();
    if (!reply.isError()) {
        qDebug() << "unistall action finished!";
        handleUninstallSuccess(appKey);
    } else {
        qDebug() << "unistall action fail";
    }
}

void MenuController::handleUninstallSuccess(const QString &appKey){
    QString cachePath = joinPath(getThumbnailsPath(), QString("%1.png").arg(appKey));
    QString message = tr("Uninstall %1 successfully!").arg(appKey);
//    m_notifcationInterface->Notify("dde-launcher",
//                                   0,
//                                   cachePath,
//                                   summary,
//                                   message,
//                                    QStringList(),
//                                    QVariantMap(),
//                                   0);
    QString command = QString("notify-send \"%1\" -a %2 -t 5 -i \"%3\"").arg(message , appKey, cachePath);

    QProcess::execute(command);
    qDebug() << "handleUninstallSuccess" << appKey << cachePath << command;
    emit signalManager->itemDeleted(appKey);
}

void MenuController::handleUninstallFail(const QString &appKey, const QString &message){
    qDebug() << "handleUninstallFail" << appKey << message;
}
