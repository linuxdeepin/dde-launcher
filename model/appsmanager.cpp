#include "appsmanager.h"

#include <QDebug>
#include <QX11Info>

AppsManager *AppsManager::INSTANCE = nullptr;

QSettings AppsManager::APP_ICON_CACHE("deepin", "dde-launcher-app-icon", nullptr);
QSettings AppsManager::APP_AUTOSTART_CACHE("deepin", "dde-launcher-app-autostart", nullptr);
//QSettings AppsManager::AppInfoCache("deepin", "dde-launcher-app-info", nullptr);

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this)),
    m_fileInfoInter(new DBusFileInfo(this)),
    m_startManagerInter(new DBusStartManager(this))
{
//    refreshAppIconCache();
//    refreshAppAutoStartCache();

    m_appInfoList = m_launcherInter->GetAllItemInfos().value();
    refreshCategoryInfoList();
}

void AppsManager::initConnection()
{
}

AppsManager *AppsManager::instance(QObject *parent)
{
    if (INSTANCE)
        return INSTANCE;

    INSTANCE = new AppsManager(parent);
    return INSTANCE;
}

void AppsManager::removeRow(const int row)
{
    m_appInfoList.removeAt(row);
}

void AppsManager::launchApp(const QModelIndex &index)
{
    m_startManagerInter->LaunchWithTimestamp(index.data(AppsListModel::AppDesktopRole).toString(), QX11Info::getTimestamp());
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

bool AppsManager::appIsAutoStart(const QString &desktop)
{
    if (APP_AUTOSTART_CACHE.contains(desktop))
        return APP_AUTOSTART_CACHE.value(desktop).toBool();

    const bool isAutoStart = m_startManagerInter->IsAutostart(desktop).value();

    APP_AUTOSTART_CACHE.setValue(desktop, isAutoStart);

    return isAutoStart;
}

const QPixmap AppsManager::appIcon(const QString &desktop, const int size)
{
    const QString cacheKey = QString("%1-%2").arg(desktop)
                                             .arg(size);

    const QPixmap cachePixmap = APP_ICON_CACHE.value(cacheKey).value<QPixmap>();
    if (!cachePixmap.isNull())
        return cachePixmap;

    // cache fail
    const QString iconFile = m_fileInfoInter->GetThemeIcon(desktop, size).value();
    QPixmap iconPixmap = QPixmap(iconFile);

    if (iconPixmap.isNull())
        iconPixmap = QPixmap(":/skin/images/application-default-icon.svg");

    APP_ICON_CACHE.setValue(cacheKey, iconPixmap);

    return iconPixmap;
}

ItemInfo AppsManager::getItemInfo(QString appKey) {
    QDBusPendingReply<ItemInfo> reply = m_launcherInter->GetItemInfo(appKey);
    ItemInfo  itemInfo;
    reply.waitForFinished();
    if (!reply.isError()){
        itemInfo = qdbus_cast<ItemInfo>(reply.argumentAt(0));
        return itemInfo;
    }else{
        qCritical() << reply.error().name() << reply.error().message();
        return itemInfo;
    }
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
    APP_ICON_CACHE.sync();
    APP_ICON_CACHE.clear();
}

void AppsManager::refreshAppAutoStartCache()
{
    APP_AUTOSTART_CACHE.sync();
    APP_AUTOSTART_CACHE.clear();
}
