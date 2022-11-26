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
    switch (m_categoryId) {
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
    return AppsListModel::FullscreenAll;
}

bool ItemInfo::operator==(const ItemInfo &other) const
{
    return (m_desktop == other.m_desktop && m_name == other.m_name &&
            m_key == other.m_key && m_iconKey == other.m_iconKey &&
            m_categoryId == other.m_categoryId && m_openCount == other.m_openCount);
}

void ItemInfo::updateInfo(const ItemInfo &info)
{
    if (*this == info)
        return;

    *this = info;
}

QDebug operator<<(QDebug argument, const ItemInfo &info)
{
    argument << info.m_categoryId << info.m_installedTime;
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey;

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

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, ItemInfo &info)
{
    argument >> info.m_desktop >> info.m_openCount;
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime >> info.m_firstRunTime;

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

bool ItemInfo::operator<(const ItemInfo &info) const
{
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
    , m_isDir(false)
    , m_appInfoList(ItemInfoList_v1())
{

}

ItemInfo_v1::ItemInfo_v1(const ItemInfo_v2 &info)
    : m_desktop(info.m_desktop)
    , m_name(info.m_name)
    , m_key(info.m_key)
    , m_iconKey(info.m_iconKey)
    , m_keywords(info.m_keywords)
    , m_status(0)
    , m_categoryId(info.m_categoryId)
    , m_progressValue(0)
    , m_installedTime(info.m_installedTime)
    , m_openCount(0)
    , m_firstRunTime(0)
    , m_isDir(false)
    , m_appInfoList(ItemInfoList_v1())
{
}

ItemInfoList_v1 ItemInfo_v1::appListToItemV1List(const AppInfoList &list)
{
    ItemInfoList_v1 itemInfoList;

    foreach (const ItemInfo_v1 &itemInfo, list)
        itemInfoList << itemInfo;

    return itemInfoList;
}

ItemInfoList_v1 ItemInfo_v1::itemListToItemV1List(const ItemInfoList &list)
{
    ItemInfoList_v1 itemInfoList;

    foreach (const ItemInfo &info, list)
        itemInfoList << info;

    return itemInfoList;
}

ItemInfoList_v1 ItemInfo_v1::itemV2ListToItemV1List(const ItemInfoList_v2 &list)
{
    ItemInfoList_v1 itemInfoList;

    for (const ItemInfo_v2 &info : list)
        itemInfoList << info;

    return itemInfoList;
}

bool ItemInfo_v1::isTitle() const
{
    return ((m_name == m_desktop) && m_name.front().isUpper()) || (m_desktop == "#");
}

bool ItemInfo_v1::startWithLetter() const
{
    static QList<QChar> alphabetList;
    if (alphabetList.isEmpty()) {
        for (int i = 0; i < 26; i++)
            alphabetList.append('A' + i);
    }

    return alphabetList.contains(m_name.left(1).toUpper().front());
}

bool ItemInfo_v1::startWithNum() const
{
    static QList<QChar> numList;
    if (numList.isEmpty()) {
        for (int i = 0; i < 10; i++)
            numList.append('0' + i);
    }

    return numList.contains(m_name.left(1).front());
}

bool ItemInfo_v1::isLingLongApp() const
{
    // desktop全路径中包含linglong字段
    return m_desktop.contains("/linglong/");
}

bool ItemInfo_v1::isEqual(const ItemInfo_v1 &other) const
{
    return (m_desktop == other.m_desktop && m_name == other.m_name &&
            m_key == other.m_key && m_iconKey == other.m_iconKey &&
            m_categoryId == other.m_categoryId && m_isDir == other.m_isDir &&
            m_appInfoList == other.m_appInfoList);
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
    switch (m_categoryId) {
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
    return AppsListModel::FullscreenAll;
}

bool ItemInfo_v1::operator==(const ItemInfo_v1 &other) const
{
    return (m_desktop == other.m_desktop && m_name == other.m_name &&
            m_key == other.m_key && m_iconKey == other.m_iconKey &&
            m_keywords == other.m_keywords && m_openCount == other.m_openCount &&
            m_categoryId == other.m_categoryId && m_description == other.m_description &&
            m_isDir == other.m_isDir && m_appInfoList == other.m_appInfoList);
}

QDebug operator<<(QDebug argument, const ItemInfo_v1 &info)
{
    argument << "categoryId: " << info.m_categoryId << ", installedTime: " << info.m_installedTime
             << ", desktop: " << info.m_desktop << ", name: " << info.m_name
             << ", key: " << info.m_key << ", iconKey: " << info.m_iconKey
             << ", isDir: " << info.m_isDir << ", appInfoList: " << info.m_appInfoList;

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
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey;
    argument << info.m_categoryId << info.m_installedTime << info.m_firstRunTime;
    argument << info.m_progressValue << info.m_openCount;
    argument << info.m_isDir << info.m_appInfoList;

    return argument;
}


const QDataStream &operator>>(QDataStream &argument, ItemInfo_v1 &info)
{
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime >> info.m_firstRunTime;
    argument >> info.m_progressValue >> info.m_openCount;
    argument >> info.m_isDir >> info.m_appInfoList;

    return argument;
}

void ItemInfo_v1::updateInfo(const ItemInfo_v1 &info)
{
    if (*this == info)
        return;

    *this = info;
}

bool ItemInfo_v1::operator<(const ItemInfo_v1 &info) const
{
    return info.m_name < m_name;
}

ItemInfo_v2::ItemInfo_v2()
    : m_categoryId(-1)
    , m_installedTime(0)
{

}

ItemInfo_v2::ItemInfo_v2(const ItemInfo_v2 &info)
    : m_desktop(info.m_desktop)
    , m_name(info.m_name)
    , m_key(info.m_key)
    , m_iconKey(info.m_iconKey)
    , m_categoryId(info.m_categoryId)
    , m_installedTime(info.m_installedTime)
    , m_keywords(info.m_keywords)
{

}

ItemInfo_v2::~ItemInfo_v2()
{

}

void ItemInfo_v2::registerMetaType()
{
    qRegisterMetaType<ItemInfo_v2>("ItemInfo_v2");
    qDBusRegisterMetaType<ItemInfo_v2>();
    qRegisterMetaType<ItemInfoList_v2>("ItemInfoList_v2");
    qDBusRegisterMetaType<ItemInfoList_v2>();
}

AppsListModel::AppCategory ItemInfo_v2::category() const
{
    switch (m_categoryId) {
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

    return AppsListModel::FullscreenAll;
}

bool ItemInfo_v2::operator==(const ItemInfo_v2 &other) const
{
    return (m_desktop == other.m_desktop && m_name == other.m_name &&
            m_key == other.m_key && m_iconKey == other.m_iconKey &&
            m_categoryId == other.m_categoryId && m_keywords == other.m_keywords);
}

QDebug operator<<(QDebug argument, const ItemInfo_v2 &info)
{
    argument << "desktop: " << info.m_desktop << ", name: " << info.m_name
             << ", key: " << info.m_key << ", iconKey: " << info.m_iconKey
             << ", categoryId: " << info.m_categoryId << ", installedTime: " << info.m_installedTime
             << ", keywords: " << info.m_keywords;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo_v2 &info)
{
    argument.beginStructure();
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey
             << info.m_categoryId << info.m_installedTime << info.m_keywords;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const ItemInfo_v2 &info)
{
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey;
    argument << info.m_categoryId << info.m_installedTime << info.m_keywords;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo_v2 &info)
{
    argument.beginStructure();
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey
             >> info.m_categoryId >> info.m_installedTime >> info.m_keywords;
    argument.endStructure();

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, ItemInfo_v2 &info)
{
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime >> info.m_keywords;

    return argument;
}

void ItemInfo_v2::updateInfo(const ItemInfo_v2 &info)
{
    if (*this == info)
        return;

    *this = info;
}

bool ItemInfo_v2::operator<(const ItemInfo_v2 &info) const
{
    return info.m_name < m_name;
}
