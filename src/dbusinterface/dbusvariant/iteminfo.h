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

#ifndef ITEMINFO_H
#define ITEMINFO_H

#include "appslistmodel.h"

#include <QtDBus>

typedef QList<ItemInfo> ItemInfoList;

class ItemInfo
{
public:
    ItemInfo();
    ItemInfo(const ItemInfo &info);
    ~ItemInfo();

    static void registerMetaType();

    AppsListModel::AppCategory category() const;

    inline bool operator==(const ItemInfo &other) const { return m_desktop == other.m_desktop; }
    friend QDebug operator<<(QDebug argument, const ItemInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo &info);
    friend QDataStream &operator<<(QDataStream &argument, const ItemInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo &info);
    friend const QDataStream &operator>>(QDataStream &argument, ItemInfo &info);

    void updateInfo(const ItemInfo &info);

    bool operator<(const ItemInfo &info) const;

public:
    QString m_desktop;              // 应用的绝对路径
    QString m_name;                 // 应用名称
    QString m_key;                  // 应用所对应的二进制名称
    QString m_iconKey;              // 图标文件名称
    qlonglong m_categoryId;         // 应用分类的id,每个分类的id值不同
    qlonglong m_installedTime;      // 安装时间
    qlonglong m_openCount;          // 打开次数
    qlonglong m_firstRunTime;       // 首次运行的时间戳
    bool m_isDir;                   // 是否为文件夹
    QModelIndexList m_modelIndexList;
    ItemInfoList m_appInfoList;
};

Q_DECLARE_METATYPE(ItemInfo)
Q_DECLARE_METATYPE(ItemInfoList)

#endif // ITEMINFO_H
