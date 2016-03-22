#include "appsmanager.h"
#include "global_util/constants.h"


#include <QDebug>
#include <QX11Info>
#include <QSvgRenderer>
#include <QPainter>

AppsManager *AppsManager::INSTANCE = nullptr;

QSettings AppsManager::APP_ICON_CACHE("deepin", "dde-launcher-app-icon", nullptr);
QSettings AppsManager::APP_AUTOSTART_CACHE("deepin", "dde-launcher-app-autostart", nullptr);

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this)),
    m_fileInfoInter(new DBusFileInfo(this)),
    m_startManagerInter(new DBusStartManager(this)),
    m_dockedAppInter(new DBusDockedAppManager(this)),
    m_displayInterface(new DBusDisplay(this)),
    m_searchTimer(new QTimer(this))
{
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();
    m_appInfoList = m_launcherInter->GetAllItemInfos().value();
    sortCategory(AppsListModel::All);
    refreshCategoryInfoList();

    if (APP_ICON_CACHE.value("version").toString() != qApp->applicationVersion())
        refreshAppIconCache();

    if (APP_AUTOSTART_CACHE.value("version").toString() != qApp->applicationVersion())
        refreshAppAutoStartCache();

    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(150);

    QSize screenSize = getPrimayRect().size();

    calUtil = new CalculateUtil(QSize(screenSize.width() - 160*2, screenSize.height()));

    connect(m_startManagerInter, &DBusStartManager::AutostartChanged, this, &AppsManager::refreshAppAutoStartCache);
    connect(m_launcherInter, &DBusLauncher::SearchDone, this, &AppsManager::searchDone);
    connect(m_launcherInter, &DBusLauncher::UninstallFailed, this, &AppsManager::reStoreItem);
    connect(this, &AppsManager::handleUninstallApp, this, &AppsManager::unInstallApp);
    connect(m_searchTimer, &QTimer::timeout, [this] {m_launcherInter->Search(m_searchText);});
    connect(m_displayInterface, &DBusDisplay::PrimaryChanged, this,  &AppsManager::primaryChanged);
    connect(m_displayInterface, &DBusDisplay::PrimaryRectChanged, this, &AppsManager::primaryChanged);
}

void AppsManager::appendSearchResult(const QString &appKey)
{
    for (const ItemInfo &info : m_appInfoList)
        if (info.m_key == appKey)
            return m_appSearchResultList.append(info);
}

void AppsManager::sortCategory(const AppsListModel::AppCategory category)
{
    switch (category)
    {
    case AppsListModel::Search:     sortByName(m_appSearchResultList);      break;
    case AppsListModel::All:        sortByName(m_appInfoList);              break;
    default:;
    }
}

void AppsManager::sortByName(ItemInfoList &processList)
{
    qSort(processList.begin(), processList.end(), [] (const ItemInfo &i1, const ItemInfo &i2) {
        return i1.m_name < i2.m_name;
    });
}

AppsManager *AppsManager::instance(QObject *parent)
{
    if (INSTANCE)
        return INSTANCE;

    INSTANCE = new AppsManager(parent);
    return INSTANCE;
}

void AppsManager::searchApp(const QString &keywords)
{
    m_searchTimer->start();
    m_searchText = keywords;
}

void AppsManager::launchApp(const QModelIndex &index)
{
    const QString appDesktop = index.data(AppsListModel::AppDesktopRole).toString();

    qDebug() << "Launch app:" << appDesktop;

    m_startManagerInter->LaunchWithTimestamp(appDesktop, QX11Info::getTimestamp());
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

const QPixmap AppsManager::appIcon(const QString &desktop)
{
    const QPixmap cachePixmap = APP_ICON_CACHE.value(desktop).value<QPixmap>();
    if (!cachePixmap.isNull())
        return cachePixmap;

    // cache fail
    const QString iconFile = m_fileInfoInter->GetThemeIcon(desktop, DLauncher::APP_ICON_SIZE).value();
    QPixmap iconPixmap;

    if (iconFile.endsWith(".svg", Qt::CaseInsensitive))
        iconPixmap = loadSvg(iconFile, DLauncher::APP_ICON_SIZE);
    else
        iconPixmap = QPixmap(iconFile);

    if (iconPixmap.isNull())
        iconPixmap = loadSvg(":/skin/images/application-default-icon.svg", DLauncher::APP_ICON_SIZE);

    APP_ICON_CACHE.setValue(desktop, iconPixmap);

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

int AppsManager::appNums(const AppsListModel::AppCategory &category) const
{
    return appsInfoList(category).size();
}

void AppsManager::unInstallApp(const QModelIndex &index, int value) {
    QString appKey = index.data(AppsListModel::AppKeyRole).toString();
    if (value==1) {
        // begin to unInstall app, remove icon firstly;
        QDBusPendingReply<ItemInfo> reply = m_launcherInter->GetItemInfo(appKey);
        reply.waitForFinished();
        if (reply.isValid() && !reply.isError()) {
            m_unInstallItem = qdbus_cast<ItemInfo>(reply.argumentAt(0));
            m_appInfoList.removeOne(m_unInstallItem);

            emit dataChanged(AppsListModel::All);
            refreshAppIconCache();
            refreshCategoryInfoList();
            //Uninstall app from backend;
            QDBusPendingReply<> reply = m_launcherInter->RequestUninstall(appKey, false);
            if (!reply.isError()) {
                qDebug() << "unistall function excute finished!";
            } else {
                qDebug() << "unistall action fail, and the error reason:" << reply.error().message();
            }
        } else {
            qDebug() << "get unInstall app itemInfo failed!";
        }
    } else {
        //cancle to unInstall app;
        qDebug() << "cancle to unInstall app" << appKey;
    }
}

void AppsManager::reStoreItem() {
    m_appInfoList.append(m_unInstallItem);
}

void AppsManager::refreshAppIconCache()
{
    APP_ICON_CACHE.setValue("version", qApp->applicationVersion());

    // generate cache
    for (const ItemInfo &info : m_appInfoList)
    {
        const QString iconFile = m_fileInfoInter->GetThemeIcon(info.m_desktop, DLauncher::APP_ICON_SIZE).value();
        QPixmap iconPixmap;

        if (iconFile.endsWith(".svg", Qt::CaseInsensitive))
            iconPixmap = loadSvg(iconFile, DLauncher::APP_ICON_SIZE);
        else
            iconPixmap = QPixmap(iconFile);

        if (iconPixmap.isNull())
            iconPixmap = loadSvg(":/skin/images/application-default-icon.svg", DLauncher::APP_ICON_SIZE);

        APP_ICON_CACHE.setValue(info.m_desktop, iconPixmap);
    }

    emit dataChanged(AppsListModel::All);
}

void AppsManager::refreshAppAutoStartCache()
{
    APP_AUTOSTART_CACHE.setValue("version", qApp->applicationVersion());

    for (const ItemInfo &info : m_appInfoList)
    {
        const bool isAutoStart = m_startManagerInter->IsAutostart(info.m_desktop).value();
        APP_AUTOSTART_CACHE.setValue(info.m_desktop, isAutoStart);
    }

    emit dataChanged(AppsListModel::All);
}

const QPixmap AppsManager::loadSvg(const QString &fileName, const int size)
{
    QPixmap image(size, size);
    image.fill(Qt::transparent);
    QSvgRenderer renderer(fileName);
    QPainter painter(&image);

    renderer.render(&painter);

    return image;
}

void AppsManager::searchDone(const QStringList &resultList)
{
    m_appSearchResultList.clear();

    for (const QString &key : resultList)
        appendSearchResult(key);

    emit dataChanged(AppsListModel::Search);
}

QRect AppsManager::getPrimayRect() {
    QRect primaryRect = QRect(m_displayInterface->primaryRect());
    qDebug() << "primaryRect: " << primaryRect;
    return primaryRect;
}
