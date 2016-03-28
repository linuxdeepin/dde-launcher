#include "appslistmodel.h"
#include "appsmanager.h"
#include "global_util/calculate_util.h"
#include "global_util/constants.h"
#include "dbusinterface/dbusvariant/iteminfo.h"

#include <QSize>
#include <QDebug>
#include <QPixmap>

AppsListModel::AppsListModel(const AppCategory &category, QObject *parent) :
    QAbstractListModel(parent),
    m_appsManager(AppsManager::instance(this)),
    m_calcUtil(CalculateUtil::instance(this)),
    m_category(category)
{
    connect(m_appsManager, &AppsManager::dataChanged, this, &AppsListModel::dataChanged);
    connect(m_appsManager, &AppsManager::layoutChanged, this, &AppsListModel::layoutChanged);
}

int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_appsManager->appsInfoList(m_category).size();
}

bool AppsListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)

    // do not allow remove items.
    Q_ASSERT(false);

//    // TODO: not support remove multiple rows
//    Q_ASSERT(count == 1);

//    beginRemoveRows(parent, row, row);
//    m_appsManager->removeRow(row);
//    endRemoveRows();

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

QMimeData *AppsListModel::mimeData(const QModelIndexList &indexes) const
{
    // only allow drag 1 item
    Q_ASSERT(indexes.size() == 1);

    const QModelIndex index = indexes.first();

    QJsonObject json;
    json.insert("appKey", index.data(AppKeyRole).toString());
    json.insert("appIcon", index.data(AppIconKeyRole).toString());
    json.insert("appName", index.data(AppNameRole).toString());

    QMimeData *mime = new QMimeData;
    mime->setData("RequestDock", QJsonDocument(json).toJson());

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
        return m_appsManager->appIsOnDock(itemInfo.m_key);
    case AppNewInstallRole:
        return m_appsManager->appIsNewInstall(itemInfo.m_key);
    case AppIconRole:
        return m_appsManager->appIcon(itemInfo.m_desktop, m_calcUtil->appIconSize());
    case ItemSizeHintRole:
        return m_calcUtil->appItemSize();
    case AppIconSizeRole:
        return m_calcUtil->appIconSize();
    case AppFontSizeRole:
        return m_calcUtil->appItemFontSize();
    default:;
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

void AppsListModel::dataChanged(const AppCategory category)
{
    if (category == All || category == m_category)
        emit QAbstractItemModel::dataChanged(index(0), index(rowCount(QModelIndex())));
}

void AppsListModel::layoutChanged(const AppsListModel::AppCategory category)
{
    if (category == All || category == m_category)
        emit QAbstractItemModel::layoutChanged();
}
