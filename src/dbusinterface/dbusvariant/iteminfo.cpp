/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "iteminfo.h"
#include <QDebug>

ItemInfo::ItemInfo()
    : m_openCount(0)
    , m_firstRunTime(0)
{
    m_isDir = false;
}

ItemInfo::ItemInfo(const ItemInfo &info)
    : m_desktop(info.m_desktop)
    , m_name(info.m_name)
    , m_key(info.m_key)
    , m_iconKey(info.m_iconKey)
    , m_categoryId(info.m_categoryId)
    , m_installedTime(info.m_installedTime)
    , m_openCount(info.m_openCount)
    , m_firstRunTime(info.m_firstRunTime)
    , m_isDir(info.m_isDir)
    , m_appInfoList(info.m_appInfoList)
{

}

ItemInfo::~ItemInfo()
{

}

void ItemInfo::registerMetaType()
{
    qRegisterMetaType<ItemInfo>("ItemInfo");
    qDBusRegisterMetaType<ItemInfo>();
    qRegisterMetaType<ItemInfoList>("ItemInfoList");
    qDBusRegisterMetaType<ItemInfoList>();
}

AppsListModel::AppCategory ItemInfo::category() const
{
    switch (m_categoryId)
    {
    case 0:     return AppsListModel::Internet;
    case 1:     return AppsListModel::Chat;
    case 2:     return AppsListModel::Music;
    case 3:     return AppsListModel::Video;
    case 4:     return AppsListModel::Graphics;
    case 5:     return AppsListModel::Game;
    case 6:     return AppsListModel::Office;
    case 7:     return AppsListModel::Reading;
    case 8:     return AppsListModel::Development;
    case 9:     return AppsListModel::System;
    case 10:    return AppsListModel::Others;
    default:;
    }

    qWarning() << "ItemInfo::category handle wrong category";
    return AppsListModel::All;
}

void ItemInfo::updateInfo(const ItemInfo &info)
{
    if (!this->operator==(info))
        return;

    m_name = info.m_name;
    m_iconKey = info.m_iconKey;
    m_openCount = info.m_openCount;
}

QDebug operator<<(QDebug argument, const ItemInfo &info)
{
    argument << info.m_categoryId << info.m_installedTime;
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey
             << info.m_isDir << info.m_appInfoList;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo &info)
{
    argument.beginStructure();
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey
             << info.m_categoryId << info.m_installedTime;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const ItemInfo &info)
{
    // Todo :  这里数据存储结构存在问题, 但为了向下兼容, 似乎没有必要去修改
    argument << info.m_desktop << info.m_openCount;
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey;
    argument << info.m_categoryId << info.m_installedTime << info.m_firstRunTime;
    argument << info.m_isDir << info.m_appInfoList;

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, ItemInfo &info)
{
    argument >> info.m_desktop >> info.m_openCount;
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime >> info.m_firstRunTime;
    argument >> info.m_isDir >> info.m_appInfoList;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo &info)
{
    argument.beginStructure();
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime;
    argument.endStructure();

    return argument;
}

bool ItemInfo::operator<(const ItemInfo &info) const {
    return info.m_name < m_name;
}

ItemInfo_v1::ItemInfo_v1()
    : m_status(-1)
    , m_categoryId(0)
    , m_progressValue(0)
    , m_installedTime(0)
    , m_openCount(0)
    , m_firstRunTime(0)
    , m_isDir(false)
    , m_appInfoList(ItemInfoList_v1())
{

}

ItemInfo_v1::ItemInfo_v1(const ItemInfo_v1 &info)
    : m_desktop(info.m_desktop)
    , m_name(info.m_name)
    , m_key(info.m_key)
    , m_iconKey(info.m_iconKey)
    , m_status(info.m_status)
    , m_categoryId(info.m_categoryId)
    , m_description(info.m_description)
    , m_progressValue(info.m_progressValue)
    , m_installedTime(info.m_installedTime)
    , m_openCount(info.m_openCount)
    , m_firstRunTime(info.m_firstRunTime)
    , m_isDir(info.m_isDir)
    , m_appInfoList(info.m_appInfoList)
{

}

ItemInfo_v1::ItemInfo_v1(const AppInfo &info)
{
    m_desktop = info.m_desktop;
    m_name = info.m_name;
    m_key = info.m_key;
    m_iconKey = info.m_iconKey;
    m_status = info.m_status;
    m_categoryId = info.m_categoryId;
    m_description = info.m_description;
    m_progressValue = info.m_progressValue;
}

ItemInfo_v1::ItemInfo_v1(const ItemInfo &info)
    : m_desktop(info.m_desktop)
    , m_name(info.m_name)
    , m_key(info.m_key)
    , m_iconKey(info.m_iconKey)
    , m_status(-1)
    , m_categoryId(info.m_categoryId)
    , m_description("")
    , m_progressValue(0)
    , m_installedTime(info.m_installedTime)
    , m_openCount(info.m_openCount)
    , m_firstRunTime(info.m_firstRunTime)
    , m_isDir(info.m_isDir)
    , m_appInfoList(ItemInfoList_v1())
{

}

ItemInfoList_v1 ItemInfo_v1::appListToItemList(const AppInfoList &list)
{
    ItemInfoList_v1 itemInfoList;

    foreach (const ItemInfo_v1 itemInfo, list) {
        itemInfoList << itemInfo;
    }

    return itemInfoList;
}

ItemInfoList_v1 ItemInfo_v1::itemListToItem_v1List(const ItemInfoList &list)
{
    ItemInfoList_v1 itemInfoList;

    foreach (const ItemInfo info, list) {
        itemInfoList << info;
    }

    return itemInfoList;
}

ItemInfo_v1::~ItemInfo_v1()
{

}

void ItemInfo_v1::registerMetaType()
{
    qRegisterMetaType<ItemInfo_v1>("ItemInfo_v1");
    qDBusRegisterMetaType<ItemInfo_v1>();
    qRegisterMetaType<ItemInfoList_v1>("ItemInfoList_v1");
    qDBusRegisterMetaType<ItemInfoList_v1>();
}

AppsListModel::AppCategory ItemInfo_v1::category() const
{
    switch (m_categoryId)
    {
    case 0:     return AppsListModel::Internet;
    case 1:     return AppsListModel::Chat;
    case 2:     return AppsListModel::Music;
    case 3:     return AppsListModel::Video;
    case 4:     return AppsListModel::Graphics;
    case 5:     return AppsListModel::Game;
    case 6:     return AppsListModel::Office;
    case 7:     return AppsListModel::Reading;
    case 8:     return AppsListModel::Development;
    case 9:     return AppsListModel::System;
    case 10:    return AppsListModel::Others;
    default:;
    }

    qWarning() << "ItemInfo::category handle wrong category";
    return AppsListModel::All;
}

QDebug operator<<(QDebug argument, const ItemInfo_v1 &info)
{
    argument << info.m_categoryId << info.m_installedTime
             << info.m_desktop << info.m_name << info.m_key << info.m_iconKey
             << info.m_isDir << info.m_appInfoList;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo_v1 &info)
{
    argument.beginStructure();
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey
             << info.m_categoryId << info.m_installedTime;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo_v1 &info)
{
    argument.beginStructure();
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const ItemInfo_v1 &info)
{
    argument << info.m_categoryId << info.m_installedTime << info.m_firstRunTime;
    argument << info.m_progressValue << info.m_openCount;
    argument << info.m_isDir << info.m_appInfoList;

    return argument;
}


const QDataStream &operator>>(QDataStream &argument, ItemInfo_v1 &info)
{
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime >> info.m_firstRunTime;
    argument >> info.m_progressValue << info.m_openCount;
    argument >> info.m_isDir >> info.m_appInfoList;

    return argument;
}

void ItemInfo_v1::updateInfo(const ItemInfo_v1 &info)
{
    if (!this->operator==(info))
        return;

    m_name = info.m_name;
    m_iconKey = info.m_iconKey;
    m_openCount = info.m_openCount;
}

bool ItemInfo_v1::operator<(const ItemInfo_v1 &info) const
{
    return info.m_name < m_name;
}
