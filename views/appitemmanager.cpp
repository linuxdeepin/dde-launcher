/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "appitemmanager.h"
#include "appitem.h"
#include "categoryitem.h"
#include "widgets/themeappicon.h"
#include "app/global.h"
#include "launcherframe.h"

AppItemManager::AppItemManager(QObject *parent) : QObject(parent)
{

}

AppItemManager::~AppItemManager()
{

}

void AppItemManager::init()
{

    m_delayCreateItemsTimer = new QTimer;
    m_delayCreateItemsTimer->setSingleShot(true);
    m_delayCreateItemsTimer->setInterval(500);
    initConnect();
}

void AppItemManager::initConnect(){
    connect(signalManager, SIGNAL(firstLoadSortedMode(int)),
            this, SLOT(setFirstLoadSortedMode(int)));
    connect(signalManager, SIGNAL(firstLoadCategoryMode(int)),
            this, SLOT(setFirstLoadCategoryMode(int)));
    connect(signalManager, SIGNAL(showFirstShowApps()),
            this, SLOT(createFirstShowAppItems()));
    connect(m_delayCreateItemsTimer, SIGNAL(timeout()),
            this, SLOT(delayCreateOtherItems()));
    connect(signalManager, SIGNAL(itemInfosChanged(QMap<QString,ItemInfo>)),
            this, SLOT(setItemInfos(QMap<QString,ItemInfo>)));
    connect(signalManager, SIGNAL(appNameItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(setAppNameInfoList(QList<ItemInfo>)));
    connect(signalManager, SIGNAL(installTimeItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(setInstallTimeInfoList(QList<ItemInfo>)));
    connect(signalManager, SIGNAL(useFrequencyItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(setUseFrequencyInfoList(QList<ItemInfo>)));
    connect(signalManager, SIGNAL(categoryInfosChanged(CategoryInfoList)),
            this, SLOT(setCategoryInfoList(CategoryInfoList)));
    connect(signalManager, SIGNAL(newinstalllindicatorHided(QString)),
            this, SLOT(hideNewInstallIndicator(QString)));
    connect(signalManager, SIGNAL(newinstalllindicatorShowed(QString)),
            this, SLOT(showNewInstallIndicator(QString)));
    connect(signalManager, SIGNAL(iconRefreshed(ItemInfo)),
            this, SLOT(refreshIcon(ItemInfo)));
}

QMap<QString, ItemInfo>& AppItemManager::getItemInfos(){
    return m_itemInfos;
}

QList<ItemInfo>& AppItemManager::getAppNameItemInfos(){
    return m_appNameItemInfoList;
}

QList<ItemInfo>& AppItemManager::getInstallTimeItemInfos(){
    return m_installtimeItemInfoList;
}

QList<ItemInfo>& AppItemManager::getUseFrequencyItemInfos(){
    return m_useFrequencyItemInfoList;
}

QList<qlonglong>& AppItemManager::getHideKeys(){
    return m_hideKeys;
}

CategoryInfoList &AppItemManager::getCategoryInfoList()
{
    return m_categoryInfoList;
}

CategoryInfoList& AppItemManager::getSortedCategoryInfoList(){
    return m_sortedCategoryInfoList;
}

QMap<QString, AppItem *> &AppItemManager::getAppItems()
{
    return m_appItems;
}

AppItem* AppItemManager::getAppItemByKey(const QString &key)
{
    if (m_appItems.contains(key)){
        return m_appItems.value(key);
    }
    return NULL;
}

QMap<qlonglong, CategoryItem *> &AppItemManager::getCategoryItems()
{
    return m_categoryItems;
}

CategoryItem* AppItemManager::getCategoryItemByKey(const qlonglong key)
{
    if (m_categoryItems.contains(key)){
        return m_categoryItems.value(key);
    }
    return NULL;
}

void AppItemManager::setFirstLoadSortedMode(int mode)
{
    m_sortMode = mode;
    qDebug() << "appManager sortMode:" << mode;
}

void AppItemManager::setFirstLoadCategoryMode(int mode)
{
    m_categoryMode = mode;
}

void AppItemManager::createFirstShowAppItems()
{
    for(int i=0; i< LauncherFrame::ColumnCount * LauncherFrame::RowCount; i++){
        if (m_sortMode == 0){
            if (m_appNameItemInfoList.size() > i)
                addItem(m_appNameItemInfoList[i]);
        }else if (m_sortMode == 2){
            if (m_installtimeItemInfoList.size() > i)
                addItem(m_installtimeItemInfoList[i]);
        }else if (m_sortMode == 3){
            if (m_useFrequencyItemInfoList.size() > i)
                addItem(m_useFrequencyItemInfoList[i]);
        }
    }
    for(int i=0; i< 4; i++){
        for (int j=0; j < m_categoryInfoList.count(); j++){
            if (m_categoryInfoList.at(j).id == i){
                foreach (QString key, m_categoryInfoList.at(j).items) {
                    addItem(m_itemInfos.value(key));
                }
            }
        }
    }
    m_delayCreateItemsTimer->start();
    qDebug() << "appitemManager sortMode:" << m_sortMode;
}

void AppItemManager::delayCreateOtherItems()
{
    addItems(m_itemInfos.values());
    if (m_sortMode == 0){
        emit signalManager->viewModeChanged(0);
    }else if (m_sortMode == 1){
        qDebug() << "AppItemManager categoryMode:" << m_categoryMode;
        emit signalManager->viewModeChanged(m_categoryMode + 1);
    }else if (m_sortMode == 2 || m_sortMode == 3){
        emit signalManager->viewModeChanged(m_sortMode + 1);
    }
}

void AppItemManager::addCategoryItem(const CategoryInfo &info)
{
    int categoryID = int(info.id);
    if (m_categoryItems.contains(categoryID)){
        return;
    }else{
        if (CategoryID(categoryID) != CategoryID::All){
            qDebug() << "add category item" << info.name << info.id;
        CategoryItem* categoryItem = new CategoryItem(info.name);
            m_categoryItems.insert(categoryID, categoryItem);
        }
    }
}

void AppItemManager::addItem(const ItemInfo &itemInfo){
    if (m_appItems.contains(itemInfo.key)){
        AppItem* appItem = m_appItems.value(itemInfo.key);
//        qDebug() << DBusController::PreInstallAppKeys.contains(itemInfo.key);
        if (DBusController::PreInstallAppKeys.contains(itemInfo.key)){
            appItem->setNewInstalled(true);
        }else{
            appItem->setNewInstalled(false);
        }
    }else{
        AppItem* appItem = new AppItem(itemInfo.isAutoStart);
        appItem->setAppKey(itemInfo.key);
        appItem->setAppIconKey(itemInfo.iconKey);
        appItem->setUrl(itemInfo.url);
        appItem->setAppName(itemInfo.name);
        int size = appItem->getIconSize();
        appItem->setAppIcon(ThemeAppIcon::getIconPixmap(itemInfo.iconKey, size, size));
        m_appItems.insert(itemInfo.key, appItem);
    }
}

void AppItemManager::addItems(const QList<ItemInfo> &itemInfos){
    for (int i=0; i< itemInfos.length(); i++) {
        addItem(itemInfos[i]);
    }
}

void AppItemManager::setItemInfos(const QMap<QString, ItemInfo> &infos){
    qDebug() << __func__<< infos.count();
    m_itemInfos = infos;
//    addItems(infos.values());
}

void AppItemManager::setAppNameInfoList(const QList<ItemInfo> &infos){
    qDebug() << "setAppNameInfoList";
    m_appNameItemInfoList = infos;
}

void AppItemManager::setInstallTimeInfoList(const QList<ItemInfo> &infos){
    qDebug() << "setInstallTimeInfoList";
    m_installtimeItemInfoList = infos;
}

void AppItemManager::setUseFrequencyInfoList(const QList<ItemInfo> &infos){
    qDebug() << "setUseFrequencyInfoList";
    m_useFrequencyItemInfoList = infos;
//    addItems(infos);
}

void AppItemManager::setCategoryInfoList(const CategoryInfoList &categoryInfoList)
{
    m_categoryInfoList = categoryInfoList;
    m_sortedCategoryInfoList.clear();
    m_hideKeys.clear();
    for(int i = 0; i< CategoryKeys.size(); i++ ) {
        foreach (CategoryInfo info, categoryInfoList) {
            if (info.id == i){
                m_sortedCategoryInfoList.append(info);
            }
        }
    }
    foreach (CategoryInfo info, categoryInfoList) {
        if (info.id == -2){
            m_sortedCategoryInfoList.append(info);
        }
    }

    foreach (CategoryInfo info, m_sortedCategoryInfoList) {
        if (info.items.count() == 0){
            m_hideKeys.append(info.id);
            qDebug() << "hide" << info.name;
        }
    }

    foreach (CategoryInfo info, categoryInfoList) {
        if(info.items.count() > 0){
            addCategoryItem(info);
        }
    }
}

void AppItemManager::hideNewInstallIndicator(QString appKey)
{
    if (m_appItems.contains(appKey)){
        AppItem* appItem = m_appItems.value(appKey);
//        qDebug() << "hideNewInstallIndicator" << appKey;
        appItem->setNewInstalled(false);
    }
}

void AppItemManager::showNewInstallIndicator(QString appKey)
{
    if (m_appItems.contains(appKey)){
        AppItem* appItem = m_appItems.value(appKey);
        appItem->setNewInstalled(true);
    }
}

void AppItemManager::refreshIcon(const ItemInfo& itemInfo)
{
    if (m_appItems.contains(itemInfo.key)){
        AppItem* appItem = m_appItems.value(itemInfo.key);
        int size = appItem->getIconSize();
        appItem->setAppIcon(ThemeAppIcon::getIconPixmap(itemInfo.iconKey, size, size));
    }
}
