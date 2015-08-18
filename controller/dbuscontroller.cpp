#include "dbuscontroller.h"
#include "dbusinterface/launcher_interface.h"
#include "dbusinterface/fileInfo_interface.h"
#include "dbusinterface/startmanager_interface.h"
#include "Logger.h"
#include "app/global.h"
#include "controller/menucontroller.h"

#include <QtCore>
#include <QtDBus>

DBusController::DBusController(QObject *parent) : QObject(parent)
{
    m_launcherInterface = new LauncherInterface(Launcher_service, Launcher_path, QDBusConnection::sessionBus(), this);
    m_fileInfoInterface = new FileInfoInterface(FileInfo_service, FileInfo_path, QDBusConnection::sessionBus(), this);
    m_startManagerInterface = new StartManagerInterface(StartManager_service, StartManager_path, QDBusConnection::sessionBus(), this);
    m_menuController = new MenuController(this);
    initConnect();
}

void DBusController::init(){
    getCategoryInfoList();
    getInstalledTimeItems();
    getAllFrequencyItems();

}

void DBusController::initConnect(){
    connect(m_launcherInterface, SIGNAL(UninstallSuccess(QString)),
            m_menuController, SLOT(handleUninstallSuccess(QString)));
    connect(m_launcherInterface, SIGNAL(UninstallFailed(QString,QString)),
            m_menuController, SLOT(handleUninstallFail(QString,QString)));
    connect(signalManager, SIGNAL(itemDeleted(QString)), this, SLOT(updateAppTable(QString)));
}

void DBusController::updateAppTable(QString appKey){
    LOG_INFO() << "updateAppTable" << appKey;
    init();
}

LauncherInterface* DBusController::getLauncherInterface(){
    return m_launcherInterface;
}

StartManagerInterface* DBusController::getStartManagerInterface(){
    return m_startManagerInterface;
}

void DBusController::getCategoryInfoList(){
    m_appIcons.clear();
    m_itemInfos.clear();
    QDBusPendingReply<CategoryInfoList> reply = m_launcherInterface->GetAllCategoryInfos();
    reply.waitForFinished();
    if (!reply.isError()){
        m_categoryInfoList = qdbus_cast<CategoryInfoList>(reply.argumentAt(0));
        foreach (CategoryInfo item, m_categoryInfoList) {
            if (item.key == "all" && item.id == -1){
                foreach (QString appKey, item.items){
                    ItemInfo itemInfo= getItemInfo(appKey);
                    QString icon = getThemeIconByAppKey(itemInfo.iconKey, 48);
                    itemInfo.icon = icon;
                    m_appIcons.insert(appKey, icon);
                    m_itemInfos.insert(appKey, itemInfo);
                }
                emit signalManager->itemInfosChanged(m_itemInfos);
            }
        }
        emit signalManager->categoryInfosChanged(m_categoryInfoList);
    }else{
        LOG_ERROR() << reply.error().message();
    }
}

void DBusController::getAllFrequencyItems(){
    QDBusPendingReply<AppFrequencyInfoList> reply = m_launcherInterface->GetAllFrequency();
    reply.waitForFinished();
    if (!reply.isError()){
        m_appFrequencyInfoList = qdbus_cast<AppFrequencyInfoList>(reply.argumentAt(0));
        foreach (AppFrequencyInfo item, m_appFrequencyInfoList) {
//            LOG_INFO() << item.key << item.count;
        }
    }else{
        LOG_ERROR() << reply.error().message();
    }
}

void DBusController::getInstalledTimeItems(){
    QDBusPendingReply<AppInstalledTimeInfoList> reply = m_launcherInterface->GetAllTimeInstalled();
    reply.waitForFinished();
    if (!reply.isError()){
        m_appInstalledTimeInfoList = qdbus_cast<AppInstalledTimeInfoList>(reply.argumentAt(0));
        foreach (AppInstalledTimeInfo item, m_appInstalledTimeInfoList) {
//            LOG_INFO() << item.key << item.intsalledTime;
        }
    }else{
        LOG_ERROR() << reply.error().message();
    }
}


ItemInfo DBusController::getItemInfo(QString appKey){
    QDBusPendingReply<ItemInfo> reply = m_launcherInterface->GetItemInfo(appKey);
    ItemInfo  itemInfo;
    reply.waitForFinished();
    if (!reply.isError()){
        itemInfo = qdbus_cast<ItemInfo>(reply.argumentAt(0));
        return itemInfo;
    }else{
        LOG_ERROR() << reply.error().message();
        return itemInfo;
    }
}

QString DBusController::getThemeIconByAppKey(QString appKey, int size){
    QDBusPendingReply<QString> reply = m_fileInfoInterface->GetThemeIcon(appKey, size);
    reply.waitForFinished();
    if (!reply.isError()){
        QString iconUrl = reply.argumentAt(0).toString();
        return iconUrl;
    }else{
        LOG_ERROR() << reply.error().message();
        return "";
    }
}

DBusController::~DBusController()
{

}

