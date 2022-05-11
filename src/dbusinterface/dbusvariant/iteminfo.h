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
#include "common.h"

#include <QDBusArgument>
#include <QDebug>
#include <QDataStream>
#include <QtDBus>

class ItemInfo;
class ItemInfo_v1;

typedef QList<ItemInfo> ItemInfoList;
typedef QList<ItemInfo_v1> ItemInfoList_v1;

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

    // Todo: 二进制兼容， 后面可以重新写一个ItemInfo2, 读取缓存配置的时候也要留意该问题。升级后，删除旧的配置文件。
    bool m_isDir;                   // 是否为文件夹
    ItemInfoList m_appInfoList;
};

class ItemInfo_v1
{
public:
    enum ItemStatus {
        Normal,         // 正常状态，不展示应用的进度值
        Busy,           // 进行中的状态，展示应用的进度值
    };

    explicit ItemInfo_v1();
    ItemInfo_v1(const ItemInfo_v1 &info);
    ItemInfo_v1(const AppInfo &info);
    ItemInfo_v1(const ItemInfo &info);

    static ItemInfoList_v1 appListToItemList(const AppInfoList &list);
    static ItemInfoList_v1 itemListToItem_v1List(const ItemInfoList &list);

    ~ItemInfo_v1();

    static void registerMetaType();

    AppsListModel::AppCategory category() const;

    inline bool operator==(const ItemInfo_v1 &other) {  return m_desktop == other.m_desktop; }
    friend QDebug operator<<(QDebug argument, const ItemInfo_v1 &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo_v1 &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo_v1 &info);
    friend const QDataStream &operator>>(QDataStream &argument, ItemInfo_v1 &info);
    friend QDataStream &operator<<(QDataStream &argument, const ItemInfo_v1 &info);

    void updateInfo(const ItemInfo_v1 &info);

    bool operator<(const ItemInfo_v1 &info) const;

public:
    QString m_desktop;              // 应用的绝对路径
    QString m_name;                 // 应用名称
    QString m_key;                  // 应用所对应的二进制名称
    QString m_iconKey;              // 图标文件名称
    int m_status;                   // 应用当前的状态
    qlonglong m_categoryId;         // 应用分类的id,每个分类的id值不同
    QString m_description;          // 应用展示的信息
    int m_progressValue;            // 显示应用的某种进度
    qlonglong m_installedTime;      // 安装时间
    qlonglong m_openCount;          // 打开次数
    qlonglong m_firstRunTime;       // 首次运行的时间戳

    // Todo: 二进制兼容， 后面可以重新写一个ItemInfo2, 读取缓存配置的时候也要留意该问题。升级后，删除旧的配置文件。
    bool m_isDir;                   // 是否为文件夹
    ItemInfoList_v1 m_appInfoList;
};

Q_DECLARE_METATYPE(ItemInfo)
Q_DECLARE_METATYPE(ItemInfoList)
Q_DECLARE_METATYPE(ItemInfo_v1)
Q_DECLARE_METATYPE(ItemInfoList_v1)

#endif // ITEMINFO_H
