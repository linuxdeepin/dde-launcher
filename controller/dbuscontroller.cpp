#include "dbuscontroller.h"
#include "dbusinterface/launcher_interface.h"
#include "dbusinterface/fileInfo_interface.h"
#include "dbusinterface/startmanager_interface.h"
#include "Logger.h"
#include "app/global.h"
#include "controller/menucontroller.h"

#include <QtCore>
#include <QtDBus>

bool appNameLessThan(const ItemInfo &info1, const ItemInfo &info2)
{
    return info1.name < info2.name;
}

bool installTimeLessThan(const ItemInfo &info1, const ItemInfo &info2)
{
    return info1.intsalledTime < info2.intsalledTime;
}

bool useFrequencyMoreThan(const AppFrequencyInfo &info1, const AppFrequencyInfo &info2){
    return info1.count > info2.count;
}

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
    m_itemInfos.clear();
    QDBusPendingReply<CategoryInfoList> reply = m_launcherInterface->GetAllCategoryInfos();
    reply.waitForFinished();
    if (!reply.isError()){
        m_categoryInfoList = qdbus_cast<CategoryInfoList>(reply.argumentAt(0));
        foreach (CategoryInfo item, m_categoryInfoList) {
            if (item.key == "all" && item.id == -1){
                foreach (QString appKey, item.items){
                    ItemInfo itemInfo= getItemInfo(appKey);
                    m_itemInfos.insert(appKey, itemInfo);
                }
                emit signalManager->itemInfosChanged(m_itemInfos);
            }
        }

        sortedByAppName(m_itemInfos.values());
        foreach (CategoryInfo item, m_categoryInfoList) {
            if (item.key != "all" && item.id != -1){
                QStringList appKeys;
                foreach (ItemInfo itemInfo, m_appNameSortedList) {
                    if (item.items.contains(itemInfo.key)){
                        appKeys.append(itemInfo.key);
                    }
                }
                item.items = appKeys;
            }
            m_categoryAppNameSortedInfoList.append(item);
        }
        emit signalManager->categoryInfosChanged(m_categoryAppNameSortedInfoList);
    }else{
        LOG_ERROR() << reply.error().message();
    }
}


void DBusController::sortedByAppName(QList<ItemInfo> infos){
    std::sort(infos.begin(), infos.end(), appNameLessThan);
    m_appNameSortedList = infos;
    emit signalManager->appNameItemInfoListChanged(m_appNameSortedList);
}

void DBusController::sortedByInstallTime(QList<ItemInfo> infos){
    std::sort(infos.begin(), infos.end(), installTimeLessThan);
    m_installTimeSortedList = infos;
    emit signalManager->installTimeItemInfoListChanged(m_installTimeSortedList);
}

void DBusController::getAllFrequencyItems(){
    QDBusPendingReply<AppFrequencyInfoList> reply = m_launcherInterface->GetAllFrequency();
    reply.waitForFinished();
    if (!reply.isError()){
        m_appFrequencyInfoList = qdbus_cast<AppFrequencyInfoList>(reply.argumentAt(0));
        std::sort(m_appFrequencyInfoList.begin(), m_appFrequencyInfoList.end(), useFrequencyMoreThan);
        for(int i=0; i< m_appFrequencyInfoList.count(); i++){
            QString key = m_appFrequencyInfoList.at(i).key;
            if (m_itemInfos.contains(key)){
                m_useFrequencySortedList.append(m_itemInfos.value(key));
            }
        }
        emit signalManager->useFrequencyItemInfoListChanged(m_useFrequencySortedList);
    }else{
        LOG_ERROR() << reply.error().message();
    }


}

void DBusController::getInstalledTimeItems(){
    sortedByInstallTime(m_itemInfos.values());
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

DBusController::~DBusController()
{

}

