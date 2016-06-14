#include "appsmanager.h"
#include "global_util/constants.h"
#include "global_util/calculate_util.h"

#include <QDebug>
#include <QX11Info>
#include <QSvgRenderer>
#include <QPainter>
#include <QDataStream>
#include <QIODevice>

AppsManager *AppsManager::INSTANCE = nullptr;

QSettings AppsManager::APP_PRESET_SORTED_LIST(
#ifdef ARCH_MIPSEL
    "/usr/share/dde-launcher/data/preset-order-mips.conf",
#else
    "/usr/share/dde-launcher/data/preset-order.conf",
#endif
    QSettings::IniFormat);

QSettings AppsManager::APP_ICON_CACHE("deepin", "dde-launcher-app-icon", nullptr);
QSettings AppsManager::APP_AUTOSTART_CACHE("deepin", "dde-launcher-app-autostart", nullptr);
QSettings AppsManager::APP_USER_SORTED_LIST("deepin", "dde-launcher-app-sorted-list", nullptr);

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this)),
    m_startManagerInter(new DBusStartManager(this)),
    m_dockedAppInter(new DBusDockedAppManager(this)),
    m_themeAppIcon(new ThemeAppIcon(this)),
    m_calUtil(CalculateUtil::instance(this)),
    m_searchTimer(new QTimer(this))
{
    m_themeAppIcon->gtkInit();
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();
    refreshCategoryInfoList();

    if (APP_ICON_CACHE.value("version").toString() != qApp->applicationVersion())
        refreshAppIconCache();

    if (APP_AUTOSTART_CACHE.value("version").toString() != qApp->applicationVersion())
        refreshAppAutoStartCache();

    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(150);

    connect(m_startManagerInter, &DBusStartManager::AutostartChanged, this, &AppsManager::refreshAppAutoStartCache);
    connect(m_launcherInter, &DBusLauncher::SearchDone, this, &AppsManager::searchDone);
    connect(m_launcherInter, &DBusLauncher::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_launcherInter, &DBusLauncher::UninstallFailed, [this] (const QString &appKey) {restoreItem(appKey); emit dataChanged(AppsListModel::All);});
//    connect(m_launcherInter, &DBusLauncher::UninstallFailed, this, &AppsManager::reStoreItem);
    connect(m_launcherInter, &DBusLauncher::ItemChanged, this, &AppsManager::handleItemChanged);
    //Maybe the signals newAppLaunched will be replaced by newAppMarkedAsLaunched
    //newAppLaunched is the old one.
    connect(m_launcherInter, &DBusLauncher::NewAppLaunched, this, &AppsManager::markLaunched);

//    connect(this, &AppsManager::handleUninstallApp, this, &AppsManager::unInstallApp);
    connect(m_searchTimer, &QTimer::timeout, [this] {m_launcherInter->Search(m_searchText);});
}

const QPixmap AppsManager::loadSvg(const QString &fileName, const int size)
{
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}

const QPixmap AppsManager::loadIconFile(const QString &fileName, const int size)
{
    QPixmap pixmap;
    if (fileName.startsWith("data:image/")) {
        //This icon file is an inline image
        QStringList strs = fileName.split("base64,");
        if (strs.length() == 2) {
            QByteArray data = QByteArray::fromBase64(strs.at(1).toLatin1());
            pixmap.loadFromData(data);
        }

    } else if (fileName.endsWith(".svg", Qt::CaseInsensitive))
        pixmap = loadSvg(fileName, size);
    else
        pixmap = QPixmap(fileName);

    if (pixmap.isNull())
        return pixmap;

    return pixmap.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void AppsManager::appendSearchResult(const QString &appKey)
{
    for (const ItemInfo &info : m_allAppInfoList)
        if (info.m_key == appKey)
            return m_appSearchResultList.append(info);
}

void AppsManager::sortCategory(const AppsListModel::AppCategory category)
{
    switch (category)
    {
    case AppsListModel::Search:     sortByPresetOrder(m_appSearchResultList);      break;
//    case AppsListModel::All:        sortByName(m_appInfoList);              break;
    // disable sort other category
    default: Q_ASSERT(false) ;
    }
}

void AppsManager::sortByPresetOrder(ItemInfoList &processList)
{
    QVariant presetFallback = APP_PRESET_SORTED_LIST.value("list");
    QString key = QString("list[%1]").arg(QLocale::system().name());
    QStringList preset = APP_PRESET_SORTED_LIST.value(key, presetFallback).toStringList();

    qSort(processList.begin(), processList.end(), [&preset] (const ItemInfo &i1, const ItemInfo &i2) {
        int index1 = preset.indexOf(i1.m_key);
        int index2 = preset.indexOf(i2.m_key);

        if (index1 == index2) {
            // If both of them don't exist in the preset list,
            // fallback to comparing their name.
            return i1.m_name < i2.m_name;
        }

        // If one of them doesn't exist in the preset list,
        // the one exists go first.
        if (index1 == -1) {
            return false;
        }
        if (index2 == -1) {
            return true;
        }

        // If both of them exist, then obey the preset order.
        return index1 < index2;
    });
}

AppsManager *AppsManager::instance(QObject *parent)
{
    if (INSTANCE)
        return INSTANCE;

    INSTANCE = new AppsManager(parent);
    return INSTANCE;
}

void AppsManager::stashItem(const QModelIndex &index)
{
    const QString key = index.data(AppsListModel::AppKeyRole).toString();

    return stashItem(key);
}

void AppsManager::stashItem(const QString &appKey)
{
    for (int i(0); i != m_allAppInfoList.size(); ++i)
    {
        if (m_allAppInfoList[i].m_key == appKey)
        {
            m_stashList.append(m_allAppInfoList[i]);
            m_allAppInfoList.removeAt(i);
            generateCategoryMap();

            return;
        }
    }
}

void AppsManager::abandonStashedItem(const QString &appKey)
{
    qDebug() << "bana" << appKey;
    for (int i(0); i != m_stashList.size(); ++i)
        if (m_stashList[i].m_key == appKey)
            return m_stashList.removeAt(i);
}

void AppsManager::restoreItem(const QString &appKey, const int pos)
{
    for (int i(0); i != m_stashList.size(); ++i)
    {
        if (m_stashList[i].m_key == appKey)
        {
            // if pos is valid
            if (pos != -1)
                m_userSortedList.insert(pos, m_stashList[i]);
            m_allAppInfoList.append(m_stashList[i]);
            m_stashList.removeAt(i);

            generateCategoryMap();

            return saveUserSortedList();
        }
    }
}

void AppsManager::saveUserSortedList()
{
    // save cache
    QByteArray writeBuf;
    QDataStream out(&writeBuf, QIODevice::WriteOnly);
    out << m_userSortedList;
    APP_USER_SORTED_LIST.setValue("list", writeBuf);
}

void AppsManager::searchApp(const QString &keywords)
{
    m_searchTimer->start();
    m_searchText = keywords;
}

void AppsManager::launchApp(const QModelIndex &index)
{
    const QString appDesktop = index.data(AppsListModel::AppDesktopRole).toString();
    QString appKey = index.data(AppsListModel::AppKeyRole).toString();
    markLaunched(appKey);

    if (!appDesktop.isEmpty())
        m_startManagerInter->LaunchWithTimestamp(appDesktop, QX11Info::getTimestamp());
}

void AppsManager::uninstallApp(const QString &appKey)
{
    // refersh auto start cache
    for (const ItemInfo &info : m_allAppInfoList)
    {
        if (info.m_key == appKey)
        {
            APP_AUTOSTART_CACHE.setValue(info.m_desktop, false);
            break;
        }
    }

    // begin uninstall, remove icon first.
    stashItem(appKey);

    // request backend
    m_launcherInter->RequestUninstall(appKey, false);

    emit dataChanged(AppsListModel::All);

    // refersh search result
    m_searchTimer->start();
}

void AppsManager::markLaunched(QString appKey)
{
    if (appKey.isEmpty() || !m_newInstalledAppsList.contains(appKey))
        return;

    m_newInstalledAppsList.removeOne(appKey);
    m_launcherInter->MarkLaunched(appKey);
}

const ItemInfoList AppsManager::appsInfoList(const AppsListModel::AppCategory &category) const
{
    switch (category)
    {
    case AppsListModel::Custom:
    case AppsListModel::All:        return m_userSortedList;        break;
    case AppsListModel::Search:     return m_appSearchResultList;   break;
    default:;
    }

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

const QPixmap AppsManager::appIcon(const QString &iconKey, const int size)
{
    const QPixmap cachePixmap = APP_ICON_CACHE.value(QString("%1-%2").arg(iconKey).arg(size)).value<QPixmap>();
    if (!cachePixmap.isNull())
        return cachePixmap;

    const QString iconPath = m_themeAppIcon->getThemeIconPath(iconKey, size);
    const QPixmap iconPixmap = loadIconFile(iconPath, size);

    if (!iconPixmap.isNull())
    {
        APP_ICON_CACHE.setValue(QString("%1-%2").arg(iconKey).arg(size), iconPixmap);
        return iconPixmap;
    }

    if (m_defaultIconPixmap.isNull() || m_defaultIconPixmap.width() != size)
        m_defaultIconPixmap = loadSvg(":/skin/images/application-default-icon.svg", size);

    return m_defaultIconPixmap;
}

void AppsManager::refreshCategoryInfoList()
{
    QByteArray readBuf = APP_USER_SORTED_LIST.value("list").toByteArray();
    QDataStream in(&readBuf, QIODevice::ReadOnly);
    in >> m_userSortedList;

    m_allAppInfoList = m_launcherInter->GetAllItemInfos().value();

    generateCategoryMap();
    saveUserSortedList();
}

void AppsManager::generateCategoryMap()
{
    m_appInfos.clear();
    sortByPresetOrder(m_allAppInfoList);

    for (const ItemInfo &info : m_allAppInfoList)
    {
        // append new installed app to user sorted list
        if (!m_userSortedList.contains(info))
            m_userSortedList.append(info);

        const AppsListModel::AppCategory category = info.category();
        if (!m_appInfos.contains(category))
            m_appInfos.insert(category, ItemInfoList());

        m_appInfos[category].append(info);
    }

    // remove uninstalled app item
    for (const ItemInfo &info : m_userSortedList)
        if (!m_allAppInfoList.contains(info))
            m_userSortedList.removeOne(info);
}

int AppsManager::appNums(const AppsListModel::AppCategory &category) const
{
    return appsInfoList(category).size();
}

//void AppsManager::unInstallApp(const QModelIndex &index, int value) {
//    QString appKey = index.data(AppsListModel::AppKeyRole).toString();
//    if (value==1) {
//        // begin to unInstall app, remove icon firstly;
//        QDBusPendingReply<ItemInfo> reply = m_launcherInter->GetItemInfo(appKey);
//        reply.waitForFinished();
//        if (reply.isValid() && !reply.isError()) {
//            m_unInstallItem = qdbus_cast<ItemInfo>(reply.argumentAt(0));
//            m_allAppInfoList.removeOne(m_unInstallItem);

//            emit dataChanged(AppsListModel::All);
//            refreshAppIconCache();
//            refreshCategoryInfoList();
//            //if after the app be unInstalled, it's category's app number is zero, update the view
//            if (appsInfoList(m_unInstallItem.category()).size()==0) {
//                emit updateCategoryView(m_unInstallItem.category());
//            }

//            emit dataChanged(m_unInstallItem.category());

//            //Uninstall app from backend;
//            QDBusPendingReply<> reply = m_launcherInter->RequestUninstall(appKey, false);
//            if (!reply.isError()) {
////                qDebug() << "unistall function excute finished!";
//            } else {
////                qDebug() << "unistall action fail, and the error reason:" << reply.error().message();
//            }
//        } else {
////            qDebug() << "get unInstall app itemInfo failed!";
//        }
//    } else {
//        //cancle to unInstall app;
////        qDebug() << "cancle to unInstall app" << appKey;
//    }
//}

//void AppsManager::reStoreItem() {
//    bool updateViewFlag = false;
//    if (appsInfoList(m_unInstallItem.category()).length() == 0) {
//        updateViewFlag = true;
//    }
//    m_allAppInfoList.append(m_unInstallItem);
//    emit dataChanged(AppsListModel::All);
//    emit dataChanged(m_unInstallItem.category());
//    refreshCategoryInfoList();
//    if (updateViewFlag) {
//        emit updateCategoryView(m_unInstallItem.category());
//    }
//}

void AppsManager::refreshAppIconCache()
{
    APP_ICON_CACHE.clear();
    APP_ICON_CACHE.setValue("version", qApp->applicationVersion());
//    return;

//    int appIconSize = m_calUtil->appIconSize().width();

//    qDebug() << "get icon size:" << appIconSize;
//    // generate cache
//    for (const ItemInfo &info : m_appInfoList)
//    {
//        const QPixmap cachePixmap = APP_ICON_CACHE.value(QString("%1-%2").arg(info.m_iconKey).arg(appIconSize)).value<QPixmap>();
//        if (!cachePixmap.isNull())
//            continue;

//        const QString iconPath = m_themeAppIcon->getThemeIconPath(info.m_iconKey, appIconSize);
//        const QPixmap iconPixmap = loadIconFile(iconPath, appIconSize);

//        if (!iconPixmap.isNull())
//            APP_ICON_CACHE.setValue(QString("%1-%2").arg(info.m_iconKey).arg(appIconSize), iconPixmap);
//    }

//    emit dataChanged(AppsListModel::All);
}

void AppsManager::refreshAppAutoStartCache()
{
    APP_AUTOSTART_CACHE.setValue("version", qApp->applicationVersion());

    for (const ItemInfo &info : m_allAppInfoList)
    {
        const bool isAutoStart = m_startManagerInter->IsAutostart(info.m_desktop).value();
        APP_AUTOSTART_CACHE.setValue(info.m_desktop, isAutoStart);
    }

    emit dataChanged(AppsListModel::All);
}

void AppsManager::searchDone(const QStringList &resultList)
{
    m_appSearchResultList.clear();

    for (const QString &key : resultList)
        appendSearchResult(key);

    emit dataChanged(AppsListModel::Search);

    if (m_appSearchResultList.isEmpty())
        emit requestTips(tr("No search results"));
    else
        emit requestHideTips();
}

//void AppsManager::handleDragedApp(const QModelIndex &index, int nextNode) {
//    qDebug() << "draged app";
//    QString appKey = index.data(AppsListModel::AppKeyRole).toString();
//    // begin to unInstall app, remove icon firstly;
//    QDBusPendingReply<ItemInfo> reply = m_launcherInter->GetItemInfo(appKey);
//    reply.waitForFinished();
//    if (reply.isValid() && !reply.isError()) {
//        ItemInfo dragedAppItemInfo = qdbus_cast<ItemInfo>(reply.argumentAt(0));
//        m_beDragedItem = dragedAppItemInfo;

//        restoreItem(appKey, nextNode);
//        qDebug() << "remove one" << nextNode;
//        emit dataChanged(AppsListModel::All);
//        refreshAppIconCache();
//        refreshCategoryInfoList();
//    }
//}

//void AppsManager::handleDropedApp(const QModelIndex &index) {
//    QString appKey = index.data(AppsListModel::AppKeyRole).toString();
//    QDBusPendingReply<ItemInfo> reply = m_launcherInter->GetItemInfo(appKey);
//    reply.waitForFinished();
//    if (reply.isValid() && !reply.isError()) {
//        ItemInfo insertAppItemInfo = qdbus_cast<ItemInfo>(reply.argumentAt(0));
//        int i = m_appInfoList.indexOf(insertAppItemInfo);

//        m_appInfoList.insert(i, m_beDragedItem);
//        emit dataChanged(AppsListModel::All);
//        refreshAppIconCache();
//        m_beDragedItem = ItemInfo();
//    }
//}

// TODO: optimize
void AppsManager::handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber)
{
    qDebug() << "in0" << operation << appInfo.m_name << "in2" << categoryNumber;

    if (operation == "created")
        m_newInstalledAppsList.append(appInfo.m_key);

    refreshCategoryInfoList();
    refreshAppIconCache();

    emit dataChanged(AppsListModel::All);
}
