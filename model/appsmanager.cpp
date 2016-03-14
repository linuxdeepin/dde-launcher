#include "appsmanager.h"

#include <QDebug>
#include <QPixmap>

QSettings AppsManager::AppIconCache("deepin", "dde-launcher-app-icon", nullptr);
//QSettings AppsManager::AppInfoCache("deepin", "dde-launcher-app-info", nullptr);

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this)),
    m_fileInfoInter(new DBusFileInfo(this))
{
    m_appInfoList = m_launcherInter->GetAllItemInfos().value();
    refreshCategoryInfoList();
}

void AppsManager::removeRow(const int row)
{
    m_appInfoList.removeAt(row);
}

const ItemInfoList AppsManager::appsInfoList(const AppsListModel::AppCategory &category) const
{
    switch (category)
    {
    case AppsListModel::Custom:
    case AppsListModel::All:        return m_appInfoList;           break;
    case AppsListModel::Search:     return m_appSearchResultList;   break;
    default:;
    }

    //Q_ASSERT(m_appInfos.contains(category));

    return m_appInfos[category];
}

const QPixmap AppsManager::appIcon(const QString &desktop, const int size)
{
    const QString cacheKey = QString("%1-%2").arg(desktop)
                                             .arg(size);

    const QPixmap cachePixmap = AppIconCache.value(cacheKey).value<QPixmap>();
    if (!cachePixmap.isNull())
        return cachePixmap;

    // cache fail
    const QString iconFile = m_fileInfoInter->GetThemeIcon(desktop, size).value();
    QPixmap iconPixmap = QPixmap(iconFile);

    if (iconPixmap.isNull())
        iconPixmap = QPixmap(":/skin/images/application-default-icon.svg");

    AppIconCache.setValue(cacheKey, iconPixmap);

    return iconPixmap;
}

void AppsManager::refreshCategoryInfoList()
{
    m_appInfos.clear();

    for (const ItemInfo &info : m_appInfoList)
    {
        const AppsListModel::AppCategory category = info.category();
        if (!m_appInfos.contains(category))
            m_appInfos.insert(category, ItemInfoList());

        m_appInfos[category].append(info);
    }
}

void AppsManager::refreshAppIconCache()
{
    AppIconCache.sync();
    AppIconCache.clear();
}
