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

#include "appslistmodel.h"
#include "appsmanager.h"
#include "global_util/calculate_util.h"
#include "global_util/constants.h"
#include "dbusinterface/dbusvariant/iteminfo.h"

#include <QSize>
#include <QDebug>
#include <QPixmap>
#include <QSettings>

const QStringList sysHoldPackages()
{
    const QSettings settings("/etc/deepin-installer.conf", QSettings::IniFormat);
    const auto holds_list = settings.value("dde_launcher_hold_packages").toStringList();

    if (holds_list.isEmpty() ||
        (holds_list.size() == 1 && holds_list.first().isEmpty()))
        return QStringList () << "dde-control-center" << "dde-computer" << "dde-trash"
                              << "dde-file-manager" << "deepin-appstore" << "deepin-toggle-desktop"
                              << "deepin-wm-multitaskingview" << "dde-calendar" << "deepin-manual";

    return holds_list;
}

AppsListModel::AppsListModel(const AppCategory &category, QObject *parent) :
    QAbstractListModel(parent),
    m_appsManager(AppsManager::instance()),
    m_calcUtil(CalculateUtil::instance()),
    m_holdPackages(sysHoldPackages()),
    m_category(category)
{
    connect(m_appsManager, &AppsManager::dataChanged, this, &AppsListModel::dataChanged);
    connect(m_appsManager, &AppsManager::layoutChanged, this, &AppsListModel::layoutChanged);
}

void AppsListModel::setCategory(const AppsListModel::AppCategory category)
{
    m_category = category;

    emit QAbstractListModel::layoutChanged();
}

///
/// \brief AppsListModel::setDraggingIndex mark current item as dragging item
/// \param index item index
///
void AppsListModel::setDraggingIndex(const QModelIndex &index)
{
    m_dragStartIndex = index;
    m_dragDropIndex = index;

    emit QAbstractListModel::dataChanged(index, index);
}

void AppsListModel::setDragDropIndex(const QModelIndex &index)
{
    if (m_dragDropIndex == index)
        return;
//    if (m_dragDropIndex == m_dragStartIndex)
//        return;

    m_dragDropIndex = index;

    emit QAbstractListModel::dataChanged(m_dragStartIndex, index);
}

///
/// \brief AppsListModel::dropInsert restore item from appsManager stash list
/// \param appKey item token in stash list
/// \param pos insert position, if pos is negetive, insert into front
///
void AppsListModel::dropInsert(const QString &appKey, const int pos)
{
    beginInsertRows(QModelIndex(), pos, pos);
    m_appsManager->restoreItem(appKey, pos);
    endInsertRows();
}

///
/// \brief AppsListModel::dropSwap drop m_draingIndex to nextPos
/// \param nextPos m_draggingIndex insert position
///
void AppsListModel::dropSwap(const int nextPos)
{
    if (!m_dragStartIndex.isValid())
        return;

    const QString appKey = m_dragStartIndex.data(AppsListModel::AppKeyRole).toString();

    removeRows(m_dragStartIndex.row(), 1, QModelIndex());
    dropInsert(appKey, nextPos);

    emit QAbstractItemModel::dataChanged(m_dragStartIndex, m_dragDropIndex);

    m_dragStartIndex = m_dragDropIndex = index(nextPos);
}

///
/// \brief AppsListModel::clearDraggingIndex reset dragging item record
///
void AppsListModel::clearDraggingIndex()
{
    const QModelIndex startIndex = m_dragStartIndex;
    const QModelIndex endIndex = m_dragDropIndex;

    m_dragStartIndex = m_dragDropIndex = QModelIndex();

    emit QAbstractItemModel::dataChanged(startIndex, endIndex);
}

int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (m_category == Used) {
        return MAXIMUM_POPULAR_ITEMS;
    }

    return m_appsManager->appsInfoList(m_category).size();
}

const QModelIndex AppsListModel::indexAt(const QString &appKey) const
{
    Q_ASSERT(m_category == All);

    int i = 0;
    const int count = rowCount(QModelIndex());
    while (i != count)
    {
        if (index(i).data(AppKeyRole).toString() == appKey)
            return index(i);
        ++i;
    }

    Q_UNREACHABLE();
}

bool AppsListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)

    // TODO: not support remove multiple rows
    Q_ASSERT(count == 1);

    beginRemoveRows(parent, row, row);
    m_appsManager->stashItem(index(row));
    endRemoveRows();

    return true;
}

bool AppsListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    // disable invalid drop
    if (data->data("RequestDock").isEmpty())
        return false;

    if (m_category != All)
        return false;

    return true;
}

QMimeData *AppsListModel::mimeData(const QModelIndexList &indexes) const
{
    // only allow drag 1 item
    Q_ASSERT(indexes.size() == 1);

    const QModelIndex index = indexes.first();

    QMimeData *mime = new QMimeData;
    mime->setData("RequestDock", index.data(AppDesktopRole).toByteArray());
    mime->setData("AppKey", index.data(AppKeyRole).toByteArray());

    if (index.data(AppIsRemovableRole).toBool())
        mime->setData("Removable", "");

    // this object will be delete in drag event finished.
    return mime;
}

QVariant AppsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_appsManager->appsInfoList(m_category).size())
        return QVariant();

    const ItemInfo itemInfo = m_appsManager->appsInfoList(m_category)[index.row()];

    switch (role)
    {
    case AppRawItemInfoRole:
        return QVariant::fromValue(itemInfo);
    case AppNameRole:
        return itemInfo.m_name;
    case AppDesktopRole:
        return itemInfo.m_desktop;
    case AppKeyRole:
        return itemInfo.m_key;
    case AppIconKeyRole:
        return itemInfo.m_iconKey;
    case AppCategoryRole:
        return QVariant::fromValue(itemInfo.category());
    case AppGroupRole:
        return QVariant::fromValue(m_category);
    case AppAutoStartRole:
        return m_appsManager->appIsAutoStart(itemInfo.m_desktop);
    case AppIsOnDesktopRole:
        return m_appsManager->appIsOnDesktop(itemInfo.m_key);
    case AppIsOnDockRole:
        return m_appsManager->appIsOnDock(itemInfo.m_desktop);
    case AppIsRemovableRole:
        return !m_holdPackages.contains(itemInfo.m_key);
    case AppIsProxyRole:
        return m_appsManager->appIsProxy(itemInfo.m_key);
    case AppEnableScalingRole:
        return m_appsManager->appIsEnableScaling(itemInfo.m_key);
    case AppNewInstallRole:
        return m_appsManager->appIsNewInstall(itemInfo.m_key);
    case AppIconRole:
        return m_appsManager->appIcon(itemInfo.m_iconKey, m_calcUtil->appIconSize().width());
    case AppDragIconRole:
        return m_appsManager->appIcon(itemInfo.m_iconKey, m_calcUtil->appIconSize().width() * 1.2);
    case ItemSizeHintRole:
        return m_calcUtil->appItemSize();
    case AppIconSizeRole:
        return m_calcUtil->appIconSize();
    case AppFontSizeRole:
        return m_calcUtil->appItemFontSize();
    case AppItemIsDraggingRole:
        return indexDragging(index);
    default:;
    }

    return QVariant();
}

Qt::ItemFlags AppsListModel::flags(const QModelIndex &index) const
{
//    if (!index.isValid() || index.row() >= m_appsManager->appsInfoList().size())
//        return Qt::NoItemFlags;

    const Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (m_category == All)
        return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else
        return defaultFlags;
}

///
/// \brief AppsListModel::dataChanged tell view the appManager data is changed
/// \param category data category
///
void AppsListModel::dataChanged(const AppCategory category)
{
    if (category == All || category == m_category)
        emit QAbstractItemModel::layoutChanged();
//        emit QAbstractItemModel::dataChanged(index(0), index(rowCount(QModelIndex())));
}

///
/// \brief AppsListModel::layoutChanged tell view the app layout is changed, such as appItem size, icon size, etc.
/// \param category data category
///
void AppsListModel::layoutChanged(const AppsListModel::AppCategory category)
{
    if (category == All || category == m_category)
        emit QAbstractItemModel::dataChanged(QModelIndex(), QModelIndex());
}

bool AppsListModel::indexDragging(const QModelIndex &index) const
{
    if (!m_dragStartIndex.isValid() || !m_dragDropIndex.isValid())
        return false;

    const int start = m_dragStartIndex.row();
    const int end = m_dragDropIndex.row();
    const int current = index.row();

    return (start <= end && current >= start && current <= end) ||
            (start >= end && current <= start && current >= end);
}

//bool AppsListModel::itemIsRemovable(const QString &desktop) const
//{
//    return m_holdPackages.contains(desktop);
//    static QStringList blacklist;
//    if (blacklist.isEmpty()) {
//        QFile file(UninstallFilterFile);
//        if (file.open(QFile::ReadOnly)) {
//            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
//            QJsonObject obj = doc.object();
//            QJsonArray arr = obj["blacklist"].toArray();
//            foreach (QJsonValue val, arr) {
//                blacklist << val.toString();
//            }
//            file.close();
//        }
//    }

//    foreach (QString val, blacklist) {
//        if (desktop.endsWith(val)) {
//            return false;
//        }
//    }

//    return true;
//}
