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
    m_startManagerInter(new DBusStartManager(this)),
    m_dockedAppInter(new DBusDockedAppManager(this))
{
//    refreshAppIconCache();
//    refreshAppAutoStartCache();

    m_appInfoList = m_launcherInter->GetAllItemInfos().value();
    refreshCategoryInfoList();

    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();
    categoryAppNums();
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

bool AppsManager::appIsNewInstall(const QString &key)
{
    return m_newInstalledAppsList.contains(key);
}

bool AppsManager::appIsAutoStart(const QString &desktop)
{
    if (APP_AUTOSTART_CACHE.contains(desktop))
        return APP_AUTOSTART_CACHE.value(desktop).toBool();

    const bool isAutoStart = m_startManagerInter->IsAutostart(desktop).value();

    APP_AUTOSTART_CACHE.setValue(desktop, isAutoStart);

    return isAutoStart;
}

bool AppsManager::appIsOnDock(const QString &appName)
{
    return m_dockedAppInter->IsDocked(appName).value();
}

bool AppsManager::appIsOnDesktop(const QString &desktop)
{
    return m_launcherInter->IsItemOnDesktop(desktop).value();
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

void AppsManager::categoryAppNums() {
    int appNums = 0;
    foreach (QString key,  this->CategoryKeys) {
        if (key == "Internet") {
            appNums = appsInfoList(AppsListModel::Internet).length();
            m_categoryAppNumsMap.insert(AppsListModel::Internet, appNums);
        }
        if (key == "Chat") {
                 appNums = appsInfoList(AppsListModel::Chat).length();
                 m_categoryAppNumsMap.insert(AppsListModel::Chat, appNums);
                qDebug() << "Chat:" << appNums;
        }
        if (key == "Music") {
                appNums = appsInfoList(AppsListModel::Music).length();
                m_categoryAppNumsMap.insert(AppsListModel::Music, appNums);
                qDebug() << "Music:" << appNums;
        }
        if (key == "Video") {
                 appNums = appsInfoList(AppsListModel::Video).length();
                 m_categoryAppNumsMap.insert(AppsListModel::Video, appNums);
                qDebug() << "Video:" << appNums;
        }
        if (key == "Game") {
               appNums = appsInfoList(AppsListModel::Game).length();
               m_categoryAppNumsMap.insert(AppsListModel::Game, appNums);
                qDebug() << "Game:" << appNums;
        }
        if (key == "Office") {
              appNums =appsInfoList(AppsListModel::Office).length();
              m_categoryAppNumsMap.insert(AppsListModel::Office, appNums);
                qDebug() << "Office:" << appNums;
        }
        if (key == "Reading") {
             appNums = appsInfoList(AppsListModel::Reading).length();
             m_categoryAppNumsMap.insert(AppsListModel::Reading, appNums);
                qDebug() << "Reading:" << appNums;
        }
        if (key == "Development") {
            appNums =  appsInfoList(AppsListModel::Development).length();
            m_categoryAppNumsMap.insert(AppsListModel::Development, appNums);
                qDebug() << "Development:" << appNums;
        }
        if (key == "System") {
            appNums = appsInfoList(AppsListModel::System).length();
            m_categoryAppNumsMap.insert(AppsListModel::System, appNums);
                qDebug() << "System:" << appNums;
        }
        if (key == "Others") {
            appNums = appsInfoList(AppsListModel::Others).length();
            m_categoryAppNumsMap.insert(AppsListModel::Others, appNums);
                qDebug() << "Others:" << appNums;
        }
    }
}

int AppsManager::getCategoryAppNums(const AppsListModel::AppCategory &category) {
    return m_categoryAppNumsMap.value(category);
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
