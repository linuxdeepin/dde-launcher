#include "appitemmanager.h"
#include "dbusinterface/dbustype.h"
#include "fwd.h"
#include "app/global.h"
#include "Logger.h"

AppItemManager::AppItemManager(QObject *parent) : QObject(parent)
{

}

void AppItemManager::init(){
    initConnect();
}

void AppItemManager::initConnect(){
    connect(signalManager, SIGNAL(itemInfosChanged(QMap<QString,ItemInfo>)), this, SLOT(createAppItems(QMap<QString,ItemInfo>)));
}

void AppItemManager::createAppItem(const ItemInfo &itemInfo, int index){\
    AppItemPointer pAppItem =  AppItemPointer::create(itemInfo.url, itemInfo.icon, itemInfo.name);
    m_appItems.insert(itemInfo.name, pAppItem);
}

void AppItemManager::createAppItems(const QMap<QString, ItemInfo> &itemInfos){
    const QList<ItemInfo>& itemInfoList = itemInfos.values();
    for (int i=0; i< itemInfoList.length(); i++) {
        createAppItem(itemInfoList.at(i), i);
    }
}

QMap<QString, AppItemPointer> AppItemManager::getAppItems(){
    return m_appItems;
}

QList<AppItemPointer>  AppItemManager::getAppItemsByNameSorted(){
    LOG_INFO() << m_appItems.values().length();
    return m_appItems.values();
}


QList<AppItemPointer>  AppItemManager::getAppItemsByInstalledTime(){
    return m_appItems.values();
}

QList<AppItemPointer>  AppItemManager::getAppItemsByUseFrequency(){
    return m_appItems.values();
}


AppItemManager::~AppItemManager()
{

}

