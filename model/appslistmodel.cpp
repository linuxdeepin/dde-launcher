#include "appslistmodel.h"
#include "appsmanager.h"

#include <QSize>
#include <QDebug>
#include <QPixmap>

AppsManager *AppsListModel::m_appsManager = nullptr;

AppsListModel::AppsListModel(const AppCategory &category, QObject *parent) :
    QAbstractListModel(parent),
    m_category(category)
{
    if (!m_appsManager)
        m_appsManager = AppsManager::instance(this);
}

int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_appsManager->appsInfoList(m_category).size();
}

bool AppsListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    // TODO: not support remove multiple rows
    Q_ASSERT(count == 1);

    beginRemoveRows(parent, row, row);
    m_appsManager->removeRow(row);
    endRemoveRows();

    return true;
}

bool AppsListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(data)
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    return true;
}

QVariant AppsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_appsManager->appsInfoList(m_category).size())
        return QVariant();

    const ItemInfo itemInfo = m_appsManager->appsInfoList(m_category)[index.row()];

    switch (role)
    {

    case AppNameRole:
        return itemInfo.m_name;
    case AppCategoryRole:
        return itemInfo.category();
    case AppAutoStartRole:
        return m_appsManager->appIsAutoStart(itemInfo.m_desktop);
//    case AppNewInstallRole:
    case AppIconRole:
        return m_appsManager->appIcon(itemInfo.m_desktop, 256);
    case ItemSizeHintRole:
        return QSize(150, 150);

    default:
        return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags AppsListModel::flags(const QModelIndex &index) const
{
//    if (!index.isValid() || index.row() >= m_appsManager->appsInfoList().size())
//        return Qt::NoItemFlags;

    const Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}
