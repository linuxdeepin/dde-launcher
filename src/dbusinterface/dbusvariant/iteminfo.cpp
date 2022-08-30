// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey;
    argument << info.m_categoryId << info.m_installedTime;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const ItemInfo &info)
{
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

bool ItemInfo::operator<(const ItemInfo &info) const {
    return info.m_name < m_name;
}
