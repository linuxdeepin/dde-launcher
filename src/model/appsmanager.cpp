// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

DWIDGET_USE_NAMESPACE

QPointer<AppsManager> AppsManager::INSTANCE = nullptr;

QGSettings *AppsManager::m_launcherSettings = SettingsPtr("com.deepin.dde.launcher", "", nullptr);
QSet<QString> AppsManager::APP_AUTOSTART_CACHE;
QSettings AppsManager::APP_USER_SORTED_LIST("deepin", "dde-launcher-app-sorted-list", nullptr);
QSettings AppsManager::APP_USED_SORTED_LIST("deepin", "dde-launcher-app-used-sorted-list");
QSettings AppsManager::APP_CATEGORY_USED_SORTED_LIST("deepin","dde-launcher-app-category-used-sorted-list");
static constexpr int USER_SORT_UNIT_TIME = 3600; // 1 hours
const QString TrashDir = QDir::homePath() + "/.local/share/Trash";
const QString TrashDirFiles = TrashDir + "/files";
const QDir::Filters ItemsShouldCount = QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

QReadWriteLock AppsManager::m_appInfoLock;
QHash<AppsListModel::AppCategory, ItemInfoList> AppsManager::m_appInfos;
ItemInfoList AppsManager::m_usedSortedList = QList<ItemInfo>();
ItemInfoList AppsManager::m_userSortedList = QList<ItemInfo>();
ItemInfoList AppsManager::m_appSearchResultList = QList<ItemInfo>();
ItemInfoList AppsManager::m_categoryList = QList<ItemInfo>();

bool AppsManager::readJsonFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QJsonParseError jsonParser;
    map = QJsonDocument::fromJson(device.readAll(), &jsonParser).toVariant().toMap();

    return jsonParser.error == QJsonParseError::NoError;
}

bool AppsManager::writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(QVariant::fromValue(map));
    return device.write(jsonDocument.toJson()) != -1;
}

void AppsManager::registerSettingsFormat()
{
    const QSettings::Format JsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    m_userSortedSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-sorted-list");
    m_usedSortedSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-used-sorted-list");
    m_categroySortedSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-category-used-sorted-list");
}

QSettings::SettingsMap AppsManager::getCacheMapData(const ItemInfoList &list)
{
    auto fillMapData = [ & ](QSettings::SettingsMap &map, const ItemInfo &info) {
        map.insert("desktop", info.m_desktop);
        map.insert("appName", info.m_name);
        map.insert("appKey", info.m_key);
        map.insert("iconKey", info.m_iconKey);
        map.insert("categoryId", info.m_categoryId);
        map.insert("installTime", info.m_installedTime);
        map.insert("openCount", info.m_openCount);
        map.insert("firstRunTime", info.m_firstRunTime);
    };

    QSettings::SettingsMap map;
    for (int i = 0; i < list.size(); i++) {
        QSettings::SettingsMap itemMap;
        const ItemInfo &info = list.at(i);
        fillMapData(itemMap, info);
        map.insert(QString("itemInfo_%1").arg(i), itemMap);
    }

    return map;
}

const ItemInfoList AppsManager::readCacheData(const QSettings::SettingsMap &map)
{
    auto getMapData = [ & ](ItemInfo &info, const QMap<QString, QVariant> &infoMap) {
        info.m_desktop = infoMap.value("desktop").toString();
        info.m_name = infoMap.value("appName").toString();
        info.m_key = infoMap.value("appKey").toString();
        info.m_iconKey = infoMap.value("iconKey").toString();
        info.m_categoryId = infoMap.value("categoryId").toLongLong();
        info.m_installedTime = infoMap.value("installTime").toInt();
        info.m_openCount = infoMap.value("openCount").toLongLong();
        info.m_firstRunTime = infoMap.value("firstRunTime").toLongLong();
    };

    ItemInfoList infoList;
    for (int i = 0; i < map.size(); i++) {
        ItemInfo info;
        const QMap<QString, QVariant> &itemInfoMap = map.value(QString("itemInfo_%1").arg(i)).toMap();

        getMapData(info, itemInfoMap);
        infoList.append(info);
    }

    return infoList;
}

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this)),
    m_startManagerInter(new DBusStartManager(this)),
    m_dockInter(new DBusDock(this)),
    m_calUtil(CalculateUtil::instance()),
    m_searchTimer(new QTimer(this)),
    m_delayRefreshTimer(new QTimer(this)),
    m_RefreshCalendarIconTimer(new QTimer(this)),
    m_lastShowDate(0),
    m_tryNums(0),
    m_tryCount(0),
    m_itemInfo(ItemInfo()),
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

    // QSettings 添加Json格式支持
    registerSettingsFormat();

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

    // 分类目录图标
    m_categoryIcon.append(QString(":/icons/skin/icons/internet_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/chat_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/music_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/multimedia_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/graphics_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/game_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/office_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/reading_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/development_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/system_normal_22px.svg"));
    m_categoryIcon.append(QString(":/icons/skin/icons/others_normal_22px.svg"));

    m_iconCacheManager = IconCacheManager::instance();

    m_updateCalendarTimer->setInterval(1000);// 1s
    m_updateCalendarTimer->start();

    // 启动应用图标和应用名称缓存线程,减少系统加载应用时的开销
    if (getDConfigValue("preloadAppsIcon", true).toBool()) {
        m_iconCacheManager->moveToThread(m_iconCacheThread);
        m_iconCacheThread->start();
    } else {
        IconCacheManager::setIconLoadState(true);
    }

    // 进程启动加载小窗口主窗体显示的图标资源 & 全屏切换到小窗口，加载小窗口资源 & 图标主题变化时，加载小窗口图标资源
    connect(this, &AppsManager::loadWindowIcon, m_iconCacheManager, &IconCacheManager::loadWindowIcon, Qt::QueuedConnection);

    // 加载小窗口其他图标资源
    connect(this, &AppsManager::loadOtherIcon, m_iconCacheManager, &IconCacheManager::loadOtherIcon, Qt::QueuedConnection);

    // 启动加载当前模式，当前ratio下的资源
    connect(this, &AppsManager::loadCurRationIcon, m_iconCacheManager, &IconCacheManager::loadCurRatioIcon, Qt::QueuedConnection);

    // 显示后，加载当前模式其他ratio下的资源， 预加载全屏下其他模式下，当前ratio下的资源
    connect(this, &AppsManager::loadOtherRatioIcon, m_iconCacheManager, &IconCacheManager::loadOtherRatioIcon, Qt::QueuedConnection);

    // 图标主题变化时，加载全屏图标资源
    connect(this, &AppsManager::loadFullWindowIcon, m_iconCacheManager, &IconCacheManager::loadFullWindowIcon, Qt::QueuedConnection);

    // 应用更新, 卸载, 安装过程中图标的处理
    connect(this, &AppsManager::loadItem, m_iconCacheManager, &IconCacheManager::loadItem, Qt::QueuedConnection);

    connect(qApp, &QCoreApplication::aboutToQuit, this, &AppsManager::stopThread, Qt::QueuedConnection);
    connect(qApp, &QCoreApplication::aboutToQuit, m_iconCacheManager, &IconCacheManager::deleteLater);
    connect(m_updateCalendarTimer, &QTimer::timeout, m_iconCacheManager, &IconCacheManager::updateCanlendarIcon, Qt::QueuedConnection);

    updateTrashState();
    refreshAllList();
    refreshAppAutoStartCache();

    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(150);
    m_delayRefreshTimer->setSingleShot(true);
    m_delayRefreshTimer->setInterval(500);

    m_RefreshCalendarIconTimer->setInterval(1000);
    m_RefreshCalendarIconTimer->setSingleShot(false);

    connect(qApp, &DApplication::iconThemeChanged, this, &AppsManager::onIconThemeChanged, Qt::QueuedConnection);
    connect(m_launcherInter, &DBusLauncher::NewAppLaunched, this, &AppsManager::markLaunched);
    connect(m_launcherInter, &DBusLauncher::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_launcherInter, &DBusLauncher::UninstallFailed, [this](const QString & appKey) { restoreItem(appKey); emit dataChanged(AppsListModel::All); });
    connect(m_launcherInter, &DBusLauncher::ItemChanged, this, &AppsManager::handleItemChanged);
    connect(m_dockInter, &DBusDock::IconSizeChanged, this, &AppsManager::IconSizeChanged, Qt::QueuedConnection);
    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &AppsManager::dockGeometryChanged, Qt::QueuedConnection);
    connect(m_startManagerInter, &DBusStartManager::AutostartChanged, this, &AppsManager::refreshAppAutoStartCache);
    connect(m_delayRefreshTimer, &QTimer::timeout, this, &AppsManager::delayRefreshData);
    connect(m_searchTimer, &QTimer::timeout, this, &AppsManager::onSearchTimeOut);
    connect(m_launcherInter, &DBusLauncher::SearchDone, this, &AppsManager::searchDone);
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &AppsManager::updateTrashState, Qt::QueuedConnection);

    onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AppsManager::onThemeTypeChanged);
    connect(m_RefreshCalendarIconTimer, &QTimer::timeout, this, &AppsManager::onRefreshCalendarTimer);

    if (!m_RefreshCalendarIconTimer->isActive())
        m_RefreshCalendarIconTimer->start();
}

/**
 * @brief AppsManager::appendSearchResult 搜索到的数据写入搜索列表
 * @param appKey 应用名
 */
void AppsManager::appendSearchResult(const QString &appKey)
{
    for (const ItemInfo &info : m_allAppInfoList)
        if (info.m_key == appKey)
            return m_appSearchResultList.append(info);
}

/**
 * @brief AppsManager::sortByPresetOrder app应用按照schemas文件中的预装应用列表顺序进行排序
 * @param processList 系统所有应用软件的信息
 */
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

    QStringList preset;
    if (m_launcherSettings && m_launcherSettings->keys().contains(key))
        preset = m_launcherSettings->get(key).toStringList();

    if (m_launcherSettings && preset.isEmpty())
        preset = m_launcherSettings->get("apps-order").toStringList();

    std::sort(processList.begin(), processList.end(), [&preset](const ItemInfo & i1, const ItemInfo & i2) {
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

/**
 * @brief AppsManager::sortByInstallTimeOrder app应用按照应用安装的时间先后排序
 * @param processList 系统所有应用软件的信息
 */
void AppsManager::sortByInstallTimeOrder(ItemInfoList &processList)
{
    std::sort(processList.begin(), processList.end(), [ & ](const ItemInfo & i1, const ItemInfo & i2) {
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
            refreshUserInfoList();
            refreshCategoryUsedInfoList();

            return;
        }
    }
}

/**
 * @brief AppsManager::abandonStashedItem 卸载应用更新列表
 * @param appKey 应用的key
 */
void AppsManager::abandonStashedItem(const QString &appKey)
{
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
                //                int itemIndex = m_pageIndex[AppsListModel::All] * m_calUtil->appPageItemCount() + pos;
                if (m_calUtil->displayMode() == ALL_APPS)
                    m_usedSortedList.insert(pos, m_stashList[i]);

                if (m_calUtil->displayMode() == GROUP_BY_CATEGORY) {
                    m_appInfoLock.lockForWrite();
                    m_appInfos[m_stashList[i].category()].insert(pos, m_stashList[i]);
                    m_appInfoLock.unlock();
                }
            }

            m_allAppInfoList.append(m_stashList[i]);
            m_stashList.removeAt(i);

            generateCategoryMap();
            refreshCategoryUsedInfoList();

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
    refreshCategoryUsedInfoList();
    refreshUserInfoList();

    // 全屏分类模式/全屏自由模式都需要界面计算,小窗口直接加载
    if (!m_launcherInter->fullscreen())
        emit loadWindowIcon();
}

void AppsManager::saveUserSortedList()
{
    m_userSortedSetting->setValue("lists", getCacheMapData(m_userSortedList));
}

/**
 * @brief AppsManager::saveUsedSortedList 保存应用使用排序(时间上的先后)列表
 */
void AppsManager::saveUsedSortedList()
{
    m_usedSortedSetting->setValue("lists", getCacheMapData(m_usedSortedList));
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

void AppsManager::uninstallApp(const QString &appKey, const int displayMode)
{
    // 遍历应用列表,存在则从列表中移除
    for (const ItemInfo &info : m_allAppInfoList) {
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

    // 重置下搜索结果,触发searchdone更新
    if (displayMode != ALL_APPS)
        m_searchTimer->start();
}

void AppsManager::markLaunched(QString appKey)
{
    if (appKey.isEmpty() || !m_newInstalledAppsList.contains(appKey))
        return;

    m_newInstalledAppsList.removeOne(appKey);

    refreshUserInfoList();

    emit newInstallListChanged();
}

/**
 * @brief AppsManager::delayRefreshData 延迟刷新安装的应用列表,保存用户操作后的应用排序
 */
void AppsManager::delayRefreshData()
{
    // refresh new installed apps
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();

    generateCategoryMap();
    saveUserSortedList();

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

/**
 * @brief 模糊匹配，反向查询key是否包含list任一个元素
 *
 * @param list 关键字列表
 * @param key 要模糊匹配的关键词
 * @return true 表示匹配成功
 * @return false 表示匹配失败
 */
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
    refreshAppListIcon(themeType);
    generateCategoryMap();
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
    case AppsListModel::Custom:    return m_userSortedList;
    case AppsListModel::All:       return m_usedSortedList;
    case AppsListModel::Search:     return m_appSearchResultList;
    case AppsListModel::Category:   return m_categoryList;
    default:;
    }

    ItemInfoList itemInfoList;

    m_appInfoLock.lockForRead();
    itemInfoList.append(m_appInfos[category]);
    m_appInfoLock.unlock();

    return itemInfoList;
}

/**
 * @brief AppsManager::appsInfoListSize 获取不同窗口模式下相应分类的应用个数
 * @param category 窗口模式，小窗口默认为Custom类型，全屏窗口默认为All类型
 * 这样也就决定了两种窗口模式下独立的应用排序逻辑
 * @return
 */
int AppsManager::appsInfoListSize(const AppsListModel::AppCategory &category)
{
    switch (category) {
    case AppsListModel::Custom:    return m_userSortedList.size();
    case AppsListModel::All:       return m_usedSortedList.size();
    case AppsListModel::Search:     return m_appSearchResultList.size();
    case AppsListModel::Category:   return m_categoryList.size();
    default:;
    }

    int size = 0;

    m_appInfoLock.lockForRead();
    size = m_appInfos[category].size();
    m_appInfoLock.unlock();

    return size;
}

/**
 * @brief AppsManager::appsInfoListIndex 获取单个模式下第n个app的信息
 * @param category 分类类型
 * @param index app在列表中的索引
 * @return 返回单个应用信息
 */
const ItemInfo AppsManager::appsInfoListIndex(const AppsListModel::AppCategory &category, const int index)
{
    switch (category) {
    case AppsListModel::Custom:
        Q_ASSERT(m_userSortedList.size() > index);
        return m_userSortedList[index];
    case AppsListModel::All:
        Q_ASSERT(m_usedSortedList.size() > index);
        return m_usedSortedList[index];
    case AppsListModel::Search:
        Q_ASSERT(m_appSearchResultList.size() > index);
        return m_appSearchResultList[index];
    case AppsListModel::Category:
        Q_ASSERT(m_categoryList.size() > index);
        return m_categoryList[index];
    default:;
    }

    ItemInfo itemInfo;
    m_appInfoLock.lockForRead();

    Q_ASSERT(m_appInfos[category].size() > index);
    itemInfo = m_appInfos[category][index];

    m_appInfoLock.unlock();

    return itemInfo;
}

const ItemInfoList &AppsManager::windowedFrameItemInfoList()
{
    return m_userSortedList;
}

const ItemInfoList &AppsManager::windowedCategoryList()
{
    return m_categoryList;
}

const ItemInfoList &AppsManager::fullscreenItemInfoList()
{
    return m_usedSortedList;
}

const QHash<AppsListModel::AppCategory, ItemInfoList> &AppsManager::categoryList()
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
const QPixmap AppsManager::appIcon(const ItemInfo &info, const int size)
{
    QPixmap pix;
    const int iconSize = perfectIconSize(size);
    QPair<QString, int> tmpKey { cacheKey(info) , iconSize };

    // 当开启预加载时，所有资源只能从子线程中加载到缓存，缓存完毕才显示界面．
    // 否则，就会存在多个线程同时访问QIcon资源的情况，会出现程序崩溃．这个是QIcon明确禁止的．

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
        m_iconValid = getThemeIcon(pix, info, size, !DSysInfo::isCommunityEdition());

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

/**
 * @brief AppsManager::appName 从缓存获取app名称
 * @param info app信息
 * @param size app的长宽
 * @return app名称
 */
const QString AppsManager::appName(const ItemInfo &info, const int size)
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
        qWarning() << "data is empty, quit!!";
        qWarning() << reply.error();
        qApp->quit();
    }

    QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();

    // 如果为空，先从缓存读取一次应用列表数据
    if (m_usedSortedList.isEmpty()) {
        if (APP_USED_SORTED_LIST.contains("list")) {
            QByteArray readBuf = APP_USED_SORTED_LIST.value("list").toByteArray();
            QDataStream in(&readBuf, QIODevice::ReadOnly);
            in >> m_usedSortedList;
            foreach(auto &used , m_usedSortedList) {
                bool bContains = fuzzyMatching(filters, used.m_key);
                if (bContains)
                    m_usedSortedList.removeOne(used);
            }
        } else {
            // 读取json缓存文件
            m_usedSortedList.append(readCacheData(m_usedSortedSetting->value("lists").toMap()));
        }
    }

    const ItemInfoList &datas = reply.value();

    // 从配置文件中读取分类应用数据
    int beginCategoryIndex = int(AppsListModel::AppCategory::Internet);
    int endCategoryIndex = int(AppsListModel::AppCategory::Others);
    for (; beginCategoryIndex < endCategoryIndex; beginCategoryIndex++) {
        ItemInfoList itemInfoList;

        if (APP_CATEGORY_USED_SORTED_LIST.contains(QString("%1").arg(beginCategoryIndex))) {
            QByteArray readCategoryBuf = APP_CATEGORY_USED_SORTED_LIST.value(QString("%1").arg(beginCategoryIndex)).toByteArray();
            QDataStream categoryIn(&readCategoryBuf, QIODevice::ReadOnly);
            categoryIn >> itemInfoList;
        } else {
            // 从json读取缓存数据
            itemInfoList  << readCacheData(m_categroySortedSetting->value(QString("AppCategory_%1").arg(beginCategoryIndex)).toMap());
        }


        m_appInfoLock.lockForWrite();
        // 当缓存数据与应用商店数据有差异时，以应用商店数据为准
        foreach (auto item , itemInfoList) {
            int index = datas.indexOf(item);
            if (index != -1 && datas.at(index).category() != item.category())
                itemInfoList.removeOne(item);
        }
        m_appInfos.insert(AppsListModel::AppCategory(beginCategoryIndex), itemInfoList);
        m_appInfoLock.unlock();
    }

    m_allAppInfoList.clear();
    m_allAppInfoList.reserve(datas.size());
    for (const auto &it : datas) {
        bool bContains = fuzzyMatching(filters, it.m_key);
        if (!m_stashList.contains(it) && !bContains) {
            if (it.m_key == "dde-trash") {
                ItemInfo trashItem = it;
                trashItem.m_iconKey = m_trashIsEmpty ? "user-trash" : "user-trash-full";
                m_allAppInfoList.append(trashItem);
                continue;
            }

            m_allAppInfoList.append(it);
        }
    }

    generateCategoryMap();
}

/**
 * @brief AppsManager::refreshUsedInfoList 更新使用过的应用列表
 */
void AppsManager::refreshUsedInfoList()
{
    if (!m_usedSortedList.isEmpty()) {
        saveUsedSortedList();
        return;
    }

    if (APP_USED_SORTED_LIST.contains("list")) {
        QByteArray readBuffer;
        QDataStream in(&readBuffer, QIODevice::ReadOnly);
        in >> m_usedSortedList;
    } else {
        m_usedSortedList.append(readCacheData(m_usedSortedSetting->value("lists").toMap()));
    }

    if (m_usedSortedList.isEmpty())
        m_usedSortedList = m_allAppInfoList;

    for (QList<ItemInfo>::ConstIterator it = m_allAppInfoList.constBegin(); it != m_allAppInfoList.constEnd(); ++it) {
        if (!m_usedSortedList.contains(*it))
            m_usedSortedList.append(*it);
    }

    for (QList<ItemInfo>::iterator it = m_usedSortedList.begin(); it != m_usedSortedList.end();) {
        if (m_allAppInfoList.contains(*it)) {
            ++it;
        } else {
            it = m_usedSortedList.erase(it);
        }
    }

    updateUsedListInfo();
    saveUsedSortedList();
}

void AppsManager::refreshCategoryUsedInfoList()
{
    QHash<AppsListModel::AppCategory, ItemInfoList>::iterator categoryApps = m_appInfos.begin();
    for (; categoryApps != m_appInfos.end(); ++categoryApps) {
        int category = categoryApps.key();
        m_categroySortedSetting->setValue(QString("AppCategory_%1").arg(category), getCacheMapData(categoryApps.value()));
    }
}

/**
 * @brief AppsManager::refreshUserInfoList app安装时间排序
 */
void AppsManager::refreshUserInfoList()
{
    if (m_userSortedList.isEmpty()) {
        // first reads the config file.
        if (APP_USER_SORTED_LIST.contains("list")) {
            QByteArray readBuffer = APP_USER_SORTED_LIST.value("list").toByteArray();
            QDataStream in(&readBuffer, QIODevice::ReadOnly);
            in >> m_userSortedList;
        } else {
            // 读取缓存
            m_userSortedList.append(readCacheData(m_userSortedSetting->value("lists").toMap()));
        }

        // if data cache file is empty.
        if (m_userSortedList.isEmpty()) {
            m_userSortedList = m_allAppInfoList;
        } else {
            QSet<int> idxSet;
            // check used list isvaild
            for (QList<ItemInfo>::iterator it = m_userSortedList.begin(); it != m_userSortedList.end();) {
                int idx = m_allAppInfoList.indexOf(*it);
                if (idx >= 0 && !idxSet.contains(idx)) {

                    // 更换语言的时候更新语言
                    it->updateInfo(m_allAppInfoList.at(idx));
                    idxSet.insert(idx);

                    ++it;
                } else {
                    it = m_userSortedList.erase(it);
                }
            }

            // m_userSortedList没有的插入到后面
            for (QList<ItemInfo>::Iterator it = m_allAppInfoList.begin(); it != m_allAppInfoList.end(); ++it) {
                if (!m_userSortedList.contains(*it)) {
                    m_userSortedList.append(*it);
                }
            }
        }
    } else {
        for (QList<ItemInfo>::iterator it = m_userSortedList.begin(); it != m_userSortedList.end();) {
            int idx = m_allAppInfoList.indexOf(*it);
            if (idx >= 0 && it->m_key == "dde-trash")
                it->updateInfo(m_allAppInfoList[idx]);

            ++it;
        }
    }

    // 从启动器小屏应用列表移除被限制使用的应用
    QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();
    for (auto it = m_userSortedList.begin(); it != m_userSortedList.end(); it++) {
        if (fuzzyMatching(filters, it->m_key))
            m_userSortedList.erase(it);
    }

    const qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    std::stable_sort(m_userSortedList.begin(), m_userSortedList.end(), [ = ](const ItemInfo & a, const ItemInfo & b) {
        const bool ANewInsatll = m_newInstalledAppsList.contains(a.m_key);
        const bool BNewInsatll = m_newInstalledAppsList.contains(b.m_key);
        if (ANewInsatll || BNewInsatll) {
            if(ANewInsatll && BNewInsatll)
                return a.m_installedTime > b.m_installedTime;

            if(ANewInsatll) return true;
            if(BNewInsatll) return false;
        }

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

/**
 * @brief AppsManager::updateUsedListInfo 更新应用信息
 */
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

/**
 * @brief AppsManager::generateCategoryMap 重新生成/更新应用分类目录信息以及所有应用信息
 */
void AppsManager::generateCategoryMap()
{
    m_categoryList.clear();
    sortByPresetOrder(m_allAppInfoList);

    ItemInfoList newInstallAppList;
    for (const ItemInfo &info : m_allAppInfoList) {

        const int userIdx = m_usedSortedList.indexOf(info);
        if (userIdx == -1) {
            m_usedSortedList.append(info);
        } else {
            const int openCount = m_usedSortedList[userIdx].m_openCount;
            m_usedSortedList[userIdx].updateInfo(info);
            m_usedSortedList[userIdx].m_openCount = openCount;
        }

        const AppsListModel::AppCategory category = info.category();

        m_appInfoLock.lockForWrite();

        if (!m_appInfos.contains(category))
            m_appInfos.insert(category, ItemInfoList());

        // 将已有应用保存到 m_appInfos， 新添加应用保存到 newInstallAppList
        if (!m_newInstalledAppsList.contains(info.m_key)) {
            // 检查应用是否已经存在
            const int idx = m_appInfos[category].indexOf(info);
            if (idx == -1) {
                m_appInfos[category].append(info);
            } else {
                // 更新一下已有应用信息
                qlonglong openCount = m_appInfos[category][idx].m_openCount;
                m_appInfos[category][idx].updateInfo(info);
                m_appInfos[category][idx].m_openCount = openCount;
            }
        } else {
            newInstallAppList.append(info);
        }

        m_appInfoLock.unlock();
    }

    // 新安装的应用以安装时间先后排序(升序),并对安装的应用做分类保存或者更新该应用信息
    sortByInstallTimeOrder(newInstallAppList);

    if (!newInstallAppList.isEmpty()) {
        m_appInfoLock.lockForWrite();
        for (const ItemInfo &info : newInstallAppList) {
            if (!m_appInfos[info.category()].contains(info)) {
                m_appInfos[info.category()].append(info);
            } else {
                // 更新一下新应用信息
                const int idx = m_appInfos[info.category()].indexOf(info);
                qlonglong openCount = m_appInfos[info.category()][idx].m_openCount;
                m_appInfos[info.category()][idx].updateInfo(info);
                m_appInfos[info.category()][idx].m_openCount = openCount;
            }
        }
        m_appInfoLock.unlock();
    }

    m_appInfoLock.lockForRead();
    // 移除 m_appInfos 中已经不存在的应用
    QHash<AppsListModel::AppCategory, ItemInfoList>::iterator categoryAppsIter = m_appInfos.begin();
    for (; categoryAppsIter != m_appInfos.end(); ++categoryAppsIter) {
        ItemInfoList &item = categoryAppsIter.value();
        for (auto it(item.begin()); it != item.end();) {
            int idx = m_allAppInfoList.indexOf(*it);

            // 在全屏自由排序模式下，m_allAppInfoList 中不存在的应用，可能缓存到 m_stashList 里面了，需要查一下
            // 检查是为了不更新 m_appInfos
            if (idx == -1 && (m_calUtil->displayMode() == ALL_APPS))
                idx = m_stashList.indexOf(*it);

            if (idx == -1)
                it = item.erase(it);
            else
                it++;
        }
    }
    m_appInfoLock.unlock();

    // remove uninstalled app item
    foreach (auto it, m_usedSortedList) {
        if (!m_allAppInfoList.contains(it) || m_stashList.contains(it)) {
            m_usedSortedList.removeOne(it);
        }
    }

    // 从所有应用中获取所有分类目录类型id,存放到临时列表categoryID中
    std::list<qlonglong> categoryID;
    for (const ItemInfo &it : m_allAppInfoList) {
        if (std::find(categoryID.begin(), categoryID.end(), it.m_categoryId) == categoryID.end()) {
            categoryID.push_back(it.m_categoryId);
        }
    }

    // 根据分类目录类型id,生成分类目录图标等信息
    for (auto it = categoryID.begin(); it != categoryID.end(); ++it) {
        m_categoryList << createOfCategory(*it);
    }

    // 按照分类目录的id大小对分类目录里列表进行排序
    std::sort(m_categoryList.begin(),
              m_categoryList.end(),
              [ = ](const ItemInfo & info1, const ItemInfo & info2) {
        return info1.m_categoryId < info2.m_categoryId;
    });

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

/**
 * @brief AppsManager::onSearchTimeOut 搜索超时错误提示
 */
void AppsManager::onSearchTimeOut()
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_launcherInter->Search(m_searchText), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * w) {
        if (w->isError()) qDebug() << w->error();

        w->deleteLater();
    });
}

void AppsManager::onIconThemeChanged()
{
    static QString lastIconTheme = QString();
    if (lastIconTheme == QIcon::themeName())
        return;

    lastIconTheme = QIcon::themeName();

    IconCacheManager::resetIconData();
    if (!CalculateUtil::instance()->fullscreen()) {
        emit loadWindowIcon();
    } else {
        emit loadFullWindowIcon();
    }
}

/**
 * @brief AppsManager::searchDone 搜索完成
 * @param resultList 搜索后接口返回的结果列表
 */
void AppsManager::searchDone(const QStringList &resultList)
{
    m_appSearchResultList.clear();

    QStringList resultCopy = resultList;

    QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();

    for (const QString& result : resultCopy) {
        bool bContains = fuzzyMatching(filters, result);
        if (bContains) {
            resultCopy.removeAll(result);
        }
    }

    for (const QString &key : resultCopy)
        appendSearchResult(key);

    emit dataChanged(AppsListModel::Search);

    if (m_appSearchResultList.isEmpty())
        emit requestTips(tr("No search results"));
    else
        emit requestHideTips();
}

/**
 * @brief AppsManager::handleItemChanged 处理应用安装、卸载、更新
 * @param operation 操作类型
 * @param appInfo 操作的应用对象信息
 * @param categoryNumber 暂时没有用
 */
void AppsManager::handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber)
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
        m_userSortedList.push_front(appInfo);
    } else if (operation == "deleted") {
        m_allAppInfoList.removeOne(appInfo);
        m_usedSortedList.removeOne(appInfo);
        m_userSortedList.removeOne(appInfo);
        //一般情况是不需要的，但是类似wps这样的程序有点特殊，删除一个其它的二进制程序也删除了，需要保存列表，否则刷新的时候会刷新出齿轮的图标
        //新增和更新则无必要
        saveUsedSortedList();
        // 如果应用被成功删除，则从临时列表中删除应用
        m_stashList.removeAll(appInfo);
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

QHash<AppsListModel::AppCategory, ItemInfoList> AppsManager::getAllAppInfo()
{
    QHash<AppsListModel::AppCategory, ItemInfoList> appInfoList;
    m_appInfoLock.lockForRead();
    appInfoList = m_appInfos;
    m_appInfoLock.unlock();

    return appInfoList;
}

void AppsManager::refreshAppListIcon(DGuiApplicationHelper::ColorType themeType)
{
    m_categoryIcon.clear();
    if (themeType == DGuiApplicationHelper::DarkType) {
        m_categoryIcon
                << QString(":/icons/skin/icons/category_network_dark.svg")
                << QString(":/icons/skin/icons/category_chat_dark.svg")
                << QString(":/icons/skin/icons/category_music_dark.svg")
                << QString(":/icons/skin/icons/category_video_dark.svg")
                << QString(":/icons/skin/icons/category_graphic_dark.svg")
                << QString(":/icons/skin/icons/category_game_dark.svg")
                << QString(":/icons/skin/icons/category_office_dark.svg")
                << QString(":/icons/skin/icons/category_reading_dark.svg")
                << QString(":/icons/skin/icons/category_develop_dark.svg")
                << QString(":/icons/skin/icons/category_system_dark.svg")
                << QString(":/icons/skin/icons/category_others_dark.svg");
    } else {
        m_categoryIcon
                << QString(":/icons/skin/icons/category_network.svg")
                << QString(":/icons/skin/icons/category_chat.svg")
                << QString(":/icons/skin/icons/category_music.svg")
                << QString(":/icons/skin/icons/category_video.svg")
                << QString(":/icons/skin/icons/category_graphic.svg")
                << QString(":/icons/skin/icons/category_game.svg")
                << QString(":/icons/skin/icons/category_office.svg")
                << QString(":/icons/skin/icons/category_reading.svg")
                << QString(":/icons/skin/icons/category_develop.svg")
                << QString(":/icons/skin/icons/category_system.svg")
                << QString(":/icons/skin/icons/category_others.svg");
    }
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

    const auto ratio = qApp->devicePixelRatio();
    for (auto *screen : qApp->screens()) {
        const QRect &sg = screen->geometry();
        const QRect &rg = QRect(sg.topLeft(), sg.size() * ratio);
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

void AppsManager::updateTrashState()
{
    int trashItemsCount = 0;
    m_fsWatcher->addPath(TrashDir);
    if (QDir(TrashDirFiles).exists()) {
        m_fsWatcher->addPath(TrashDirFiles);
        trashItemsCount = QDir(TrashDirFiles).entryList(ItemsShouldCount).count();
    }

    if (m_trashIsEmpty == !trashItemsCount)
        return;

    m_trashIsEmpty = !trashItemsCount;
    refreshAllList();

    emit dataChanged(AppsListModel::All);
}
