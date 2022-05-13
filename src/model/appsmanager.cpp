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
#include "util.h"
#include "constants.h"
#include "calculate_util.h"
#include "iconcachemanager.h"

#include <QDebug>
#include <QX11Info>
#include <QSvgRenderer>
#include <QPainter>
#include <QDataStream>
#include <QIODevice>
#include <QIcon>
#include <QScopedPointer>
#include <QIconEngine>
#include <QDate>
#include <QStandardPaths>
#include <QByteArrayList>
#include <QQueue>

#include <private/qguiapplication_p.h>
#include <private/qiconloader_p.h>
#include <qpa/qplatformtheme.h>
#include <DHiDPIHelper>
#include <DApplication>
#include "dpinyin.h"

DWIDGET_USE_NAMESPACE

QPointer<AppsManager> AppsManager::INSTANCE = nullptr;

QGSettings *AppsManager::m_launcherSettings = SettingsPtr("com.deepin.dde.launcher", "", nullptr);
QSet<QString> AppsManager::APP_AUTOSTART_CACHE;
QSettings AppsManager::APP_USED_SORTED_LIST("deepin", "dde-launcher-app-used-sorted-list");
QSettings AppsManager::APP_CATEGORY_USED_SORTED_LIST("deepin","dde-launcher-app-category-used-sorted-list");
QSettings AppsManager::APP_COLLECT_LIST("deepin", "dde-launcher-app-collect-list");
static constexpr int USER_SORT_UNIT_TIME = 3600; // 1 hours
const QString TRASH_DIR = QDir::homePath() + "/.local/share/Trash";
const QString TRASH_PATH = TRASH_DIR + "/files";
const QDir::Filters NAME_FILTERS = QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

QReadWriteLock AppsManager::m_appInfoLock;
QHash<AppsListModel::AppCategory, ItemInfoList_v1> AppsManager::m_appInfos;
ItemInfoList_v1 AppsManager::m_appCategoryInfos;
ItemInfoList_v1 AppsManager::m_appLetterModeInfos = ItemInfoList_v1();
ItemInfoList_v1 AppsManager::m_usedSortedList = ItemInfoList_v1();
ItemInfoList_v1 AppsManager::m_collectSortedList = ItemInfoList_v1();
ItemInfoList_v1 AppsManager::m_dirAppInfoList = ItemInfoList_v1();
ItemInfoList_v1 AppsManager::m_appSearchResultList = ItemInfoList_v1();
ItemInfoList_v1 AppsManager::m_categoryList = ItemInfoList_v1();

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this)),
    m_startManagerInter(new DBusStartManager(this)),
    m_dockInter(new DBusDock(this)),
    m_calUtil(CalculateUtil::instance()),
    m_delayRefreshTimer(new QTimer(this)),
    m_refreshCalendarIconTimer(new QTimer(this)),
    m_lastShowDate(0),
    m_tryNums(0),
    m_tryCount(0),
    m_itemInfo(ItemInfo_v1()),
    m_filterSetting(nullptr),
    m_iconValid(true),
    m_trashIsEmpty(false),
    m_fsWatcher(new QFileSystemWatcher(this)),
    m_iconCacheThread(new QThread(this)),
    m_updateCalendarTimer(new QTimer(this))
{
    if (QGSettings::isSchemaInstalled("com.deepin.dde.launcher")) {
        m_filterSetting = new QGSettings("com.deepin.dde.launcher", "/com/deepin/dde/launcher/");
        connect(m_filterSetting, &QGSettings::changed, this, &AppsManager::onGSettingChanged);
    }

    // 分类目录名称
    m_categoryTs.append(tr("Internet"));
    m_categoryTs.append(tr("Chat"));
    m_categoryTs.append(tr("Music"));
    m_categoryTs.append(tr("Video"));
    m_categoryTs.append(tr("Graphics"));
    m_categoryTs.append(tr("Games"));
    m_categoryTs.append(tr("Office"));
    m_categoryTs.append(tr("Reading"));
    m_categoryTs.append(tr("Development"));
    m_categoryTs.append(tr("System"));
    m_categoryTs.append(tr("Other"));

    m_iconCacheManager = IconCacheManager::instance();

    m_updateCalendarTimer->setInterval(60 * 1000);// 1min
    m_updateCalendarTimer->start();

    // 启动应用图标和应用名称缓存线程,减少系统加载应用时的开销
    if (getDConfigValue("preload-apps-icon", false).toBool()) {
        m_iconCacheManager->moveToThread(m_iconCacheThread);
        m_iconCacheThread->start();
    } else {
        IconCacheManager::setIconLoadState(true);
    }

    // 进程启动加载小窗口主窗体显示的图标资源
    connect(this, &AppsManager::startLoadIcon, m_iconCacheManager, &IconCacheManager::loadWindowIcon, Qt::QueuedConnection);

    // 加载小窗口其他图标资源
    connect(this, &AppsManager::loadOtherIcon, m_iconCacheManager, &IconCacheManager::loadOtherIcon, Qt::QueuedConnection);

    // 全屏切换到小窗口，加载小窗口资源
    connect(m_calUtil, &CalculateUtil::loadWindowIcon, m_iconCacheManager, &IconCacheManager::loadWindowIcon, Qt::QueuedConnection);

    // 启动加载当前模式，当前ratio下的资源
    connect(this, &AppsManager::loadCurRationIcon, m_iconCacheManager, &IconCacheManager::loadCurRatioIcon, Qt::QueuedConnection);

    // 显示后，加载当前模式其他ratio下的资源， 预加载全屏下其他模式下，当前ratio下的资源
    connect(this, &AppsManager::loadOtherRatioIcon, m_iconCacheManager, &IconCacheManager::loadOtherRatioIcon, Qt::QueuedConnection);

    // 全屏状态下，自由模式和分类模式来回切换，加载切换后对应模式下的其他ratio下的资源
    connect(this, &AppsManager::loadFullWindowIcon, m_iconCacheManager, static_cast<void(IconCacheManager::*)()>(&IconCacheManager::loadFullWindowIcon), Qt::QueuedConnection);

    connect(this, &AppsManager::loadItem, m_iconCacheManager, &IconCacheManager::loadItem, Qt::QueuedConnection);

    connect(qApp, &QCoreApplication::aboutToQuit, m_iconCacheManager, &IconCacheManager::deleteLater);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &AppsManager::stopThread, Qt::QueuedConnection);
    connect(m_updateCalendarTimer, &QTimer::timeout, m_iconCacheManager, &IconCacheManager::updateCanlendarIcon, Qt::QueuedConnection);

    updateTrashState();
    refreshAllList();
    refreshAppAutoStartCache();

    m_delayRefreshTimer->setSingleShot(true);
    m_delayRefreshTimer->setInterval(500);

    m_refreshCalendarIconTimer->setInterval(1000);
    m_refreshCalendarIconTimer->setSingleShot(false);

    connect(qApp, &DApplication::iconThemeChanged, this, &AppsManager::onIconThemeChanged, Qt::QueuedConnection);
    connect(m_launcherInter, &DBusLauncher::NewAppLaunched, this, &AppsManager::markLaunched);
    connect(m_launcherInter, &DBusLauncher::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_launcherInter, &DBusLauncher::UninstallFailed, [this](const QString & appKey) {
        restoreItem(appKey);
        emit dataChanged(AppsListModel::All);
    });
    connect(m_launcherInter, &DBusLauncher::ItemChanged, this, &AppsManager::handleItemChanged);
    connect(m_dockInter, &DBusDock::IconSizeChanged, this, &AppsManager::IconSizeChanged, Qt::QueuedConnection);
    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &AppsManager::dockGeometryChanged, Qt::QueuedConnection);
    connect(m_startManagerInter, &DBusStartManager::AutostartChanged, this, &AppsManager::refreshAppAutoStartCache);
    connect(m_delayRefreshTimer, &QTimer::timeout, this, &AppsManager::delayRefreshData);
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &AppsManager::updateTrashState, Qt::QueuedConnection);

    onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AppsManager::onThemeTypeChanged);
    connect(m_refreshCalendarIconTimer, &QTimer::timeout, this, &AppsManager::onRefreshCalendarTimer);

    if (!m_refreshCalendarIconTimer->isActive())
        m_refreshCalendarIconTimer->start();
}

void AppsManager::showSearchedData(const AppInfoList &list)
{
    m_appSearchResultList = ItemInfo_v1::appListToItemList(list);
}

void AppsManager::sortByLetterOrder(ItemInfoList_v1 &list)
{
    std::sort(list.begin(), list.end(), [ & ](const ItemInfo_v1 info1, const ItemInfo_v1 info2) {
        return info1.m_name < info2.m_name;
    });
}

/**
 * @brief AppsManager::appendSearchResult 搜索到的数据写入搜索列表
 * @param appKey 应用名
 */
void AppsManager::appendSearchResult(const QString &appKey)
{
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        if (info.m_key == appKey)
            return m_appSearchResultList.append(info);
    }
}

/**
 * @brief AppsManager::sortByPresetOrder app应用按照schemas文件中的预装应用列表顺序进行排序
 * @param processList 系统所有应用软件的信息
 */
void AppsManager::sortByPresetOrder(ItemInfoList_v1 &processList)
{
    const QString system_lang = QLocale::system().name();

    QString key = "appsOrder";
    for (const auto &item : system_lang.split('_')) {
        Q_ASSERT(!item.isEmpty());

        QString k = item.toLower();
        k[0] = k[0].toUpper();

        key.append(k);
    }

    QStringList preset;
    if (m_launcherSettings && m_launcherSettings->keys().contains(key))
        preset = m_launcherSettings->get(key).toStringList();

    if (m_launcherSettings && preset.isEmpty())
        preset = m_launcherSettings->get("apps-order").toStringList();

    std::sort(processList.begin(), processList.end(), [&preset](const ItemInfo_v1 & i1, const ItemInfo_v1 & i2) {
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

void AppsManager::sortByUseFrequence(ItemInfoList_v1 &processList)
{
    std::sort(processList.begin(), processList.end(), [](const ItemInfo_v1 & info1, const ItemInfo_v1 & info2) {
        // 当前系统时间-秒数
        qint64 second = QDateTime::currentMSecsSinceEpoch() / 1000;
        qreal average1 = (info1.m_openCount * 1.0 / ((second - info1.m_installedTime) / USER_SORT_UNIT_TIME));
        qreal average2 = (info2.m_openCount * 1.0 / ((second - info2.m_installedTime) / USER_SORT_UNIT_TIME));

        return average1 > average2;
    });
}

void AppsManager::filterCollectedApp(ItemInfoList_v1 &processList)
{
    ItemInfo_v1 info;
    info.m_desktop = "/usr/share/applications/dde-control-center.desktop";
    int index = processList.indexOf(info);
    if (index != -1 && !m_collectSortedList.contains(info))
        m_collectSortedList.append(processList.at(index));

    info.m_desktop = "/usr/share/applications/dde-file-manager.desktop";
    index = processList.indexOf(info);
    if (index != -1 && !m_collectSortedList.contains(info))
        m_collectSortedList.append(processList.at(index));

    info.m_desktop = "/usr/share/applications/deepin-defender.desktop";
    index = processList.indexOf(info);
    if (index != -1 && !m_collectSortedList.contains(info))
        m_collectSortedList.append(processList.at(index));

    info.m_desktop = "/usr/share/applications/deepin-mail.desktop";
    index = processList.indexOf(info);
    if (index != -1 && !m_collectSortedList.contains(info))
        m_collectSortedList.append(processList.at(index));
}

/**
 * @brief AppsManager::sortByInstallTimeOrder app应用按照应用安装的时间先后排序
 * @param processList 系统所有应用软件的信息
 */
void AppsManager::sortByInstallTimeOrder(ItemInfoList_v1 &processList)
{
    std::sort(processList.begin(), processList.end(), [ & ](const ItemInfo_v1 & i1, const ItemInfo_v1 & i2) {
        if (i1.m_installedTime == i2.m_installedTime && i1.m_installedTime != 0) {
            // If both of them don't exist in the preset list,
            // fallback to comparing their name.
            return i1.m_name < i2.m_name;
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

/** 拖拽时应用数据的缓存处理
 * @brief AppsManager::dragdropStashItem
 * @param index
 */
void AppsManager::dragdropStashItem(const QModelIndex &index)
{
    const QString key = index.data(AppsListModel::AppKeyRole).toString();

    for (int i = 0; i != m_usedSortedList.size(); ++i) {
        if (m_usedSortedList[i].m_key == key) {
            m_stashList.append(m_usedSortedList[i]);
            m_allAppInfoList.removeOne(m_usedSortedList[i]);

            generateCategoryMap();
            refreshUsedInfoList();
            saveAppCategoryInfoList();
            break;
        }
    }
}

void AppsManager::stashItem(const QModelIndex &index)
{
    const QString key = index.data(AppsListModel::AppKeyRole).toString();

    return stashItem(key);
}

/**
 * @brief AppsManager::stashItem 从所有应用列表中删除卸载的应用,并更新各个分类列表数据
 * @param appKey 应用的key
 */
void AppsManager::stashItem(const QString &appKey)
{
    for (int i(0); i != m_allAppInfoList.size(); ++i) {
        if (m_allAppInfoList[i].m_key == appKey) {
            m_stashList.append(m_allAppInfoList[i]);
            m_allAppInfoList.removeAt(i);

            generateCategoryMap();
            refreshUsedInfoList();
            saveAppCategoryInfoList();
            break;
        }
    }
}

/**
 * @brief AppsManager::abandonStashedItem 卸载应用更新列表
 * @param appKey 应用的key
 */
void AppsManager::abandonStashedItem(const QString &appKey)
{
    //qDebug() << "bana" << appKey;
    for (int i(0); i != m_stashList.size(); ++i) {
        if (m_stashList[i].m_key == appKey) {
            m_stashList.removeAt(i);
            break;
        }
    }
    //重新获取分类数据，类似wps一个appkey对应多个desktop文件的时候,有可能会导致漏掉
    refreshCategoryInfoList();

    emit dataChanged(AppsListModel::All);
}

void AppsManager::restoreItem(const QString &appKey, const int pos)
{
    for (int i(0); i != m_stashList.size(); ++i) {
        if (m_stashList[i].m_key == appKey) {
            // if pos is valid
            if (pos != -1) {
                if (m_calUtil->displayMode() == ALL_APPS)
                    m_usedSortedList.insert(pos, m_stashList[i]);
            }

            m_allAppInfoList.append(m_stashList[i]);
            m_stashList.removeAt(i);

            generateCategoryMap();
            saveAppCategoryInfoList();

            return saveUsedSortedList();
        }
    }
}

int AppsManager::dockPosition() const
{
    return m_dockInter->position();
}

QRect AppsManager::dockGeometry() const
{
    return QRect(m_dockInter->frontendRect());
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
    saveAppCategoryInfoList();

    // 全屏分类模式/全屏自由模式都需要界面计算,小窗口直接加载
    if (!m_launcherInter->fullscreen())
        emit startLoadIcon();
}

void AppsManager::saveUsedSortedList()
{
    QByteArray writeBuf;
    QDataStream out(&writeBuf, QIODevice::WriteOnly);
    out << m_usedSortedList;

    APP_USED_SORTED_LIST.setValue("lists", writeBuf);
}

void AppsManager::searchApp(const QString &keywords)
{
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

void AppsManager::uninstallApp(const QString &appKey, const int displayMode)
{
    // 遍历应用列表,存在则从列表中移除
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        if (info.m_key == appKey) {
            APP_AUTOSTART_CACHE.remove(info.m_desktop);
            break;
        }
    }
    // 从应用列表中删除该应用信息
    stashItem(appKey);

    // 向后端发起卸载请求
    m_launcherInter->RequestUninstall(appKey, false);

    // 刷新各列表的分页信息
    emit dataChanged(AppsListModel::All);
}

void AppsManager::markLaunched(QString appKey)
{
    if (appKey.isEmpty() || !m_newInstalledAppsList.contains(appKey))
        return;

    m_newInstalledAppsList.removeOne(appKey);

    emit newInstallListChanged();
}

/**
 * @brief AppsManager::delayRefreshData 延迟刷新安装的应用列表,保存用户操作后的应用排序
 */
void AppsManager::delayRefreshData()
{
    // refresh new installed apps
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();

//    generateCategoryMap();

    emit newInstallListChanged();
    emit dataChanged(AppsListModel::All);
}

/**
 * @brief AppsManager::refreshIcon 刷新当前安装/卸载/更新的应用图标
 */
void AppsManager::refreshIcon()
{
    // 更新单个应用信息
    appIcon(m_itemInfo);
}

bool AppsManager::fuzzyMatching(const QStringList& list, const QString& key)
{
    for (const QString& l : list) {
        if (key.indexOf(l, Qt::CaseInsensitive) != -1) {
            return true;
        }
    }
    return false;
}

void AppsManager::onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    Q_UNUSED(themeType);
}

void AppsManager::onRefreshCalendarTimer()
{
    m_curDate = QDate::currentDate();

    if(m_lastShowDate != m_curDate.day()) {
        delayRefreshData();
        m_lastShowDate = m_curDate.day();
    }
}

void AppsManager::onGSettingChanged(const QString &keyName)
{
    if (keyName != "filter-keys" && keyName != "filterKeys")
        return;

    refreshAllList();
}

void AppsManager::stopThread()
{
    m_iconCacheThread->quit();
    m_iconCacheThread->wait();
}

/**
 * @brief AppsManager::createOfCategory 创建分类目录信息
 * @param category 目录类型
 * @return 应用软件信息
 */
const ItemInfo_v1 AppsManager::createOfCategory(qlonglong category)
{
    ItemInfo_v1 info;
    info.m_name = m_categoryTs[static_cast<int>(category)];
    info.m_categoryId = category;
    info.m_iconKey = "";
    return info;
}

const ItemInfoList_v1 AppsManager::appsInfoList(const AppsListModel::AppCategory &category) const
{
    switch (category) {
    case AppsListModel::TitleMode:
        return m_appCategoryInfos;
    case AppsListModel::LetterMode:
        return m_appLetterModeInfos;
    case AppsListModel::All:
        return m_usedSortedList;
    case AppsListModel::Search:
    case AppsListModel::PluginSearch:
        return m_appSearchResultList;
    case AppsListModel::Collect:
        return m_collectSortedList;
    default:
        break;
    }

    ItemInfoList_v1 ItemInfoList_v1;
    ItemInfoList_v1.append(m_appInfos[category]);
    return ItemInfoList_v1;
}

int AppsManager::appsInfoListSize(const AppsListModel::AppCategory &category)
{
    switch (category) {
    case AppsListModel::TitleMode:
        return m_appCategoryInfos.size();
    case AppsListModel::LetterMode:
        return m_appLetterModeInfos.size();
    case AppsListModel::All:
        return m_usedSortedList.size();
    case AppsListModel::Search:
        return m_appSearchResultList.size();
    case AppsListModel::Collect:
        return m_collectSortedList.size();
    case AppsListModel::Dir:
        return m_dirAppInfoList.size();
    case AppsListModel::SearchFilter:
        return m_usedSortedList.size();
    case AppsListModel::PluginSearch:
        return m_appSearchResultList.size();
    default:
        break;
    }

    return m_appInfos[category].size();
}

const ItemInfo_v1 AppsManager::appsInfoListIndex(const AppsListModel::AppCategory &category, const int index)
{
    switch (category) {
    case AppsListModel::TitleMode:
        Q_ASSERT(m_appCategoryInfos.size() > index);
        return m_appCategoryInfos[index];
    case AppsListModel::LetterMode:
        Q_ASSERT(m_appLetterModeInfos.size() > index);
        return m_appLetterModeInfos[index];
    case AppsListModel::Collect:
        Q_ASSERT(m_collectSortedList.size() > index);
        return m_collectSortedList[index];
    case AppsListModel::All:
        Q_ASSERT(m_usedSortedList.size() > index);
        return m_usedSortedList[index];
    case AppsListModel::Search:
    case AppsListModel::PluginSearch:
        Q_ASSERT(m_appSearchResultList.size() > index);
        return m_appSearchResultList[index];
    case AppsListModel::SearchFilter:
        return m_usedSortedList[index];
    case AppsListModel::Dir:
        return m_dirAppInfoList[index];
    default:
        break;
    }

    ItemInfo_v1 itemInfo;
    m_appInfoLock.lockForRead();

    Q_ASSERT(m_appInfos[category].size() > index);
    itemInfo = m_appInfos[category][index];

    m_appInfoLock.unlock();

    return itemInfo;
}

const ItemInfo_v1 AppsManager::appsCategoryListIndex(const int index)
{
    if (index > m_appCategoryInfos.size() - 1)
        return ItemInfo_v1();

    return m_appCategoryInfos.at(index);
}

const ItemInfo_v1 AppsManager::appsLetterListIndex(const int index)
{
    if (index > m_appLetterModeInfos.size() - 1)
        return ItemInfo_v1();

    return m_appLetterModeInfos.at(index);
}

const ItemInfoList_v1 &AppsManager::windowedFrameItemInfoList()
{
    return m_appCategoryInfos;
}

const ItemInfoList_v1 &AppsManager::windowedCategoryList()
{
    return m_categoryList;
}

const ItemInfoList_v1 &AppsManager::fullscreenItemInfoList()
{
    return m_usedSortedList;
}

const ItemInfo_v1 AppsManager::dirAppInfo(int index)
{
    if (index >= m_dirAppInfoList.size())
        return ItemInfo_v1();

    return m_dirAppInfoList.at(index);
}

void AppsManager::setDirAppInfoList(const QModelIndex index)
{
    m_dirAppInfoList = index.data(AppsListModel::DirItemInfoRole).value<ItemInfoList_v1>();
    qInfo() << "m_dirAppInfoList size:" << m_dirAppInfoList.size();
    emit dataChanged(AppsListModel::All);
}

const QHash<AppsListModel::AppCategory, ItemInfoList_v1> &AppsManager::categoryList()
{
    return m_appInfos;
}

bool AppsManager::appIsNewInstall(const QString &key)
{
    return m_newInstalledAppsList.contains(key);
}

bool AppsManager::appIsAutoStart(const QString &desktop)
{
    int index = desktop.lastIndexOf('/');
    return APP_AUTOSTART_CACHE.contains(index > 0 ? desktop.right(desktop.size() - index - 1) : desktop);
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

/**
 * @brief AppsManager::appIcon 从缓存中获取app图片
 * @param info app信息
 * @param size app的长宽
 * @return 图片对象
 */
const QPixmap AppsManager::appIcon(const ItemInfo_v1 &info, const int size)
{
    QPixmap pix;
    const int iconSize = perfectIconSize(size);
    QPair<QString, int> tmpKey { cacheKey(info) , iconSize};

    // 开启子线程加载应用图标时
    if (m_iconCacheThread->isRunning()) {
        IconCacheManager::getPixFromCache(tmpKey, pix);
    } else {
        // 如存在，优先读取缓存
        if (IconCacheManager::existInCache(tmpKey)) {
            IconCacheManager::getPixFromCache(tmpKey, pix);
            return pix;
        }

        // 缓存中没有时，资源从主线程加载
        m_itemInfo = info;
        m_iconValid = getThemeIcon(pix, info, size, !m_iconValid);

        if (m_iconValid) {
            m_tryNums = 0;
            return pix;
        }

        // 先返回齿轮，然后继续找
        qreal ratio = qApp->devicePixelRatio();
        QIcon icon = QIcon(":/widgets/images/application-x-desktop.svg");
        pix = icon.pixmap(QSize(iconSize, iconSize) * ratio);
        pix.setDevicePixelRatio(ratio);

        if (m_tryNums < 10) {
            ++m_tryNums;

            if (!QFile::exists(info.m_iconKey))
                QIcon::setThemeSearchPaths(QIcon::themeSearchPaths());

            QTimer::singleShot(5 * 1000, this, &AppsManager::refreshIcon);
        } else {
            if (m_tryCount > 10)
                return pix;

            ++m_tryCount;
            QTimer::singleShot(10 * 1000, this, &AppsManager::refreshIcon);
        }
    }
    return pix;
}

const QString AppsManager::appName(const ItemInfo_v1 &info, const int size)
{
    const QFontMetrics fm = qApp->fontMetrics();
    const QString &fm_string = fm.elidedText(info.m_name, Qt::ElideRight, size);
    return fm_string;
}

/**
 * @brief AppsManager::refreshCategoryInfoList 更新所有应用信息
 */
void AppsManager::refreshCategoryInfoList()
{
    // 从应用商店配置文件/var/lib/lastore/applications.json获取应用数据
    QDBusPendingReply<ItemInfoList> reply = m_launcherInter->GetAllItemInfos();
    if (reply.isError()) {
        qWarning() << reply.error();
        qApp->quit();
    }

    QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();

    // 兼容历史版本, 1050以前使用list, v23即以后使用lists作为键值
    if (APP_USED_SORTED_LIST.contains("list")) {
        ItemInfoList oldUsedSortedList;
        QByteArray usedBuf = APP_USED_SORTED_LIST.value("list").toByteArray();
        QDataStream in(&usedBuf, QIODevice::ReadOnly);
        in >> oldUsedSortedList;
        m_usedSortedList = ItemInfo_v1::itemListToItem_v1List(oldUsedSortedList);
    } else {
        QByteArray usedBuf = APP_USED_SORTED_LIST.value("lists").toByteArray();
        QDataStream in(&usedBuf, QIODevice::ReadOnly);
        in >> m_usedSortedList;
    }

    QByteArray commonBuf = APP_COLLECT_LIST.value("lists").toByteArray();
    QDataStream commonData(&commonBuf, QIODevice::ReadOnly);
    commonData >> m_collectSortedList;

    qInfo() << "m_collectSortedList:" << m_collectSortedList.size();

    foreach(auto info , m_usedSortedList) {
        bool bContains = fuzzyMatching(filters, info.m_key);
        if (bContains) {
            m_usedSortedList.removeOne(info);
        }
    }

    const ItemInfoList_v1 &datas = ItemInfo_v1::itemListToItem_v1List(reply.value());

    // 从配置文件中读取分类应用数据
    int startIndex = AppsListModel::Internet;
    int endIndex = AppsListModel::Others;
    for (; startIndex < endIndex; startIndex++) {
        ItemInfoList itemInfoList;
        ItemInfoList_v1 itemInfoList_v1;

        if (APP_CATEGORY_USED_SORTED_LIST.contains(QString("%1").arg(startIndex))) {
            QByteArray categoryBuf = APP_CATEGORY_USED_SORTED_LIST.value(QString("%1").arg(startIndex)).toByteArray();
            QDataStream categoryIn(&categoryBuf, QIODevice::ReadOnly);
            categoryIn >> itemInfoList;
            itemInfoList_v1 = ItemInfo_v1::itemListToItem_v1List(itemInfoList);
        }

        // 当缓存数据与应用商店数据有差异时，以应用商店数据为准
        foreach (auto info , itemInfoList_v1) {
            int index = datas.indexOf(info);
            if (index != -1 && datas.at(index).category() != info.category())
                itemInfoList_v1.removeOne(info);
        }
        m_appInfos.insert(AppsListModel::AppCategory(startIndex), itemInfoList_v1);
    }

    m_allAppInfoList.clear();
    m_allAppInfoList.reserve(datas.size());
    for (const auto &info : datas) {
        bool bContains = fuzzyMatching(filters, info.m_key);
        if (!m_stashList.contains(info) && !bContains) {
            if (info.m_key == "dde-trash") {
                ItemInfo_v1 trashItem = info;
                trashItem.m_iconKey = m_trashIsEmpty ? "user-trash" : "user-trash-full";
                m_allAppInfoList.append(trashItem);
                continue;
            }

            m_allAppInfoList.append(info);
        }
    }

    generateCategoryMap();
}

void AppsManager::refreshUsedInfoList()
{
    if (!m_usedSortedList.isEmpty())
        return;

    QByteArray readBuffer = APP_USED_SORTED_LIST.value("lists").toByteArray();
    QDataStream in(&readBuffer, QIODevice::ReadOnly);
    in >> m_usedSortedList;

    if (m_usedSortedList.isEmpty()) {
        m_usedSortedList = m_allAppInfoList;
    }

    ItemInfoList_v1::ConstIterator allItemItor = m_allAppInfoList.constBegin();
    for (; allItemItor != m_allAppInfoList.constEnd(); ++allItemItor) {
        if (!m_usedSortedList.contains(*allItemItor)) {
            m_usedSortedList.append(*allItemItor);
        }
    }

    ItemInfoList_v1::iterator usedItemItor = m_usedSortedList.begin();
    for (; usedItemItor != m_usedSortedList.end();) {
        if (m_allAppInfoList.contains(*usedItemItor)) {
            ++usedItemItor;
        } else {
            usedItemItor = m_usedSortedList.erase(usedItemItor);
        }
    }

    updateUsedListInfo();
    saveUsedSortedList();
}

void AppsManager::saveAppCategoryInfoList()
{
    // 保存排序信息
    QHash<AppsListModel::AppCategory, ItemInfoList_v1>::iterator categoryAppsIter = m_appInfos.begin();
    for (; categoryAppsIter != m_appInfos.end(); ++categoryAppsIter) {
        int category = categoryAppsIter.key();

        QByteArray writeBuf;
        QDataStream out(&writeBuf, QIODevice::WriteOnly);
        out << categoryAppsIter.value();
        APP_CATEGORY_USED_SORTED_LIST.setValue(QString("lists_%1").arg(category), writeBuf);
    }
}

void AppsManager::updateUsedListInfo()
{
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        const int index = m_usedSortedList.indexOf(info);

        if (index == -1)
            continue;

        m_usedSortedList[index].updateInfo(info);
    }
}

void AppsManager::generateCategoryMap()
{
    m_categoryList.clear();
    sortByPresetOrder(m_allAppInfoList);

    ItemInfoList_v1 newInstallAppList;
    ItemInfoList_v1 dirAppInfoList;
    for (const ItemInfo_v1 &itemInfo : m_usedSortedList) {
        if (itemInfo.m_isDir) {
            // 应用文件夹中也没有时就加入
            dirAppInfoList.append(itemInfo.m_appInfoList);
        }
    }

    // 分类数据整理, 新安装应用整理
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        const int userIdx = m_usedSortedList.indexOf(info);
        const int dirIdx = dirAppInfoList.indexOf(info);

        // 只要文件夹中有就不处理
        if (dirIdx != -1)
            continue;

        if (userIdx == -1) {
            m_usedSortedList.append(info);
        } else {
            m_usedSortedList[userIdx].updateInfo(info);
        }

        const AppsListModel::AppCategory category = info.category();

        if (!m_appInfos.contains(category))
            m_appInfos.insert(category, ItemInfoList_v1());

        if (!m_newInstalledAppsList.contains(info.m_key)) {
            const int idx = m_appInfos[category].indexOf(info);
            if (idx == -1) {
                m_appInfos[category].append(info);
            } else {
                m_appInfos[category][idx].updateInfo(info);
            }
        } else {
            newInstallAppList.append(info);
        }
    }

    sortByInstallTimeOrder(newInstallAppList);

    // TODO: 这段代码从逻辑上，没有意义。。。暂且搁置。2022-04-13。SWT
    if (!newInstallAppList.isEmpty()) {
        for (const ItemInfo_v1 &info : newInstallAppList) {
            if (!m_appInfos[info.category()].contains(info)) {
                m_appInfos[info.category()].append(info);
            } else {
                const int idx = m_appInfos[info.category()].indexOf(info);
                m_appInfos[info.category()][idx].updateInfo(info);
            }
        }
    }

    // 移除 m_appInfos 中已经不存在的应用
    QHash<AppsListModel::AppCategory, ItemInfoList_v1>::iterator categoryAppsIter = m_appInfos.begin();
    for (; categoryAppsIter != m_appInfos.end(); ++categoryAppsIter) {
        ItemInfoList_v1 &item = categoryAppsIter.value();
        for (auto it(item.begin()); it != item.end();) {
            int idx = m_allAppInfoList.indexOf(*it);
            if (idx == -1)
                it = item.erase(it);
            else
                it++;
        }
    }

    for (auto it(m_usedSortedList.begin()); it != m_usedSortedList.end();) {
        int idx = m_allAppInfoList.indexOf(*it);

        if (idx == -1) {
            it = m_usedSortedList.erase(it);
            break;
        } else {
            ++it;
        }
    }

    // 从所有应用中获取所有分类目录类型id,存放到临时列表categoryID中
    QList<qlonglong> categoryID;
    for (const ItemInfo_v1 &it : m_allAppInfoList) {
        if (!categoryID.contains(it.m_categoryId)) {
            categoryID.append(it.m_categoryId);
        }
    }

    // 生成分类标题、图标等信息
    for (auto it = categoryID.begin(); it != categoryID.end(); ++it) {
        m_categoryList << createOfCategory(*it);
    }

    // 按照分类目录的id大小对分类目录里列表进行排序
    std::sort(m_categoryList.begin(),
              m_categoryList.end(),
              [ = ](const ItemInfo_v1 & info1, const ItemInfo_v1 & info2) {
        return info1.m_categoryId < info2.m_categoryId;
    });

    // 获取小窗口应用分类数据.
    m_appCategoryInfos.clear();
    for (int i = 0; i < m_categoryList.size(); i++) {
        // 获取实际的分类categoryId
        int categoryId = static_cast<int>(m_categoryList.at(i).m_categoryId + AppsListModel::Internet);
        if (m_appInfos.value(AppsListModel::AppCategory(categoryId)).size() <= 0)
            continue;

        m_appCategoryInfos.append(m_categoryList.at(i));
        m_appCategoryInfos.append(m_appInfos.value(AppsListModel::AppCategory(categoryId)));
    }

    // 字母排序
    ItemInfoList_v1 letterSortList = m_allAppInfoList;
    // 先分类， 再按照字母标题分组
    sortByLetterOrder(letterSortList);

    // 字母标题生成器
    QChar titleArray[27] = { 0 };
    titleArray[0] = '#';
    for (int i = 0; i < 26; i++) {
        titleArray[i + 1] = QChar(65 + i);
    }

    ItemInfoList_v1 lettergroupList;
    // 字母排序后的分组
    for (int i = 0; i < 27; i++) {
        QChar titleChar = titleArray[i];
        ItemInfo_v1 titleInfo;
        titleInfo.m_name = titleChar;
        titleInfo.m_desktop = titleChar;

        for (int j = 0; j < letterSortList.size(); j++) {
            const ItemInfo_v1 info = letterSortList.at(j);
            // 同步接口，加延时等待处理
            QString pinYinStr = Chinese2Pinyin(info.m_name);
            QThread::usleep(10);
            QChar firstKey = pinYinStr.front();
            if (firstKey.isNumber()) {
                if (lettergroupList.indexOf(titleInfo) == -1)
                    lettergroupList.append(titleInfo);

                lettergroupList.append(info);
            } else if (pinYinStr.startsWith(titleChar, Qt::CaseInsensitive)) {
                if (lettergroupList.indexOf(titleInfo) == -1) {
                    lettergroupList.append(titleInfo);
                }

                lettergroupList.append(info);
            }
        }
    }
    m_appLetterModeInfos.append(lettergroupList);

    // 获取小窗口默认收藏应用列表
    ItemInfoList_v1 tempData = m_allAppInfoList;
    m_collectSortedList.clear();
    filterCollectedApp(tempData);

    // 更新各个分类下应用的数量
    emit categoryListChanged();
}

int AppsManager::appNums(const AppsListModel::AppCategory &category) const
{
    return appsInfoListSize(category);
}

/**
 * @brief AppsManager::refreshAppAutoStartCache 刷新自启动应用集
 * @param type 操作类型
 * @param desktpFilePath 应用路径
 */
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

void AppsManager::onIconThemeChanged()
{
    static QString lastIconTheme = QString();
    if (lastIconTheme == QIcon::themeName())
        return;

    lastIconTheme = QIcon::themeName();

    IconCacheManager::resetIconData();
    if (!CalculateUtil::instance()->fullscreen()) {
        emit startLoadIcon();
    } else {
        emit loadFullWindowIcon();
    }
}

/**
 * @brief AppsManager::handleItemChanged 处理应用安装、卸载、更新
 * @param operation 操作类型
 * @param appInfo 操作的应用对象信息
 * @param categoryNumber 暂时没有用
 */
void AppsManager::handleItemChanged(const QString &operation, const ItemInfo_v1 &appInfo, qlonglong categoryNumber)
{
    Q_UNUSED(categoryNumber);

    //　更新应用到缓存
    emit loadItem(appInfo, operation);
    if (operation == "created") {

        QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();

        if (fuzzyMatching(filters, appInfo.m_key))
            return;

        m_allAppInfoList.append(appInfo);
        m_usedSortedList.append(appInfo);
    } else if (operation == "deleted") {
        m_allAppInfoList.removeOne(appInfo);
        m_usedSortedList.removeOne(appInfo);
        //一般情况是不需要的，但是类似wps这样的程序有点特殊，删除一个其它的二进制程序也删除了，需要保存列表，否则刷新的时候会刷新出齿轮的图标
        //新增和更新则无必要
        saveUsedSortedList();
    } else if (operation == "updated") {

        Q_ASSERT(m_allAppInfoList.contains(appInfo));

        // 更新所有应用列表
        int appIndex = m_allAppInfoList.indexOf(appInfo);
        if (appIndex != -1) {
            m_allAppInfoList[appIndex].updateInfo(appInfo);
        }

        // 更新按照最近使用顺序排序的列表
        int sortAppIndex = m_usedSortedList.indexOf(appInfo);
        if (sortAppIndex != -1) {
            m_usedSortedList[sortAppIndex].updateInfo(appInfo);
        }
    }

    m_delayRefreshTimer->start();
}

QHash<AppsListModel::AppCategory, ItemInfoList_v1> AppsManager::getAllAppInfo()
{
    QHash<AppsListModel::AppCategory, ItemInfoList_v1> appInfoList;
    m_appInfoLock.lockForRead();
    appInfoList = m_appInfos;
    m_appInfoLock.unlock();

    return appInfoList;
}

/**
 * @brief AppsManager::getPageCount 获取应用分类下列表中的item分页后的总页数
 * @param category 应用分类类型
 * @return 返回应用分类下列表中的item分页后的总页数
 */
int AppsManager::getPageCount(const AppsListModel::AppCategory category)
{
    int nSize = appsInfoListSize(category);
    int pageCount = m_calUtil->appPageItemCount(category);
    int page = nSize / pageCount;
    page = (nSize % pageCount) ? page + 1 : page;
    return page;
}

const QScreen *AppsManager::currentScreen()
{
    QRect dockRect = dockGeometry();

    for (auto *screen : qApp->screens()) {
        const QRect &sg = screen->geometry();
        const QRect &rg = QRect(sg.topLeft(), sg.size());
        if (rg.contains(dockRect.topLeft())) {
            return  screen;
        }
    }

    return qApp->primaryScreen();
}

int AppsManager::getVisibleCategoryCount()
{
    int ret = 0;

    for (int i = AppsListModel::Internet; i <= AppsListModel::Others; i++) {
        AppsListModel::AppCategory appCategory = AppsListModel::AppCategory(i);

        if (appsInfoListSize(appCategory) > 0) {
            ret++;
        }
    }

    return ret;
}

bool AppsManager::fullscreen() const
{
    return m_launcherInter->fullscreen();
}

int AppsManager::displayMode() const
{
    return m_launcherInter->displaymode();
}

qreal AppsManager::getCurRatio()
{
    return m_calUtil->getCurRatio();
}

void AppsManager::updateUsedSortData(QModelIndex dragIndex, QModelIndex dropIndex)
{
    ItemInfo_v1 dragItemInfo = dragIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    ItemInfo_v1 dropItemInfo = dropIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    bool dragItemIsDir = dragIndex.data(AppsListModel::ItemIsDirRole).toBool();
    bool dropItemIsDir = dropIndex.data(AppsListModel::ItemIsDirRole).toBool();

    // 1. 如果被拖拽的对象是文件夹, 那么文件夹及其内部的应用都需要写入到dropItem对象的数据中.
    // 2. 如果拖拽的对象不是文件夹, 那么将被拖拽应用的数据写入到dropItem对象的数据中.
    // 3. 如果拖拽的对象是文件夹, 那么拖拽的应用图标显示文件夹图标.

    auto saveAppDirData = [ & ](ItemInfoList_v1 &list) {
        int index = list.indexOf(dropItemInfo);
        if (index == -1)
            return;

        if (!dropItemIsDir)
            list[index].m_isDir = true;

        ItemInfoList_v1 itemList;
        if (!dropItemIsDir)
            itemList.append(dropItemInfo);

        if (!list[index].m_appInfoList.contains(dragItemInfo)) {
            if (!dragItemIsDir) {
                itemList.append(dragItemInfo);
            } else {
                // 确保被拖动的应用放到后面
                dragItemInfo.m_appInfoList.removeOne(dragItemInfo);
                itemList.append(dragItemInfo.m_appInfoList);
                dragItemInfo.m_appInfoList.clear();
                itemList.append(dragItemInfo);
            }

            list[index].m_appInfoList.append(itemList);
        }
    };

    if (m_calUtil->displayMode() == ALL_APPS)
        saveAppDirData(m_usedSortedList);
}

QList<QPixmap> AppsManager::getDirAppIcon(QModelIndex modelIndex)
{
    QList<QPixmap> pixmapList;
    ItemInfoList_v1 infoList;
    ItemInfo_v1 info = modelIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    int  index = m_usedSortedList.indexOf(info);

    if (index == -1) {
        // 在应用的目录中查找
        if (!info.m_isDir)
            return pixmapList;

        index = info.m_appInfoList.indexOf(info);
        if (index == -1)
            return pixmapList;

        infoList = info.m_appInfoList;
    } else {
        infoList = m_usedSortedList.at(index).m_appInfoList;
    }

    for (int i = 0; i < infoList.size(); i++) {
        ItemInfo_v1 itemInfo = infoList.at(i);
        // todo : 图标大小这里可以优化, 看最终方案
        pixmapList << appIcon(itemInfo, m_calUtil->appIconSize().width());
    }

    return pixmapList;
}

void AppsManager::updateTrashState()
{
    int trashItemsCount = 0;
    m_fsWatcher->addPath(TRASH_DIR);
    if (QDir(TRASH_PATH).exists()) {
        m_fsWatcher->addPath(TRASH_PATH);
        trashItemsCount = QDir(TRASH_PATH).entryList(NAME_FILTERS).count();
    }

    if (m_trashIsEmpty == !trashItemsCount)
        return;

    m_trashIsEmpty = !trashItemsCount;
    emit dataChanged(AppsListModel::All);
}
