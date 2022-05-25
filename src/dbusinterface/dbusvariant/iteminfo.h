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
class ItemInfo_v2;

typedef QList<ItemInfo> ItemInfoList;
typedef QList<ItemInfo_v1> ItemInfoList_v1;
typedef QList<ItemInfo_v2> ItemInfoList_v2;

/** 1050 版本及之前的原始数据结构,为提升兼容性,
 *  数据接口在下面进行扩展
 * @brief The ItemInfo class
 */
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
};

/**为适配应用文件夹需求,新增数据结构ItemInfo_v1
 * @brief The ItemInfo_v1 class
 */
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
    ItemInfo_v1(const ItemInfo_v2 &info);

    static ItemInfoList_v1 appListToItemV1List(const AppInfoList &list);
    static ItemInfoList_v1 itemListToItemV1List(const ItemInfoList &list);
    static ItemInfoList_v1 itemV2ListToItemV1List(const ItemInfoList_v2 &list);

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
    QStringList m_keywords;         // *.desktop文件中的关键字,供搜索用
    int m_status;                   // 应用当前的状态
    qlonglong m_categoryId;         // 应用分类的id,每个分类的id值不同
    QString m_description;          // 应用展示的信息
    int m_progressValue;            // 显示应用的某种进度
    qlonglong m_installedTime;      // 安装时间
    qlonglong m_openCount;          // 打开次数
    qlonglong m_firstRunTime;       // 首次运行的时间戳

    bool m_isDir;                   // 是否为文件夹
    ItemInfoList_v1 m_appInfoList;
};

/**为适配AM,新增加ItemInfo_v2数据结构,
 * 该接口中新增搜索关键字m_keywords
 * @brief The ItemInfo_v2 class
 */
class ItemInfo_v2
{
public:
    ItemInfo_v2();
    ItemInfo_v2(const ItemInfo_v2 &info);
    ~ItemInfo_v2();

    static void registerMetaType();

    AppsListModel::AppCategory category() const;

    inline bool operator==(const ItemInfo_v2 &other) const { return m_desktop == other.m_desktop; }
    friend QDebug operator<<(QDebug argument, const ItemInfo_v2 &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo_v2 &info);
    friend QDataStream &operator<<(QDataStream &argument, const ItemInfo_v2 &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo_v2 &info);
    friend const QDataStream &operator>>(QDataStream &argument, ItemInfo_v2 &info);

    void updateInfo(const ItemInfo_v2 &info);

    bool operator<(const ItemInfo_v2 &info) const;

public:
    QString m_desktop;              // 应用的绝对路径
    QString m_name;                 // 应用名称
    QString m_key;                  // 应用所对应的二进制名称
    QString m_iconKey;              // 图标文件名称
    qlonglong m_categoryId;         // 应用分类的id,每个分类的id值不同
    qlonglong m_installedTime;      // 安装时间
    QStringList m_keywords;         // 搜索关键字
};

Q_DECLARE_METATYPE(ItemInfo)
Q_DECLARE_METATYPE(ItemInfoList)
Q_DECLARE_METATYPE(ItemInfo_v1)
Q_DECLARE_METATYPE(ItemInfoList_v1)
Q_DECLARE_METATYPE(ItemInfo_v2)
Q_DECLARE_METATYPE(ItemInfoList_v2)

#endif // ITEMINFO_H
