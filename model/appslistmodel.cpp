#include "appslistmodel.h"

#include <QSize>
#include <QDebug>

AppsListModel::AppsListModel(QObject *parent) :
    QAbstractListModel(parent),
    m_appsManager(new AppsManager(this))
{

}

bool AppsListModel::removeRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    beginRemoveRows(parent, row, row);
    m_appsManager->removeRow(row);
    endRemoveRows();

    return true;
}

int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_appsManager->appsInfoList().size();
}

QVariant AppsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_appsManager->appsInfoList().size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return m_appsManager->appsInfoList()[index.row()].m_name;
    case Qt::SizeHintRole:
        return QSize(150, 150);
    default:
        return QVariant();
    }

    return QVariant();
}
