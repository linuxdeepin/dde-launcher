/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

ItemInfo::ItemInfo()
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
    case 0:     return AppsListModel::Internet;         break;
    case 1:     return AppsListModel::Chat;             break;
    case 2:     return AppsListModel::Music;            break;
    case 3:     return AppsListModel::Video;            break;
    case 4:     return AppsListModel::Graphics;         break;
    case 5:     return AppsListModel::Game;             break;
    case 6:     return AppsListModel::Office;           break;
    case 7:     return AppsListModel::Reading;          break;
    case 8:     return AppsListModel::Development;      break;
    case 9:     return AppsListModel::System;           break;
    case 10:    return AppsListModel::Others;           break;
    default:;
    }

    qWarning() << "ItemInfo::category handle wrong category";
    return AppsListModel::All;
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
    argument << info.m_desktop << info.m_name << info.m_key << info.m_iconKey;
    argument << info.m_categoryId << info.m_installedTime;

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, ItemInfo &info)
{
    argument >> info.m_desktop >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_categoryId >> info.m_installedTime;

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
