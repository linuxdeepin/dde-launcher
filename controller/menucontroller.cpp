#include "menucontroller.h"
#include "dbusinterface/menumanager_interface.h"
#include "dbusinterface/menu_interface.h"
#include "dbusinterface/launcher_interface.h"
#include "dbusinterface/dbusdockedappmanager.h"
#include "dbusinterface/startmanager_interface.h"
#include "app/global.h"
#include "dbusinterface/dbustype.h"
#include "dialogs/confirmuninstalldialog.h"
#include "Logger.h"
#include "dbuscontroller.h"
#include <QDBusObjectPath>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDBusPendingReply>
#include <QVariant>

MenuController::MenuController(QObject *parent) : QObject(parent)
{
    m_menuManagerInterface = new MenumanagerInterface(MenuManager_service, MenuManager_path, QDBusConnection::sessionBus(), this);
    m_dockAppManagerInterface = new DBusDockedAppManager(this);
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
        LOG_ERROR() << "register menu fail!";
    }
}

QString MenuController::createMenuContent(QString appKey){
    m_isItemOnDesktop = isItemOnDesktop(appKey);
    m_isItemOnDock = isItemOnDock(appKey);
    m_isItemStartup = isItemStartup(appKey);

    QJsonObject openObj = createMenuItem(0, tr("Open(O)"));
    QJsonObject seperatorObj1 = createSeperator();
    QJsonObject desktopObj;
    if (m_isItemOnDesktop){
        desktopObj = createMenuItem(1, tr("Remove from desktop(F)"));
    }else{
        desktopObj = createMenuItem(1, tr("Send to desktop(E)"));
    }
    QJsonObject dockObj;
    if (m_isItemOnDock){
        dockObj = createMenuItem(2, tr("Remove from dock(F)"));
    }else{
        dockObj = createMenuItem(2, tr("Send to dock(E)"));
    }
    QJsonObject seperatorObj2 = createSeperator();
    QJsonObject startupObj;
    if (m_isItemStartup){
        startupObj = createMenuItem(3, tr("remove from startup(R)"));
    }else{
        startupObj = createMenuItem(3, tr("Add to startup(A)"));
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
        LOG_ERROR() << reply.error().message();
    }
    LOG_INFO() << appKey << flag;
    return flag;
}

bool MenuController::isItemOnDock(QString appKey){
    bool flag = false;
    QDBusPendingReply<bool> reply = m_dockAppManagerInterface->IsDocked(appKey);
    reply.waitForFinished();
    if (!reply.isError()){
        flag = reply.argumentAt(0).toBool();
    } else {
        LOG_ERROR() << reply.error().message();
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
        LOG_ERROR() << reply.error().message();
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
    connect(m_menuInterface, SIGNAL(MenuUnregistered()), m_menuInterface, SLOT(deleteLater()));
}

void MenuController::menuItemInvoked(QString itemId, bool flag){
    Q_UNUSED(flag)
    int id = itemId.toInt();
    LOG_INFO() << "menuItemInvoked" << itemId;
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
        handleUninstall(m_appKeyRightClicked);
        break;
    default:
        break;
    }
}


void MenuController::handleOpen(QString appKey){
    emit signalManager->Hide();
    QString url = dbusController->getItemInfo(appKey).url;
    LOG_INFO() << "handleOpen" << appKey << url;
    QDBusPendingReply<bool> reply = dbusController->getStartManagerInterface()->Launch(appKey);
    reply.waitForFinished();
    if (!reply.isError()) {
        bool ret = reply.argumentAt(0).toBool();
        LOG_INFO() << "Launch app:" << ret;

    } else {
    }
}

void MenuController::handleToDesktop(QString appKey){
    LOG_INFO() << "handleToDesktop" << appKey;
    if (m_isItemOnDesktop){
        QDBusPendingReply<bool> reply = dbusController->getLauncherInterface()->RequestRemoveFromDesktop(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            LOG_INFO() << "remove from desktop:" << ret;
        } else {
        }
    }else{
        QDBusPendingReply<bool> reply = dbusController->getLauncherInterface()->RequestSendToDesktop(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            LOG_INFO() << "send to desktop:" << ret;
        } else {
        }
    }
}

void MenuController::handleToDock(QString appKey){
    LOG_INFO() << "handleToDock" << appKey;
    if (m_isItemOnDock){
        QDBusPendingReply<bool> reply = m_dockAppManagerInterface->RequestUndock(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            LOG_INFO() << "remove from dock:" << ret;
        } else {
        }
    }else{
        QDBusPendingReply<bool> reply =  m_dockAppManagerInterface->ReqeustDock(appKey, "", "", "");
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            LOG_INFO() << "send to dock:" << ret;
        } else {
        }
    }
}

void MenuController::handleToStartup(QString appKey){
    QString url = dbusController->getItemInfo(appKey).url;
    LOG_INFO() << "handleToStartup" << appKey << url;

    if (m_isItemStartup){
        QDBusPendingReply<bool> reply = dbusController->getStartManagerInterface()->RemoveAutostart(url);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            LOG_INFO() << "remove from startup:" << ret;
        } else {
        }
    }else{
        QDBusPendingReply<bool> reply =  dbusController->getStartManagerInterface()->AddAutostart(url);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            LOG_INFO() << "add to startup:" << ret;
        } else {
        }
    }
}

void MenuController::handleUninstall(QString appKey){
    LOG_INFO() << "handleUninstall" << appKey;
    ConfirmUninstallDialog d;
    QString message = tr("Are you sure to uninstall ") + appKey;
    d.setMessage(message);
    connect(&d, SIGNAL(buttonClicked(int)), this, SLOT(handleUninstallAction(int)));
    int code = d.exec();
    qDebug() << code << "dialog close";
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
        LOG_INFO() << "unistall action finished!";
    } else {
        LOG_INFO() << "unistall action fail";
    }
}

void MenuController::handleUninstallSuccess(const QString &appKey){
    LOG_INFO() << "handleUninstallSuccess" << appKey;
    emit signalManager->itemDeleted(appKey);
}

void MenuController::handleUninstallFail(const QString &appKey, const QString &message){
    LOG_INFO() << "handleUninstallFail" << appKey << message;
}
