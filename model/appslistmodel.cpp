
#include "appslistmodel.h"
#include "appsmanager.h"
#include "global_util/constants.h"
#include "dbus/dbusvariant/iteminfo.h"

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

    connect(m_appsManager, &AppsManager::dataChanged, this, &AppsListModel::dataChanged);
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
    case AppRawItemInfoRole:
        return QVariant::fromValue(itemInfo);
    case AppNameRole:
        return itemInfo.m_name;
    case AppDesktopRole:
        return itemInfo.m_desktop;
    case AppKeyRole:
        return itemInfo.m_key;
    case AppCategoryRole:
        return itemInfo.category();
    case AppAutoStartRole:
        return m_appsManager->appIsAutoStart(itemInfo.m_desktop);
    case AppIsOnDesktopRole:
        return m_appsManager->appIsOnDesktop(itemInfo.m_desktop);
    case AppIsOnDockRole:
        return m_appsManager->appIsOnDock(itemInfo.m_key);
    case AppNewInstallRole:
        return m_appsManager->appIsNewInstall(itemInfo.m_key);
    case AppIconRole:
        return m_appsManager->appIcon(itemInfo.m_desktop, 256);
    case ItemSizeHintRole:
        return QSize(DLauncher::APP_ITEM_WIDTH, DLauncher::APP_ITEM_HEIGHT);
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
    if (category == m_category)
        emit QAbstractItemModel::dataChanged(index(0), index(rowCount(QModelIndex())));
}
