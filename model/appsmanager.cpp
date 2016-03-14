#include "appsmanager.h"

#include <QDebug>
#include <QPixmap>

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

    Q_ASSERT(m_appInfos.contains(category));

    return m_appInfos[category];
}

QPixmap AppsManager::appIcon(const QString &desktop, const int size)
{
    const QString pixmap = m_fileInfoInter->GetThemeIcon(desktop, size).value();

    return QPixmap(pixmap);
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
