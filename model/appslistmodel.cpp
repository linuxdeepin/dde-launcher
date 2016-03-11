#include "appslistmodel.h"
#include "global_util/themeappicon.h"
#include "appsmanager.h"

#include <QSize>
#include <QDebug>

AppsListModel::AppsListModel(CategoryID id, QObject *parent) :
    QAbstractListModel(parent)

{
    m_appCategory = id;

}

int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_appCategory == CategoryID::All) {
    return appsManager->appsInfoList().size();
    } else {
        for(int i(0); i < 11; i++) {
            if (m_appCategory == CategoryID(i)) {

        ItemInfoList tmpCateItemInfoList = appsManager->getCategoryItemInfo(i);
                return tmpCateItemInfoList.length();
            } else {
                continue;
            }
        }
        return 0;
    }
}

bool AppsListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    // TODO: not support remove multiple rows
    Q_ASSERT(count == 1);

    beginRemoveRows(parent, row, row);
    appsManager->removeRow(row);
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
    for(int i(-1); i < 11; i++) {
        if (m_appCategory == CategoryID(i)) {
            ItemInfoList tmpCateItemInfoList = appsManager->getCategoryItemInfo(i);
            qDebug() << "List model:" << i << tmpCateItemInfoList.length();
            if (!index.isValid() || index.row() >= tmpCateItemInfoList.size())
                return QVariant();
            switch (role) {
            case AppNameRole:
            {
                qDebug() << "appNameRole:" <<  tmpCateItemInfoList.at(index.row()).m_name;
                qDebug() << tmpCateItemInfoList.at(index.row()).m_name;
                return  tmpCateItemInfoList.at(index.row()).m_name;
            }
            case AppIconRole:
            {
                QString appUrl = tmpCateItemInfoList.at(index.row()).iconUrl;
                return appUrl;
            }
            case ItemSizeHintRole:
                return QSize(150, 150);
            default:
            {
                qDebug() << "default role";
                return QVariant();
            }
            }

        } else {
            continue;
        }
    }


    return QVariant();
}

void AppsListModel::setListModelData(CategoryID cate) {
    m_appCategory = cate;

}

Qt::ItemFlags AppsListModel::flags(const QModelIndex &index) const
{
//    if (!index.isValid() || index.row() >= appsManager->appsInfoList().size())
//        return Qt::NoItemFlags;

    const Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}
