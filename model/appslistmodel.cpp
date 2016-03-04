#include "appslistmodel.h"

#include <QSize>
#include <QDebug>

AppsListModel::AppsListModel(QObject *parent) :
    QAbstractListModel(parent),
    m_appsManager(new AppsManager(this))
{

}

int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_appsManager->appsInfoList().size();
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
    if (!index.isValid() || index.row() >= m_appsManager->appsInfoList().size())
        return QVariant();

    switch (role)
    {
    case AppNameRole:
        return m_appsManager->appsInfoList()[index.row()].m_name;
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
