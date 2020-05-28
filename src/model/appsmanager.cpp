/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "appsmanager.h"
#include "src/global_util/util.h"
#include "src/global_util/constants.h"
#include "src/global_util/calculate_util.h"

#include <QDebug>
#include <QX11Info>
#include <QSvgRenderer>
#include <QPainter>
#include <QDataStream>
#include <QIODevice>
#include <QIcon>

#include <QGSettings>
#include <DHiDPIHelper>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>
#include <qiconengine.h>
#include <DApplication>
#include <QScopedPointer>

DWIDGET_USE_NAMESPACE

QPointer<AppsManager> AppsManager::INSTANCE = nullptr;

QGSettings AppsManager::LAUNCHER_SETTINGS("com.deepin.dde.launcher", "", nullptr);
QSet<QString> AppsManager::APP_AUTOSTART_CACHE;
QSettings AppsManager::APP_USER_SORTED_LIST("deepin", "dde-launcher-app-sorted-list", nullptr);
QSettings AppsManager::APP_USED_SORTED_LIST("deepin", "dde-launcher-app-used-sorted-list");
static constexpr int USER_SORT_UNIT_TIME = 3600; // 1 hours

int perfectIconSize(const int size)
{
    const int s = 8;
    const int l[s] = { 16, 24, 32, 48, 64, 96, 128, 256 };

    for (int i(0); i != s; ++i)
        if (size < l[i])
            return l[i];

    return 256;
}

const QPixmap AppsManager::getThemeIcon(const ItemInfo &itemInfo, const int size)
{
    const QString &iconName = itemInfo.m_iconKey;
    const auto ratio = qApp->devicePixelRatio();
    const int s = perfectIconSize(size);
    QPlatformTheme *const platformTheme = QGuiApplicationPrivate::platformTheme();

    QPixmap pixmap;
    do {
        if (iconName.startsWith("data:image/")) {
            const QStringList strs = iconName.split("base64,");
            if (strs.size() == 2)
                pixmap.loadFromData(QByteArray::fromBase64(strs.at(1).toLatin1()));

            if (!pixmap.isNull())
                break;
        }

        if (QFile::exists(iconName)) {
            if (iconName.endsWith(".svg"))
                pixmap = loadSvg(iconName, s * ratio);
            else
                pixmap = DHiDPIHelper::loadNxPixmap(iconName);

            if (!pixmap.isNull())
                break;
        }

        QScopedPointer<QIconEngine> engine(platformTheme->createIconEngine(iconName));
        QIcon icon;

        if (!engine.isNull()) {
            if (engine->isNull()) {
                auto iterator =
                    std::find_if(m_notExistIconMap.begin(), m_notExistIconMap.end(),
                [ = ](const std::pair<std::pair<ItemInfo, int>, int> value) {
                    return value.first.first.m_iconKey == iconName && value.first.second == size;
                });

                if (iterator == m_notExistIconMap.end()) {
                    if (!iconName.isEmpty()) {
                        const std::pair<ItemInfo, int> pair{ itemInfo, size };
                        m_notExistIconMap[pair] = 0;
                        m_iconRefreshTimer->start();
                    }
                    icon = QIcon::fromTheme(iconName);
                    if(icon.isNull()){
                            icon = QIcon::fromTheme("deepinwine-"+iconName, QIcon::fromTheme("application-x-desktop"));
                    }
                }
            } else {
               icon = QIcon::fromTheme(iconName);
               if(icon.isNull()){
                       icon = QIcon::fromTheme("deepinwine-"+iconName, QIcon::fromTheme("application-x-desktop"));
               }
            }
        }

        pixmap = icon.pixmap(QSize(s, s));
        if (!pixmap.isNull())
            break;

        pixmap = loadSvg(":/skin/images/application-default-icon.svg", s * ratio);
        Q_ASSERT(!pixmap.isNull());
    } while (false);

    if (qFuzzyCompare(pixmap.devicePixelRatioF(), 1.))
    {
        pixmap = pixmap.scaled(QSize(s, s) * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(ratio);
    }

    return pixmap;
}

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this)),
    m_startManagerInter(new DBusStartManager(this)),
    m_dockInter(new DBusDock(this)),
    m_iconRefreshTimer(std::make_unique<QTimer>(new QTimer)),
    m_calUtil(CalculateUtil::instance()),
    m_searchTimer(new QTimer(this)),
    m_delayRefreshTimer(new QTimer(this))
{
    m_iconRefreshTimer->setInterval(10 * 1000);
    m_iconRefreshTimer->setSingleShot(false);

    m_categoryTs
            << tr("Internet")
            << tr("Chat")
            << tr("Music")
            << tr("Video")
            << tr("Graphics")
            << tr("Games")
            << tr("Office")
            << tr("Reading")
            << tr("Development")
            << tr("System")
            << tr("Other");

    m_categoryIcon
            << QString(":/icons/skin/icons/internet_normal_22px.svg")
            << QString(":/icons/skin/icons/chat_normal_22px.svg")
            << QString(":/icons/skin/icons/music_normal_22px.svg")
            << QString(":/icons/skin/icons/multimedia_normal_22px.svg")
            << QString(":/icons/skin/icons/graphics_normal_22px.svg")
            << QString(":/icons/skin/icons/game_normal_22px.svg")
            << QString(":/icons/skin/icons/office_normal_22px.svg")
            << QString(":/icons/skin/icons/reading_normal_22px.svg")
            << QString(":/icons/skin/icons/development_normal_22px.svg")
            << QString(":/icons/skin/icons/system_normal_22px.svg")
            << QString(":/icons/skin/icons/others_normal_22px.svg");

    refreshAllList();
    refreshAppAutoStartCache();

    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(150);
    m_delayRefreshTimer->setSingleShot(true);
    m_delayRefreshTimer->setInterval(500);

    connect(qApp, &DApplication::iconThemeChanged, this, &AppsManager::onIconThemeChanged, Qt::QueuedConnection);
    connect(m_launcherInter, &DBusLauncher::NewAppLaunched, this, &AppsManager::markLaunched);
    connect(m_launcherInter, &DBusLauncher::SearchDone, this, &AppsManager::searchDone);
    connect(m_launcherInter, &DBusLauncher::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_launcherInter, &DBusLauncher::UninstallFailed, [this] (const QString &appKey) { restoreItem(appKey); emit dataChanged(AppsListModel::All); });
    connect(m_launcherInter, &DBusLauncher::ItemChanged, this, &AppsManager::handleItemChanged);
    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &AppsManager::dockGeometryChanged);
    connect(m_dockInter, &DBusDock::IconSizeChanged, this, &AppsManager::dockGeometryChanged);
    connect(m_startManagerInter, &DBusStartManager::AutostartChanged, this, &AppsManager::refreshAppAutoStartCache);
    connect(m_delayRefreshTimer, &QTimer::timeout, this, &AppsManager::delayRefreshData);
    connect(m_searchTimer, &QTimer::timeout, this, &AppsManager::onSearchTimeOut);
    connect(m_iconRefreshTimer.get(), &QTimer::timeout, this, &AppsManager::refreshNotFoundIcon);
}

void AppsManager::appendSearchResult(const QString &appKey)
{
    for (const ItemInfo &info : m_allAppInfoList)
        if (info.m_key == appKey)
            return m_appSearchResultList.append(info);
}

void AppsManager::sortCategory(const AppsListModel::AppCategory category)
{
    switch (category) {
    case AppsListModel::Search:     sortByPresetOrder(m_appSearchResultList);      break;
//    case AppsListModel::All:        sortByName(m_appInfoList);              break;
    // disable sort other category
    default: Q_ASSERT(false) ;
    }
}

void AppsManager::sortByPresetOrder(ItemInfoList &processList)
{
    const QString system_lang = QLocale::system().name();

    QString key = "appsOrder";
    for (const auto &item : system_lang.split('_')) {
        Q_ASSERT(!item.isEmpty());

        QString k = item.toLower();
        k[0] = k[0].toUpper();

        key.append(k);
    }

//    qDebug() << "preset order: " << key << APP_PRESET_SORTED_LIST.keys();
    QStringList preset;
    if (LAUNCHER_SETTINGS.keys().contains(key))
        preset = LAUNCHER_SETTINGS.get(key).toStringList();
    if (preset.isEmpty())
        preset = LAUNCHER_SETTINGS.get("apps-order").toStringList();

    qSort(processList.begin(), processList.end(), [&preset] (const ItemInfo &i1, const ItemInfo &i2) {
        int index1 = preset.indexOf(i1.m_key.toLower());
        int index2 = preset.indexOf(i2.m_key.toLower());

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

void AppsManager::sortByInstallTimeOrder(ItemInfoList &processList)
{
    qSort(processList.begin(), processList.end(), [&](const ItemInfo & i1, const ItemInfo & i2) {

        if (i1.m_installedTime == i2.m_installedTime && i1.m_installedTime != 0) {
            // If both of them don't exist in the preset list,
            // fallback to comparing their name.
            return i1.m_installedTime < i2.m_installedTime;
        }

        // If one of them doesn't exist in the preset list,
        // the one exists go first.
        if (i1.m_installedTime == 0) {
            return false;
        }

        if (i2.m_installedTime == 0) {
            return true;
        }

        // If both of them exist, then obey the preset order.
        return i1.m_installedTime < i2.m_installedTime;
    });
}

AppsManager *AppsManager::instance()
{
    if (INSTANCE.isNull())
        INSTANCE = new AppsManager;

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
            refreshUsedInfoList();
            refreshUserInfoList();

            return;
        }
    }
}

void AppsManager::abandonStashedItem(const QString &appKey)
{
    //qDebug() << "bana" << appKey;
    for (int i(0); i != m_stashList.size(); ++i) {
        if (m_stashList[i].m_key == appKey) {
            m_stashList.removeAt(i);
            break;
        }
    }

    emit dataChanged(AppsListModel::All);
}

void AppsManager::restoreItem(const QString &appKey, const int pos)
{
    for (int i(0); i != m_stashList.size(); ++i)
    {
        if (m_stashList[i].m_key == appKey)
        {
            // if pos is valid
            if (pos != -1)
                m_usedSortedList.insert(pos, m_stashList[i]);
            m_allAppInfoList.append(m_stashList[i]);
            m_stashList.removeAt(i);

            generateCategoryMap();

            return saveUsedSortedList();
        }
    }
}

int AppsManager::dockPosition() const
{
    return m_dockInter->position();
}

int AppsManager::dockWidth() const
{
    return QRect(m_dockInter->frontendRect()).width();
}

QRect AppsManager::dockGeometry() const
{
    return std::move(m_dockInter->frontendRect());
}

bool AppsManager::isVaild()
{
    return m_launcherInter->isValid() && !m_allAppInfoList.isEmpty();
}

void AppsManager::refreshAllList()
{
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();

    refreshCategoryInfoList();
    refreshUsedInfoList();
    refreshUserInfoList();
}

void AppsManager::saveUserSortedList()
{
    // save cache
    QByteArray writeBuf;
    QDataStream out(&writeBuf, QIODevice::WriteOnly);
    out << m_userSortedList;

    APP_USER_SORTED_LIST.setValue("list", writeBuf);
}

void AppsManager::saveUsedSortedList()
{
    QByteArray writeBuf;
    QDataStream out(&writeBuf, QIODevice::WriteOnly);
    out << m_usedSortedList;

    APP_USED_SORTED_LIST.setValue("list", writeBuf);
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

    for (ItemInfo &info : m_userSortedList) {
        if (info.m_key == appKey) {
            const int idx = m_userSortedList.indexOf(info);

            if (idx != -1) {
                m_userSortedList[idx].m_openCount++;

                if (m_userSortedList[idx].m_firstRunTime == 0) {
                    m_userSortedList[idx].m_firstRunTime = QDateTime::currentMSecsSinceEpoch() / 1000;
                }
            }

            break;
        }
    }

    refreshUserInfoList();

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
            APP_AUTOSTART_CACHE.remove(info.m_desktop);
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

    emit newInstallListChanged();
}

void AppsManager::delayRefreshData()
{
    // refresh new installed apps
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();

    generateCategoryMap();
    saveUserSortedList();

    emit newInstallListChanged();

    emit dataChanged(AppsListModel::All);
}

const ItemInfo AppsManager::createOfCategory(qlonglong category)
{
    ItemInfo info;
    info.m_name = m_categoryTs[category];
    info.m_categoryId = category;
    info.m_iconKey = m_categoryIcon[category];
    return info;
}

const ItemInfoList AppsManager::appsInfoList(const AppsListModel::AppCategory &category) const
{
    switch (category) {
    case AppsListModel::Custom:    return m_userSortedList;        break;
    case AppsListModel::All:       return m_usedSortedList;        break;
    case AppsListModel::Search:     return m_appSearchResultList;   break;
    case AppsListModel::Category:   return m_categoryList;          break;
    default:;
    }

    return m_appInfos[category];
}

ItemInfoList& AppsManager::appInfoList(const AppsListModel::AppCategory &category)
{
    switch (category) {
    case AppsListModel::Custom:    return m_userSortedList;        break;
    case AppsListModel::All:       return m_usedSortedList;        break;
    case AppsListModel::Search:     return m_appSearchResultList;   break;
    case AppsListModel::Category:   return m_categoryList;          break;
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
    return APP_AUTOSTART_CACHE.contains(desktop.split("/").last());
}

bool AppsManager::appIsOnDock(const QString &desktop)
{
    return m_dockInter->IsDocked(desktop);
}

bool AppsManager::appIsOnDesktop(const QString &desktop)
{
    return m_launcherInter->IsItemOnDesktop(desktop).value();
}

bool AppsManager::appIsProxy(const QString &desktop)
{
    return m_launcherInter->GetUseProxy(desktop).value();
}

bool AppsManager::appIsEnableScaling(const QString &desktop)
{
    return !m_launcherInter->GetDisableScaling(desktop);
}

const QPixmap AppsManager::appIcon(const ItemInfo &info, const int size)
{
    QPair<QString, int> tmpKey { info.m_iconKey, size };

    if (m_iconCache.contains(tmpKey) && !m_iconCache[tmpKey].isNull()) {
        return m_iconCache[tmpKey];
    }

    const QPixmap &pixmap = getThemeIcon(info, size);

    m_iconCache[tmpKey] = pixmap;

    return pixmap;
}

void AppsManager::refreshCategoryInfoList()
{
    QDBusPendingReply<ItemInfoList> reply = m_launcherInter->GetAllItemInfos();
    if (reply.isError()) {
        qWarning() << "data is empty, quit!!";
        qWarning() << reply.error();
        qApp->quit();
    }

    QByteArray readBuf = APP_USED_SORTED_LIST.value("list").toByteArray();
    QDataStream in(&readBuf, QIODevice::ReadOnly);
    in >> m_usedSortedList;

    const ItemInfoList &datas = reply.value();
    m_allAppInfoList.clear();
    m_allAppInfoList.reserve(datas.size());
    for (const auto &it : datas) {
        if (!m_stashList.contains(it)) {
            m_allAppInfoList.append(it);
        }
    }

    generateCategoryMap();
}

void AppsManager::refreshUsedInfoList()
{
    // init data if used sorted list is empty.
    if (m_usedSortedList.isEmpty()) {
        // first reads the config file.
        QByteArray readBuffer = APP_USED_SORTED_LIST.value("list").toByteArray();
        QDataStream in(&readBuffer, QIODevice::ReadOnly);
        in >> m_usedSortedList;

        // if data cache file is empty.
        if (m_usedSortedList.isEmpty()) {
            m_usedSortedList = m_allAppInfoList;
        }

        // add new additions
        for (QList<ItemInfo>::ConstIterator it = m_allAppInfoList.constBegin(); it != m_allAppInfoList.constEnd(); ++it) {
            if (!m_usedSortedList.contains(*it)) {
                m_usedSortedList.append(*it);
            }
        }

        // check used list isvaild
        for (QList<ItemInfo>::iterator it = m_usedSortedList.begin(); it != m_usedSortedList.end();) {
            if (m_allAppInfoList.contains(*it)) {
                it++;
            } else {
                it = m_usedSortedList.erase(it);
            }
        }

        updateUsedListInfo();
    }

    saveUsedSortedList();
}

void AppsManager::refreshUserInfoList()
{
    if (m_userSortedList.isEmpty()) {
        // first reads the config file.
        QByteArray readBuffer = APP_USER_SORTED_LIST.value("list").toByteArray();
        QDataStream in(&readBuffer, QIODevice::ReadOnly);
        in >> m_userSortedList;

        // if data cache file is empty.
        if (m_userSortedList.isEmpty()) {
            m_userSortedList = m_allAppInfoList;
        } else {
            // check used list isvaild
            for (QList<ItemInfo>::iterator it = m_userSortedList.begin(); it != m_userSortedList.end();) {
                int idx = m_allAppInfoList.indexOf(*it);

                if (idx >= 0) {
                    // 更新app的其它信息（sort list中可能未保存app的所有信息）
                    const int openCount = it->m_openCount;
                    it->updateInfo(m_allAppInfoList.at(idx));
                    it->m_openCount = openCount;

                    if (it->m_openCount > 0 && it->m_firstRunTime == 0) {
                        // 对于未曾记录过第一次运行时间的应用（但是确保打开过），假定其单位小时打开次数为1，以此为根据给它一个有效的firstRunTime
                        it->m_firstRunTime = QDateTime::currentMSecsSinceEpoch() / 1000 - it->m_openCount * USER_SORT_UNIT_TIME;
                    }

                    it++;
                }
                else {
                    it = m_userSortedList.erase(it);
                }
            }

            // add new additions
            for (QList<ItemInfo>::Iterator it = m_allAppInfoList.begin(); it != m_allAppInfoList.end(); ++it) {
                if (!m_userSortedList.contains(*it)) {
                    m_userSortedList.append(*it);
                }
            }
        }
    }

    const qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    // If the first run time is less than the current time, I am not sure can maintain the correct results.
    std::stable_sort(m_userSortedList.begin(), m_userSortedList.end(), [=] (const ItemInfo &a, const ItemInfo &b) {
        const auto AFirstRunTime = a.m_firstRunTime;
        const auto BFirstRunTime = b.m_firstRunTime;

        // If it's past time, will be sorted by open count
        if (AFirstRunTime > currentTime || BFirstRunTime > currentTime) {
            return a.m_openCount > b.m_openCount;
        }

        int hours_diff_a = (currentTime - AFirstRunTime) / USER_SORT_UNIT_TIME + 1;
        int hours_diff_b = (currentTime - BFirstRunTime) / USER_SORT_UNIT_TIME + 1;

        // Average number of starts
        return (static_cast<double>(a.m_openCount) / hours_diff_a) > (static_cast<double>(b.m_openCount) / hours_diff_b);
    });

    saveUserSortedList();
}

void AppsManager::updateUsedListInfo()
{
    for (const ItemInfo &info : m_allAppInfoList) {
        const int idx = m_usedSortedList.indexOf(info);

        if (idx != -1) {
            const int openCount = m_usedSortedList[idx].m_openCount;
            m_usedSortedList[idx].updateInfo(info);
            m_usedSortedList[idx].m_openCount = openCount;
        }
    }
}

void AppsManager::generateCategoryMap()
{
    m_appInfos.clear();
    m_categoryList.clear();
    sortByPresetOrder(m_allAppInfoList);

    ItemInfoList newInstallAppList;
    for (const ItemInfo &info : m_allAppInfoList) {

        const int userIdx = m_usedSortedList.indexOf(info);
        // append new installed app to user sorted list
        if (userIdx == -1) {
            m_usedSortedList.append(info);
        } else {
            const int openCount = m_usedSortedList[userIdx].m_openCount;
            m_usedSortedList[userIdx].updateInfo(info);
            m_usedSortedList[userIdx].m_openCount = openCount;
        }

        const AppsListModel::AppCategory category = info.category();
        if (!m_appInfos.contains(category))
            m_appInfos.insert(category, ItemInfoList());

        if (!m_newInstalledAppsList.contains(info.m_key)){
            m_appInfos[category].append(info);
        } else {
            newInstallAppList.append(info);
        }
    }


    sortByInstallTimeOrder(newInstallAppList);
    if (!newInstallAppList.isEmpty()) {
        for (const ItemInfo &info : newInstallAppList){
           m_appInfos[info.category()].append(info);
      }
    }

    // remove uninstalled app item
    for (auto it(m_usedSortedList.begin()); it != m_usedSortedList.end();) {
        const int idx = m_allAppInfoList.indexOf(*it);

        if (idx == -1)
            it = m_usedSortedList.erase(it);
        else
            ++it;
    }

    std::list<qlonglong> categoryID;
    for (const ItemInfo &it : m_allAppInfoList) {
        if (std::find(categoryID.begin(), categoryID.end(), it.m_categoryId) == categoryID.end()) {
            categoryID.push_back(it.m_categoryId);
        }
    }

    for (auto it = categoryID.begin(); it != categoryID.end(); ++it) {
        m_categoryList << createOfCategory(*it);
    }

    std::sort(m_categoryList.begin(),
              m_categoryList.end(),
              [=] (const ItemInfo &info1, const ItemInfo &info2) {
        return info1.m_categoryId < info2.m_categoryId;
    });

    emit categoryListChanged();
}

int AppsManager::appNums(const AppsListModel::AppCategory &category) const
{
    return appsInfoList(category).size();
}

void AppsManager::refreshAppAutoStartCache(const QString &type, const QString &desktpFilePath)
{
    if (type.isEmpty()) {
        APP_AUTOSTART_CACHE.clear();
        const QStringList &desktop_list = m_startManagerInter->AutostartList().value();

        for (const QString &auto_start_desktop : desktop_list) {
            const QString desktop_file_name = auto_start_desktop.split("/").last();

            if (!desktop_file_name.isEmpty())
                APP_AUTOSTART_CACHE.insert(desktop_file_name);
        }
    } else {
        const QString desktop_file_name = desktpFilePath.split("/").last();

        if (desktop_file_name.isEmpty())
            return;

        if (type == "added") {
            APP_AUTOSTART_CACHE.insert(desktop_file_name);
        } else if (type == "deleted") {
            APP_AUTOSTART_CACHE.remove(desktop_file_name);
        }

        emit dataChanged(AppsListModel::All);
    }
}

void AppsManager::onSearchTimeOut()
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_launcherInter->Search(m_searchText), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * w) {
        if (w->isError()) qDebug() << w->error();

        w->deleteLater();
    });
}

void AppsManager::refreshNotFoundIcon() {
    QPlatformTheme * const platformTheme = QGuiApplicationPrivate::platformTheme();
    const qreal ratio = qApp->devicePixelRatio();

    for (auto it = m_notExistIconMap.begin(); it != m_notExistIconMap.end();) {
        // The number of retries is 6
        if (it->second > 6) {
            it = m_notExistIconMap.erase(it);
            continue;
        }

        const std::pair<ItemInfo, int> itemPair = it->first;
        // QIcon have cache, cannot use QIcon::fromTheme or QIcon::hasThemeIcon
        QScopedPointer<QIconEngine> engine(platformTheme->createIconEngine(itemPair.first.m_iconKey));
        if (!engine.isNull() && !engine->isNull()) {
            for (auto iconIt = m_iconCache.begin(); iconIt != m_iconCache.end(); ++iconIt) {
                if (iconIt.key().first == itemPair.first.m_iconKey && iconIt.key().second / ratio == itemPair.second) {
                    const QPair<QString, int> iconPair{ itemPair.first.m_iconKey, iconIt.key().second };
                    const QPixmap &           pixmap = getThemeIcon(itemPair.first, itemPair.second);
                    m_iconCache[iconPair]            = pixmap;
                    emit itemDataChanged(itemPair.first);
                    it = m_notExistIconMap.erase(it);
                    break;
                }
            }
        }

        // Maybe map only have one data
        if (it != m_notExistIconMap.end()) {
            it->second += 1;
            ++it;
        }
    }

    if (m_notExistIconMap.size() == 0) {
        return m_iconRefreshTimer->stop();
    }
}

void AppsManager::removeAppIconCache(const ItemInfo &appInfo)
{
    for (auto item = m_iconCache.begin(); item != m_iconCache.end();) {
        if (item.key().first == appInfo.m_key) {
            item = m_iconCache.erase(item);
        } else {
            ++item;
        }
    }
}

void AppsManager::updateItemInfo(const ItemInfo &appInfo, ItemInfoList &itemList)
{
    for (ItemInfo& item : itemList) {
        if (item == appInfo) {
            item.updateInfo(appInfo);
            return;
        }
    }
}

void AppsManager::onIconThemeChanged()
{
    m_iconCache.clear();

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

void AppsManager::handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber)
{
    qDebug() << operation << appInfo << categoryNumber;

    if (operation == "created") {
        ItemInfo info = appInfo;
        m_allAppInfoList.append(info);
        m_usedSortedList.append(info);
        m_userSortedList.append(info);
    } else if (operation == "deleted") {
        m_allAppInfoList.removeOne(appInfo);
        m_usedSortedList.removeOne(appInfo);
        m_userSortedList.removeOne(appInfo);
    } else if (operation == "updated") {
        removeAppIconCache(appInfo);
        updateItemInfo(appInfo, m_allAppInfoList);
        updateItemInfo(appInfo, m_userSortedList);
        updateItemInfo(appInfo, m_usedSortedList);
        updateItemInfo(appInfo, m_stashList);
        for (auto& itemList : m_appInfos) {
            updateItemInfo(appInfo, itemList);
        }
    }

    m_delayRefreshTimer->start();
}
