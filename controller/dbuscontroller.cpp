#include "dbuscontroller.h"
#include "dbusinterface/launcher_interface.h"
#include "dbusinterface/fileInfo_interface.h"
#include "Logger.h"
#include "app/global.h"

#include <QtCore>
#include <QtDBus>

DBusController::DBusController(QObject *parent) : QObject(parent)
{
    m_launcherInterface = new LauncherInterface(Launcher_service, Launcher_path, QDBusConnection::sessionBus(), this);
    m_fileInfoInterface = new FileInfoInterface(FileInfo_service, FileInfo_path, QDBusConnection::sessionBus(), this);
}

void DBusController::init(){
    getCategoryInfoList();
    getInstalledTimeItems();
    getAllFrequencyItems();

}

void DBusController::getCategoryInfoList(){
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

