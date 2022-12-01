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

#define AUTOSTART_KEY "autostart-desktop-list"

DWIDGET_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

QPointer<AppsManager> AppsManager::INSTANCE = nullptr;

QGSettings *AppsManager::m_launcherSettings = SettingsPtr("com.deepin.dde.launcher", "", nullptr);
QSet<QString> AppsManager::APP_AUTOSTART_CACHE;
QSettings AppsManager::APP_USED_SORTED_LIST("deepin", "dde-launcher-app-used-sorted-list");
QSettings AppsManager::APP_CATEGORY_USED_SORTED_LIST("deepin","dde-launcher-app-category-used-sorted-list");
static constexpr int USER_SORT_UNIT_TIME = 3600; // 1 hours
const QString TRASH_DIR = QDir::homePath() + "/.local/share/Trash";
const QString TRASH_PATH = TRASH_DIR + "/files";
const QDir::Filters NAME_FILTERS = QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

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
    m_collectedSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-collect-list", this);
    m_categorySetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-category-used-sorted-list", this);
    m_fullscreenUsedSortSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-fullscreen-app-used-sorted-list", this);
    m_windowedUsedSortSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin", "dde-launcher-windowed-app-used-sorted-list", this);
}

AppsManager::AppsManager(QObject *parent)
    : QObject(parent)
    , m_startManagerInter(new DBusStartManager(this))
#ifdef USE_AM_API
    , m_amDbusLauncherInter(new AMDBusLauncherInter(this))
    , m_amDbusDockInter(new AMDBusDockInter(this))
#else
    , m_launcherInter(new DBusLauncher(this))
    , m_dockInter(new DBusDock(this))
#endif
    , m_calUtil(CalculateUtil::instance())
    , m_delayRefreshTimer(new QTimer(this))
    , m_refreshCalendarIconTimer(new QTimer(this))
    , m_lastShowDate(0)
    , m_tryNums(0)
    , m_tryCount(0)
    , m_itemInfo(ItemInfo_v1())
    , m_autostartDesktopListSetting(new QSettings("deepin", AUTOSTART_KEY, this))
    , m_filterSetting(nullptr)
    , m_iconValid(true)
    , m_trashIsEmpty(false)
    , m_fsWatcher(new QFileSystemWatcher(this))
    , m_updateCalendarTimer(new QTimer(this))
    , m_uninstallDlgIsShown(false)
    , m_dragMode(Other)
    , m_curCategory(AppsListModel::FullscreenAll)
    , m_pageIndex(0)
    , m_appModel(nullptr)
    , m_appView(nullptr)
    , m_dragItemInfo(ItemInfo_v1())
    , m_dropRow(0)
{
    if (QGSettings::isSchemaInstalled("com.deepin.dde.launcher")) {
        m_filterSetting = new QGSettings("com.deepin.dde.launcher", "/com/deepin/dde/launcher/");
        connect(m_filterSetting, &QGSettings::changed, this, &AppsManager::onGSettingChanged);
    }

#ifdef USE_AM_API
    qDebug() << "m_amDbusLauncherInter is valid:" << m_amDbusLauncherInter->isValid();
#endif

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

    m_updateCalendarTimer->setInterval(1000);// 1s
    m_updateCalendarTimer->start();

    updateTrashState();
    refreshAllList();

    m_delayRefreshTimer->setSingleShot(true);
    m_delayRefreshTimer->setInterval(500);

    m_refreshCalendarIconTimer->setInterval(1000);
    m_refreshCalendarIconTimer->setSingleShot(false);

#ifdef USE_AM_API
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::NewAppLaunched, this, &AppsManager::markLaunched);
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::UninstallFailed, this, &AppsManager::onUninstallFail);
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::ItemChanged, this, qOverload<const QString &, const ItemInfo_v2 &, qlonglong>(&AppsManager::handleItemChanged));

    connect(m_amDbusDockInter, &AMDBusDockInter::IconSizeChanged, this, &AppsManager::IconSizeChanged, Qt::QueuedConnection);
    connect(m_amDbusDockInter, &AMDBusDockInter::FrontendWindowRectChanged, this, &AppsManager::dockGeometryChanged, Qt::QueuedConnection);
#else
    connect(m_launcherInter, &DBusLauncher::NewAppLaunched, this, &AppsManager::markLaunched);
    connect(m_launcherInter, &DBusLauncher::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_launcherInter, &DBusLauncher::UninstallFailed, this, &AppsManager::onUninstallFail);
    connect(m_launcherInter, &DBusLauncher::ItemChanged, this, qOverload<const QString &, const ItemInfo &, qlonglong>(&AppsManager::handleItemChanged));
    connect(m_dockInter, &DBusDock::IconSizeChanged, this, &AppsManager::IconSizeChanged, Qt::QueuedConnection);
    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &AppsManager::dockGeometryChanged, Qt::QueuedConnection);
#endif

    // TODO：自启动/打开应用这个接口后期sprint2时再改，目前 AM 未做处理
    connect(m_startManagerInter, &DBusStartManager::AutostartChanged, this, &AppsManager::refreshAppAutoStartCache);

    connect(m_delayRefreshTimer, &QTimer::timeout, this, &AppsManager::delayRefreshData);
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &AppsManager::updateTrashState, Qt::QueuedConnection);
    connect(m_refreshCalendarIconTimer, &QTimer::timeout, this, &AppsManager::onRefreshCalendarTimer);

    if (!m_refreshCalendarIconTimer->isActive())
        m_refreshCalendarIconTimer->start();
}

void AppsManager::showSearchedData(const AppInfoList &list)
{
    m_appSearchResultList = ItemInfo_v1::appListToItemV1List(list);
}

ItemInfoList_v1 AppsManager::sortByLetterOrder(ItemInfoList_v1 &list)
{
    // 字母标题生成器
    static QList<QChar> alphabetList;
    if (alphabetList.isEmpty()) {
        alphabetList.append('#');
        for (int i = 0; i < 26; i++)
            alphabetList.append(QChar('A' + i));
    }

    ItemInfoList_v1 letterGroupList;
    // 按照字母表顺序对应用进行分组排序
    for (int i = 0; i < alphabetList.size(); i++) {
        const QChar &titleChar = alphabetList[i];
        ItemInfo_v1 titleInfo;
        titleInfo.m_name = titleChar;
        titleInfo.m_desktop = titleChar;

        ItemInfoList_v1 groupList;
        for (int j = 0; j < list.size(); j++) {
            const ItemInfo_v1 &info = list.at(j);
            // 去掉字符串中的数字(表示拼音中的声调)
            const QString pinYinStr = Chinese2Pinyin(info.m_name).remove(QRegExp("\\d"));
            if (info.startWithNum()) {
                if (!contains(groupList, titleInfo) && (!contains(letterGroupList, titleInfo)))
                    groupList.append(titleInfo);

                if (!contains(letterGroupList, info))
                    groupList.append(info);
            } else if (pinYinStr.startsWith(titleChar, Qt::CaseInsensitive)) {
                if (!contains(groupList, titleInfo))
                    groupList.append(titleInfo);

                if (!contains(letterGroupList, info))
                    groupList.append(info);
            }

            if (j == list.size() - 1) {
                // 数字不变, 在首字母相同的且都为中文的分组中比较全拼音，按升序排列
                sortByPinyinOrder(groupList);
            }
        }

        // 该字母分类下没有应用时，不加入到列表
        if (groupList.size() > 1)
            letterGroupList.append(groupList);
    }

    return letterGroupList;
}

void AppsManager::sortByPinyinOrder(ItemInfoList_v1 &processList)
{
    std::sort(processList.begin(), processList.end(), [ & ](ItemInfo_v1 &info1, ItemInfo_v1 &info2) {
        QString appPinyinName1 = Chinese2Pinyin(info1.m_name).remove(QRegExp("\\d"));
        QString appPinyinName2 = Chinese2Pinyin(info2.m_name).remove(QRegExp("\\d"));

        // 1. 若是标题则不执行交换位置;二者中只有一个以字母或者数字开头，保持从小到大的顺序, 维持通用排序的顺序不变
        // 2. 二者中的其他情况则进行比对, 如果不是从小到大顺序排列，则交换位置

        if (info1.isTitle() || info2.isTitle()
                || (info1.startWithLetter() && !info2.startWithLetter())
                || (!info1.startWithLetter() && info2.startWithLetter()))
            return false;

        return (appPinyinName1.compare(appPinyinName2, Qt::CaseSensitive) < 0);
    });
}

const ItemInfo_v1 AppsManager::getItemInfo(const QString &desktop)
{
    ItemInfo_v1 itemInfo;
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        if (info.m_desktop == desktop) {
            itemInfo = info;
            break;
        }
    }

    return itemInfo;
}

void AppsManager::dropToCollected(const ItemInfo_v1 &info, const int row)
{
    if (contains(m_favoriteSortedList, info))
        return;

    m_favoriteSortedList.insert(row, info);

    saveCollectedSortedList();
    emit dataChanged(AppsListModel::Favorite);
}

QSettings::SettingsMap AppsManager::getCacheMapData(const ItemInfoList_v1 &list)
{
    auto fillMapData = [ & ](QSettings::SettingsMap &map, const ItemInfo_v1 &info) {
        map.insert("desktop", info.m_desktop);
        map.insert("appName", info.m_name);
        map.insert("appKey", info.m_key);
        map.insert("iconKey", info.m_iconKey);
        map.insert("appStatus", info.m_status);
        map.insert("categoryId", info.m_categoryId);
        map.insert("description", info.m_description);
        map.insert("progressValue", info.m_progressValue);
        map.insert("installTime", info.m_installedTime);
        map.insert("openCount", info.m_openCount);
        map.insert("firstRunTime", info.m_firstRunTime);
        map.insert("isDir", info.m_isDir);
    };

    QSettings::SettingsMap map;
    for (int i = 0; i < list.size(); i++) {
        QSettings::SettingsMap itemMap, itemDirMap;
        const ItemInfo_v1 &info = list.at(i);
        fillMapData(itemMap, info);

        for (int j = 0; j < info.m_appInfoList.size(); j++) {
            const ItemInfo_v1 &dirInfo = info.m_appInfoList.at(j);
            fillMapData(itemDirMap, dirInfo);
            itemMap.insert(QString("appInfoList_%1").arg(j), itemDirMap);
        }
        if (info.m_isDir)
            itemMap.insert("appInfoSize", info.m_appInfoList.size());

        map.insert(QString("itemInfoList_%1").arg(i), itemMap);
    }

    return map;
}

const ItemInfoList_v1 AppsManager::readCacheData(const QSettings::SettingsMap &map)
{
    auto getMapData = [ & ](ItemInfo_v1 &info, const QMap<QString, QVariant> &infoMap) {
        info.m_desktop = infoMap.value("desktop").toString();
        info.m_name = infoMap.value("appName").toString();
        info.m_key = infoMap.value("appKey").toString();
        info.m_iconKey = infoMap.value("iconKey").toString();
        info.m_status = infoMap.value("appStatus").toInt();
        info.m_categoryId = infoMap.value("categoryId").toLongLong();
        info.m_description = infoMap.value("description").toString();
        info.m_progressValue = infoMap.value("progressValue").toInt();
        info.m_installedTime = infoMap.value("installTime").toInt();
        info.m_openCount = infoMap.value("openCount").toLongLong();
        info.m_firstRunTime = infoMap.value("firstRunTime").toLongLong();
        info.m_isDir = infoMap.value("isDir").toLongLong();
    };

    ItemInfoList_v1 infoList;
    for (int i = 0; i < map.size(); i++) {
        ItemInfo_v1 info;
        ItemInfoList_v1 appList;
        QMap<QString, QVariant> itemInfoMap = map.value(QString("itemInfoList_%1").arg(i)).toMap();

        getMapData(info, itemInfoMap);
        int appInfoSize = itemInfoMap.contains("appInfoSize") ? itemInfoMap.value("appInfoSize").toInt() : 0;

        ItemInfo_v1 appInfo;
        for (int j = 0; j < appInfoSize; j++) {
            QMap<QString, QVariant> appInfoMap = itemInfoMap.value(QString("appInfoList_%1").arg(j)).toMap();
            getMapData(appInfo, appInfoMap);
            appList.append(appInfo);
        }
        info.m_appInfoList.append(appList);
        infoList.append(info);
    }

    return infoList;
}

/**保存当前拖拽的类型
 * @brief AppsManager::setDragMode
 * @param mode 拖拽类型
 */
void AppsManager::setDragMode(const DragMode &mode)
{
    m_dragMode = mode;
}

AppsManager::DragMode AppsManager::getDragMode() const
{
    return m_dragMode;
}

void AppsManager::setRemainedLastItem(const ItemInfo_v1 &info)
{
    m_remainedLastItemInfo = info;
}

const ItemInfo_v1 AppsManager::getRemainedLastItem() const
{
    return m_remainedLastItemInfo;
}

/** 保存文件夹当前页面的所在行数
 * @brief AppsManager::setDirAppRow
 * @param row
 */
void AppsManager::setDirAppRow(const int &row)
{
    m_dirAppRow = row;
}

int AppsManager::getDirAppRow() const
{
    return m_dirAppRow;
}

/**保存文件夹当前所在页面索引
 * @brief AppsManager::setDirAppPageIndex
 * @param pageIndex
 */
void AppsManager::setDirAppPageIndex(const int &pageIndex)
{
    m_dirAppPageIndex = pageIndex;
}

int AppsManager::getDirAppPageIndex() const
{
    return m_dirAppPageIndex;
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
    const qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    std::sort(processList.begin(), processList.end(), [ = ](const ItemInfo_v1 &itemInfo1, const ItemInfo_v1 &itemInfo2) {
        const bool itemANewInstall = m_newInstalledAppsList.contains(itemInfo1.m_key);
        const bool itemBNewInstall = m_newInstalledAppsList.contains(itemInfo2.m_key);
        if (itemANewInstall || itemBNewInstall) {
            if(itemANewInstall && itemBNewInstall)
                return (itemInfo1.m_installedTime > itemInfo2.m_installedTime);

            if(itemANewInstall)
                return true;

            if(itemBNewInstall)
                return false;
        }

        const qint64 itemAFirstRunTime = itemInfo1.m_firstRunTime;
        const qint64 itemBFirstRunTime = itemInfo2.m_firstRunTime;

        // If it's past time, will be sorted by open count
        if ((itemAFirstRunTime > currentTime) || (itemBFirstRunTime > currentTime))
            return itemInfo1.m_openCount > itemInfo2.m_openCount;

        qint64 itemAHoursDiff = (currentTime - itemAFirstRunTime) / USER_SORT_UNIT_TIME + 1;
        qint64 itemBHoursDiff = (currentTime - itemBFirstRunTime) / USER_SORT_UNIT_TIME + 1;

        // Average number of starts
        return ((static_cast<double>(itemInfo1.m_openCount) / itemAHoursDiff) > (static_cast<double>(itemInfo2.m_openCount) / itemBHoursDiff));
    });
}

void AppsManager::loadDefaultFavoriteList(const ItemInfoList_v1 &processList)
{
    m_favoriteSortedList.clear();

    auto defaultFavoriteList = [ & ](const ItemInfoList_v1 &list, const QStringList &strAppKeyList) {
        for (const QString &appKey: strAppKeyList) {
            for (const ItemInfo_v1 &info : list) {
                // 优先显示玲珑应用
                if (info.m_key == appKey && info.isLingLongApp()) {
                    m_favoriteSortedList.append(info);
                    break;
                }

                if (info.m_key == appKey && !info.isLingLongApp()) {
                    m_favoriteSortedList.append(info);
                    break;
                }
            }
        }
    };

    QStringList appKeyList;
    appKeyList.append("dde-control-center");
    appKeyList.append("dde-file-manager");
    appKeyList.append("deepin-defender");
    appKeyList.append("deepin-mail");
    defaultFavoriteList(processList, appKeyList);
}

void AppsManager::sortByGeneralOrder(ItemInfoList_v1 &processList)
{
    std::sort(processList.begin(), processList.end(), [](const ItemInfo_v1 &info1, const ItemInfo_v1 &info2) {
        return info1.m_name < info2.m_name;
    });
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

void AppsManager::removeNonexistentData()
{
    // 移除 m_appInfos 中已经不存在的应用
    QHash<AppsListModel::AppCategory, ItemInfoList_v1>::iterator categoryAppsIter = m_appInfos.begin();
    for (; categoryAppsIter != m_appInfos.end(); ++categoryAppsIter) {
        ItemInfoList_v1 &item = categoryAppsIter.value();
        for (auto it(item.begin()); it != item.end();) {
            if (!contains(m_allAppInfoList, *it)) {
                it = item.erase(it);
            } else {
                // 多语言时，更新应用信息
                int index = itemIndex(m_allAppInfoList, *it);
                if (index != -1)
                    it->updateInfo(m_allAppInfoList[index]);

                ++it;
            }
        }
    }

    // 移除 m_fullscreenUsedSortedList 所有应用中不存在的应用
    ItemInfoList_v1 appListToRemove;
    for (ItemInfo_v1 &info : m_fullscreenUsedSortedList) {
        if (info.m_isDir) {
            // 从文件夹中移除不存在的应用
            for (ItemInfo_v1 &dirItem : info.m_appInfoList) {
                if (!contains(m_allAppInfoList, dirItem)) {
                    // 当文件夹只有一个应用且被卸载时，删除该文件夹信息
                    if (info.m_appInfoList.size() <= 1) {
                        appListToRemove.append(info);
                    }

                    appListToRemove.append(dirItem);
                } else {
                    // 多语言时，更新应用信息
                    int index = itemIndex(m_allAppInfoList, dirItem);
                    if (index != -1)
                        dirItem.updateInfo(m_allAppInfoList[index]);
                }
            }
        } else {
            if (!contains(m_allAppInfoList, info)) {
                appListToRemove.append(info);
            } else {
                // 多语言时，更新应用信息
                int index = itemIndex(m_allAppInfoList, info);
                if (index != -1)
                    info.updateInfo(m_allAppInfoList[index]);
            }
        }
    }

    //  移除全屏所有应用列表中, 全屏文件夹列表中不存在的应用
    for (const ItemInfo_v1 &info : appListToRemove) {
        if (contains(m_fullscreenUsedSortedList, info)) {
            m_fullscreenUsedSortedList.removeOne(info);
        } else {
            for (ItemInfo_v1 &itemInfo : m_fullscreenUsedSortedList) {
                if (itemInfo.m_isDir && !itemInfo.m_appInfoList.isEmpty() && contains(itemInfo.m_appInfoList, info)) {
                    itemInfo.m_appInfoList.removeOne(info);
                    break;
                }
            }
        }
    }

    auto removeItems = [ & ](ItemInfoList_v1 &list) {
        ItemInfoList_v1 listToRemove;
        for (const ItemInfo_v1 &info : list) {
            if (!contains(m_allAppInfoList, info))
                listToRemove.append(info);
        }

        for (const ItemInfo_v1 &info : listToRemove)
            list.removeOne(info);
    };

    // 移除 m_favoriteSortedList 收藏应用中不存在的应用
    removeItems(m_favoriteSortedList);

    // 移除 m_windowedUsedSortedList 中不存在的应用
    removeItems(m_windowedUsedSortedList);
}

/** 根据应用分类 ID 对应用分类列表进行排序
 * @brief AppsManager::getCategoryListAndSortCategoryId
 */
void AppsManager::getCategoryListAndSortCategoryId()
{
    // 获取应用分类ID列表
    QList<qlonglong> categoryID;
    for (const ItemInfo_v1 &itemInfo : m_allAppInfoList) {
        if (!categoryID.contains(itemInfo.m_categoryId)) {
            categoryID.append(itemInfo.m_categoryId);
        }
    }

    m_categoryList.clear();
    // 生成分类标题、图标等信息
    for (auto it = categoryID.begin(); it != categoryID.end(); ++it) {
        m_categoryList << createOfCategory(*it);
    }

    auto compareCategoryId = [](const ItemInfo_v1 &info1, const ItemInfo_v1 &info2) {
        return info1.m_categoryId < info2.m_categoryId;
    };

    std::sort(m_categoryList.begin(), m_categoryList.end(), compareCategoryId);
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
void AppsManager::dragdropStashItem(const QModelIndex &index, AppsListModel::AppCategory mode)
{
    const QString key = index.data(AppsListModel::AppKeyRole).toString();

    auto handleData = [ & ](ItemInfoList_v1 &list) {
        foreach (const ItemInfo_v1 &info, list) {
            if (info.m_key == key) {
                m_stashList.append(info);
                list.removeOne(info);
                break;
            }
        }
    };

    if (mode == AppsListModel::FullscreenAll) {
        handleData(m_fullscreenUsedSortedList);
    } else if (mode == AppsListModel::Dir) {
        ItemInfoList_v1 list_toRemove;
        const ItemInfo_v1 removeItemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

        if (contains(m_dirAppInfoList, removeItemInfo)) {
            for (ItemInfo_v1 &info : m_fullscreenUsedSortedList) {
                if (info.m_isDir && info.m_appInfoList == m_dirAppInfoList) {
                    m_stashList.append(removeItemInfo);
                    m_dirAppInfoList.removeOne(removeItemInfo);
                    info.m_appInfoList.removeOne(removeItemInfo);

                    // 当为空时，清除这个空的文件夹内容
                    if (info.m_appInfoList.isEmpty())
                        list_toRemove.append(info);

                    break;
                }
            }
        }

        for (const ItemInfo_v1 &info : list_toRemove)
            m_fullscreenUsedSortedList.removeOne(info);

    } else if (mode == AppsListModel::Favorite) {
        handleData(m_favoriteSortedList);
    }

    saveAppCategoryInfoList();
    saveFullscreenUsedSortedList();
}

/**保存被拖拽的应用信息
 * @brief AppsManager::setDragItem
 * @param info 被拖拽的应用信息
 */
void AppsManager::setDragItem(const ItemInfo_v1 &info)
{
    if (!info.m_desktop.isEmpty())
        m_dragItemInfo = info;
}

const ItemInfo_v1 AppsManager::getDragItem() const
{
    return m_dragItemInfo;
}

/**保存拖拽释放时，应用在视图列表中的行数
 * @brief AppsManager::setReleasePos
 * @param row
 */
void AppsManager::setReleasePos(const int &row)
{
    m_dropRow = row;
}

int AppsManager::getReleasePos() const
{
    return m_dropRow;
}

/**保存当前视图列表的模式类型
 * @brief AppsManager::setCategory
 * @param category
 */
void AppsManager::setCategory(const AppsListModel::AppCategory category)
{
    m_curCategory = category;
}

AppsListModel::AppCategory AppsManager::getCategory() const
{
    return m_curCategory;
}

/**保存当前模式下的页面索引
 * @brief AppsManager::setPageIndex
 * @param pageIndex
 */
void AppsManager::setPageIndex(const int &pageIndex)
{
    m_pageIndex = pageIndex;
}

int AppsManager::getPageIndex() const
{
    return m_pageIndex;
}

AppsListModel *AppsManager::getListModel() const
{
    return m_appModel;
}

/**多个页面间进行拖拽时，需要用到目标视图列表当前页对应的模型
 * 保存当前视图列表对应的模式对象指针
 * @brief AppsManager::setListModel
 * @param model
 */
void AppsManager::setListModel(AppsListModel *model)
{
    m_appModel = model;
}

AppGridView *AppsManager::getListView() const
{
    return m_appView;
}

void AppsManager::setDragModelIndex(const QModelIndex &index)
{
    m_dragIndex = index;
}

QModelIndex AppsManager::dragModelIndex() const
{
    return m_dragIndex;
}

/**多个页面间进行拖拽时，需要用到目标视图列表当前页对应的视图
 * 保存当前视图列表对象指针
 * @brief AppsManager::setListView
 * @param view
 */
void AppsManager::setListView(AppGridView *view)
{
    m_appView = view;
}

void AppsManager::removeDragItem()
{
    if (getDragMode() != DirOut) {
        qDebug() << "drag mode: " << getDragMode();
        return;
    }

    ItemInfo_v1 removeItemInfo = getDragItem();
#ifdef QT_DEBUG
    qDebug() << "removeItemInfo:" << removeItemInfo;
#endif

    if (!contains(m_dirAppInfoList, removeItemInfo)) {
        qDebug() << "not exist in dir";
        return;
    }

    ItemInfoList_v1 list_toRemove;
    for (ItemInfo_v1 &info : m_fullscreenUsedSortedList) {
        if (info.m_isDir && info.m_appInfoList == m_dirAppInfoList) {
            m_dirAppInfoList.removeOne(removeItemInfo);
            info.m_appInfoList.removeOne(removeItemInfo);

            // 当从文件夹展开窗口中移除应用且中只剩一个时，把这个应用放置在原文件夹的位置，并清除文件夹样式
            // 实现步骤：
            // 1. 把最后一个应用保存下来到AppManager中
            // 2. 再插入到文件夹原始页面的所在行数， 界面刷新
            // 3. 清除文件夹样式
            // 4. 数据缓存，界面刷新
            if (info.m_appInfoList.size() < 2) {
                // 1. 保存最后一个应用数据到本地, 并清空文件夹列表
                if (info.m_appInfoList.size() > 0) {
                    setRemainedLastItem(info.m_appInfoList.at(0));
                    m_dirAppInfoList.removeLast();
                    info.m_appInfoList.removeLast();
                }

                list_toRemove.append(info);
            }
#ifdef QT_DEBUG
            qDebug() << QString("remove  %1 successfully").arg(removeItemInfo.m_desktop);
#endif
            break;
        }
    }

    // 2. 将文件夹中剩余的唯一一个应用插入到点击文件夹时的初始位置
    if (m_dirAppInfoList.isEmpty()) {
        const int originDirAppRow = getDirAppRow() + getDirAppPageIndex() * m_calUtil->appPageItemCount(AppsListModel::FullscreenAll);
        const ItemInfo_v1 &info = getRemainedLastItem();
        m_fullscreenUsedSortedList.insert(originDirAppRow, info);
    }

    // 3. 清除文件夹样式
    for (const ItemInfo_v1 &info : list_toRemove)
        m_fullscreenUsedSortedList.removeOne(info);

    // 4. 数据保存， 界面刷新
    saveFullscreenUsedSortedList();
    emit dataChanged(AppsListModel::FullscreenAll);
}

void AppsManager::insertDropItem(int pos)
{
    if (getDragMode() != DirOut) {
        qDebug() << "drag mode != DirOut, cur drag mode: " << getDragMode();
        return;
    }

    ItemInfo_v1 dropItemInfo = getDragItem();
#ifdef QT_DEBUG
    qDebug() << "dropItemInfo:" << dropItemInfo;
#endif

    if (!contains(m_fullscreenUsedSortedList, dropItemInfo)) {
        m_fullscreenUsedSortedList.insert(pos, dropItemInfo);
#ifdef QT_DEBUG
        qDebug() << "insert successfully in row : " << pos;
#endif
    } else {
        qDebug() << "dropItem is in the fullscreen list, dropItem is:" << dropItemInfo.m_desktop << ", exist index:" <<
                    itemIndex(m_fullscreenUsedSortedList, dropItemInfo);
    }

    saveFullscreenUsedSortedList();
    setDragMode(Other);
    emit dataChanged(AppsListModel::FullscreenAll);
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
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        if (info.m_key == appKey) {
            m_stashList.append(info);
            m_allAppInfoList.removeOne(info);

            generateCategoryMap();
            refreshItemInfoList();
            saveAppCategoryInfoList();
            break;
        }
    }
}

/**
 * @brief AppsManager::abandonStashedItem 卸载应用更新列表
 * @param appKey 应用的key
 */
void AppsManager::abandonStashedItem(const QString &desktop)
{
    // 遍历应用列表,存在则从列表中移除
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        if (info.m_desktop == desktop) {
            APP_AUTOSTART_CACHE.remove(info.m_desktop);
            break;
        }
    }

    //重新获取分类数据，类似wps一个appkey对应多个desktop文件的时候,有可能会导致漏掉
    refreshCategoryInfoList();

    emit dataChanged(AppsListModel::FullscreenAll);
}

void AppsManager::restoreItem(const QString &desktop, AppsListModel::AppCategory mode, const int pos)
{
    if (pos == -1)
        return;

    for (const ItemInfo_v1 &info : m_stashList) {
        if (info.m_desktop == desktop) {
            switch (mode) {
            case AppsListModel::FullscreenAll:
                m_fullscreenUsedSortedList.insert(pos, info);
                break;
            case AppsListModel::Favorite:
                m_favoriteSortedList.insert(pos, info);
                break;
            case AppsListModel::Dir:
                for (ItemInfo_v1 &itemInfo : m_fullscreenUsedSortedList) {
                    if (itemInfo.m_appInfoList == m_dirAppInfoList) {
                        itemInfo.m_appInfoList.insert(pos, info);

                        // 文件夹中显示的是本地变量缓存的内容，因此这里进行添加，保持与列表中的数据一致
                        m_dirAppInfoList.insert(pos, info);
                        break;
                    }
                }
                break;
            default:
                break;
            }

            m_allAppInfoList.append(info);
            m_stashList.removeOne(info);
            saveFullscreenUsedSortedList();
            saveCollectedSortedList();
            break;
        }
    }
}

int AppsManager::dockPosition() const
{
#ifdef USE_AM_API
    return m_amDbusDockInter->position();
#else
    return m_dockInter->position();
#endif
}

QRect AppsManager::dockGeometry() const
{
#ifdef USE_AM_API
    return QRect(m_amDbusDockInter->frontendWindowRect());
#else
    return QRect(m_dockInter->frontendRect());
#endif
}

bool AppsManager::isVaild()
{
#ifdef USE_AM_API
    return m_amDbusLauncherInter->isValid() && !m_allAppInfoList.isEmpty();
#else
    return m_launcherInter->isValid() && !m_allAppInfoList.isEmpty();
#endif
}

void AppsManager::refreshAllList()
{
    refreshCategoryInfoList();
    refreshItemInfoList();
    saveAppCategoryInfoList();
    readCollectedCacheData();
}

void AppsManager::saveWidowedUsedSortedList()
{
    m_windowedUsedSortSetting->setValue("lists", getCacheMapData(m_windowedUsedSortedList));
}

void AppsManager::saveFullscreenUsedSortedList()
{
    m_fullscreenUsedSortSetting->setValue("lists", getCacheMapData(m_fullscreenUsedSortedList));
}

void AppsManager::saveCollectedSortedList()
{
    m_collectedSetting->setValue("lists", getCacheMapData(m_favoriteSortedList));
}

void AppsManager::searchApp(const QString &keywords)
{
    m_searchText = keywords;
}

void AppsManager::launchApp(const QModelIndex &index)
{
    const QString &desktop = index.data(AppsListModel::AppDesktopRole).toString();

    if (desktop.isEmpty()) {
        qWarning() << "empty desktop file path.";
        return;
    }

    m_startManagerInter->Launch(desktop);

    // 更新应用的打开次数以及首次启动的时间戳
    for (ItemInfo_v1 &info: m_allAppInfoList) {
        if (info.m_desktop == desktop) {
            ++info.m_openCount;
            if (info.m_firstRunTime == 0)
                info.m_firstRunTime = QDateTime::currentMSecsSinceEpoch() / 1000;

            break;
        }
    }

    refreshItemInfoList();
    markLaunched(index.data(AppsListModel::AppKeyRole).toString());
}

void AppsManager::uninstallApp(const QString &appKey)
{
    // 向后端发起卸载请求
#ifdef USE_AM_API
    m_amDbusLauncherInter->RequestUninstall(appKey, false);
#else
    m_launcherInter->RequestUninstall(appKey, false);
#endif

    // 刷新各列表的分页信息
    emit dataChanged(AppsListModel::FullscreenAll);
}

void AppsManager::uninstallApp(const ItemInfo_v1 &info)
{
    // 向后端发起卸载请求
#ifdef USE_AM_API
    m_amDbusLauncherInter->RequestUninstall(info.m_desktop, false);
#else
    m_launcherInter->RequestUninstall(info.m_key, false);
#endif

    // 刷新各列表的分页信息
    emit dataChanged(AppsListModel::FullscreenAll);
}

/**
 * @brief AppsManager::onEditCollected
 * @param index 应用模型索引
 * @param isInCollected true,在收藏列表中， false，不在收藏列表中
 */
void AppsManager::onEditCollected(const QModelIndex index, const bool isInCollected)
{
    if (!index.isValid())
        return;

    const ItemInfo_v1 &info = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    if (!isInCollected) {
        if (itemIndex(m_favoriteSortedList, info) == -1)
            m_favoriteSortedList.append(info);
    } else {
        if (itemIndex(m_favoriteSortedList, info) != -1)
            m_favoriteSortedList.removeOne(info);
    }

    saveCollectedSortedList();
    emit dataChanged(AppsListModel::Favorite);
}

void AppsManager::onMoveToFirstInCollected(const QModelIndex index)
{
    if (!index.isValid() || (index.row() <= 0))
        return;

    ItemInfo_v1 info = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    if (itemIndex(m_favoriteSortedList, info) != -1) {
        m_favoriteSortedList.removeOne(info);
        m_favoriteSortedList.insert(0, info);
    }

    saveCollectedSortedList();
    emit dataChanged(AppsListModel::Favorite);
}

void AppsManager::markLaunched(const QString &appKey)
{
    if (appKey.isEmpty() || !m_newInstalledAppsList.contains(appKey))
        return;

    m_newInstalledAppsList.removeOne(appKey);

    emit dataChanged(AppsListModel::FullscreenAll);
}

void AppsManager::delayRefreshData()
{
#ifdef USE_AM_API
    // TODO: 这个接口返回数据存在异常
    m_newInstalledAppsList = m_amDbusLauncherInter->GetAllNewInstalledApps().value();
#else
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();
#endif

    refreshCategoryInfoList();
    emit dataChanged(AppsListModel::FullscreenAll);
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

void AppsManager::onUninstallFail(const QString &desktop)
{
    restoreItem(desktop, AppsListModel::FullscreenAll);
    emit dataChanged(AppsListModel::FullscreenAll);
}

bool AppsManager::contains(const ItemInfoList_v1 &list, const ItemInfo_v1 &item) const
{
    bool find = false;
    for (const ItemInfo_v1 &info : list) {
        if (info.isEqual(item)) {
            find = true;
            break;
        }
    }

    return find;
}

int AppsManager::itemIndex(const ItemInfoList_v1 &list, const ItemInfo_v1 &item) const
{
    int index = -1;
    for (int i = 0; i < list.size(); i++) {
        if (list[i].isEqual(item)) {
            index = i;
            break;
        }
    }

    return index;
}

const ItemInfoList_v1 AppsManager::appsInfoList(const AppsListModel::AppCategory &category) const
{
    switch (category) {
    case AppsListModel::TitleMode:
        return m_appCategoryInfos;
    case AppsListModel::LetterMode:
        return m_appLetterModeInfos;
    case AppsListModel::WindowedAll:
        return m_windowedUsedSortedList;
    case AppsListModel::FullscreenAll:
        return m_fullscreenUsedSortedList;
    case AppsListModel::Search:
    case AppsListModel::PluginSearch:
        return m_appSearchResultList;
    case AppsListModel::Favorite:
        return m_favoriteSortedList;
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
    case AppsListModel::WindowedAll:
        return m_windowedUsedSortedList.size();
    case AppsListModel::Search:
        return m_windowedUsedSortedList.size();
    case AppsListModel::FullscreenAll:
        return m_fullscreenUsedSortedList.size();
    case AppsListModel::PluginSearch:
        return m_appSearchResultList.size();
    case AppsListModel::Favorite:
        return m_favoriteSortedList.size();
    case AppsListModel::Dir:
        return m_dirAppInfoList.size();
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
    case AppsListModel::Favorite:
        Q_ASSERT(m_favoriteSortedList.size() > index);
        return m_favoriteSortedList[index];
    case AppsListModel::WindowedAll:
        Q_ASSERT(m_windowedUsedSortedList.size() > index);
        return m_windowedUsedSortedList[index];
    case AppsListModel::Search:
        Q_ASSERT(m_windowedUsedSortedList.size() > index);
        return m_windowedUsedSortedList[index];
    case AppsListModel::FullscreenAll:
        Q_ASSERT(m_fullscreenUsedSortedList.size() > index);
        return m_fullscreenUsedSortedList[index];
    case AppsListModel::PluginSearch:
        Q_ASSERT(m_appSearchResultList.size() > index);
        return m_appSearchResultList[index];
    case AppsListModel::Dir:
        return m_dirAppInfoList[index];
    default:
        break;
    }

    ItemInfo_v1 itemInfo;

    Q_ASSERT(m_appInfos[category].size() > index);
    itemInfo = m_appInfos[category][index];

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
    return m_fullscreenUsedSortedList;
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
    emit dataChanged(AppsListModel::FullscreenAll);
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
    return getAutostartValue().contains(desktop);
}

bool AppsManager::appIsOnDock(const QString &desktop)
{
#ifdef USE_AM_API
    return m_amDbusDockInter->IsDocked(desktop);
#else
    return m_dockInter->IsDocked(desktop);
#endif
}

bool AppsManager::appIsOnDesktop(const QString &desktop)
{
#ifdef USE_AM_API
    return m_amDbusLauncherInter->IsItemOnDesktop(desktop).value();
#else
    return m_launcherInter->IsItemOnDesktop(desktop).value();
#endif
}

bool AppsManager::appIsProxy(const QString &desktop)
{
#ifdef USE_AM_API
    return m_amDbusLauncherInter->GetUseProxy(desktop).value();
#else
    return m_launcherInter->GetUseProxy(desktop).value();
#endif
}

bool AppsManager::appIsEnableScaling(const QString &desktop)
{
#ifdef USE_AM_API
    return !m_amDbusLauncherInter->GetDisableScaling(desktop);
#else
    return !m_launcherInter->GetDisableScaling(desktop);
#endif
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
    return pix;
}

const QString AppsManager::appName(const ItemInfo_v1 &info, const int size)
{
    const QFontMetrics fm = qApp->fontMetrics();
    bool showSuffix = ConfigWorker::getValue(DLauncher::SHOW_LINGLONG_SUFFIX).toBool();
    bool isLingLongApp = info.isLingLongApp();
    const QString &displayName = (showSuffix && isLingLongApp) ? (QString("%1(%2)").arg(info.m_name).arg(tr("LingLong"))) : info.m_name;
    const QString &fm_string = fm.elidedText(displayName, Qt::ElideRight, size);
    return fm_string;
}

/**
 * @brief AppsManager::refreshCategoryInfoList 更新所有应用信息
 */
void AppsManager::refreshCategoryInfoList()
{
    // 0. 从应用商店配置文件/var/lib/lastore/applications.json获取应用数据
#ifdef USE_AM_API
    QDBusPendingReply<ItemInfoList_v2> reply = m_amDbusLauncherInter->GetAllItemInfos();
#else
    QDBusPendingReply<ItemInfoList> reply = m_launcherInter->GetAllItemInfos();
#endif

    if (reply.isError()) {
        qWarning() << reply.error();
        qApp->quit();
    }

    QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();

    // 1. 从后端服务获取所有应用列表
#ifdef USE_AM_API
    const ItemInfoList_v1 &datas = ItemInfo_v1::itemV2ListToItemV1List(reply.value());
#else
    const ItemInfoList_v1 &datas = ItemInfo_v1::itemListToItemV1List(reply.value());
#endif

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

    sortByPresetOrder(m_allAppInfoList);

    // 2. 读取小窗口所有应用列表的缓存数据
    m_windowedUsedSortedList = readCacheData(m_windowedUsedSortSetting->value("lists").toMap());

    // 缓存数据中没有的, 则加入到所有应用列表中
    for (const ItemInfo_v1 &itemInfo : m_allAppInfoList) {
        if (!contains(m_windowedUsedSortedList, itemInfo)) {
            m_windowedUsedSortedList.append(itemInfo);
        } else {
            // 多语言时，更新应用信息
            int index = itemIndex(m_windowedUsedSortedList, itemInfo);
            if (index != -1)
                m_windowedUsedSortedList[index].updateInfo(itemInfo);
        }
    }

    // 3. 读取全屏下所有应用列表的缓存数据
    // 为兼容历史版本, 1050以前使用list, v23即以后使用lists作为键值
    if (APP_USED_SORTED_LIST.contains("list")) {
        ItemInfoList oldUsedSortedList;
        QByteArray usedBuf = APP_USED_SORTED_LIST.value("list").toByteArray();
        QDataStream in(&usedBuf, QIODevice::ReadOnly);
        in >> oldUsedSortedList;
        m_fullscreenUsedSortedList = ItemInfo_v1::itemListToItemV1List(oldUsedSortedList);
    } else {
        m_fullscreenUsedSortedList = readCacheData(m_fullscreenUsedSortSetting->value("lists").toMap());
    }

    for (const ItemInfo_v1 &info : m_fullscreenUsedSortedList) {
        if (fuzzyMatching(filters, info.m_key))
            m_fullscreenUsedSortedList.removeOne(info);
    }

    // 4. 从缓存中读取已分类的应用数据, 降低数据处理次数
    if (m_appInfos.isEmpty()) {
        for (int categoryIndex = AppsListModel::Internet; categoryIndex < static_cast<int>(AppsListModel::Others); categoryIndex++) {
            ItemInfoList_v1 itemInfoList_v1 = m_appInfos.value(AppsListModel::AppCategory(categoryIndex));

            // 读取1050缓存的话，需要减去(v23阶段)新增的4个枚举变量的偏移值
            int originCategoryIndex = categoryIndex - 4;
            if (APP_CATEGORY_USED_SORTED_LIST.contains(QString("%1").arg(originCategoryIndex))) {
                ItemInfoList itemInfoList;
                QByteArray categoryBuf = APP_CATEGORY_USED_SORTED_LIST.value(QString("%1").arg(originCategoryIndex)).toByteArray();
                QDataStream categoryIn(&categoryBuf, QIODevice::ReadOnly);
                categoryIn >> itemInfoList;
                itemInfoList_v1 = ItemInfo_v1::itemListToItemV1List(itemInfoList);
            } else if (m_categorySetting->contains(QString("lists_%1").arg(categoryIndex))) {
                itemInfoList_v1 = readCacheData(m_categorySetting->value(QString("lists_%1").arg(categoryIndex)).toMap());
            }

            ItemInfoList_v1 list_ToRemove;
            for (const ItemInfo_v1 &info : itemInfoList_v1) {
                int index = itemIndex(m_allAppInfoList, info);
                // 当缓存数据与应用商店数据有差异时，以应用商店数据为准
                if (index != -1 && m_allAppInfoList.at(index).category() != info.category())
                    list_ToRemove.append(info);

                // 如果应用已经卸载，从更新缓存数据列表
                if (index == -1)
                    list_ToRemove.append(info);
            }

            for (const ItemInfo_v1 &info : list_ToRemove)
                itemInfoList_v1.removeOne(info);

            m_appInfos.insert(AppsListModel::AppCategory(categoryIndex), itemInfoList_v1);

            if (categoryIndex == static_cast<int>(AppsListModel::System) && QFileInfo::exists(APP_CATEGORY_USED_SORTED_LIST.fileName())) {
                QDir removeCacheFileDir;
                removeCacheFileDir.remove(APP_CATEGORY_USED_SORTED_LIST.fileName());
                qDebug() << "remove dde-launcher-app-category-used-sorted-list conf file success!";
            }
        }
    }

    // 5. 获取新安装的应用列表
#ifdef USE_AM_API
    m_newInstalledAppsList = m_amDbusLauncherInter->GetAllNewInstalledApps().value();
#else
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();
#endif

    // 6. 清除不存在的数据
    removeNonexistentData();
    generateCategoryMap();
}

void AppsManager::refreshItemInfoList()
{
    if (m_fullscreenUsedSortedList.isEmpty())
        m_fullscreenUsedSortedList = m_allAppInfoList;

    // 更新全屏窗口-所有应用列表
    ItemInfoList_v1::ConstIterator allItemItor = m_allAppInfoList.constBegin();
    for (; allItemItor != m_allAppInfoList.constEnd(); ++allItemItor) {
        if (!contains(m_fullscreenUsedSortedList, *allItemItor)) {
            m_fullscreenUsedSortedList.append(*allItemItor);
        } else {
            // 多语言时，更新应用信息
            int index = itemIndex(m_fullscreenUsedSortedList, *allItemItor);
            if (index != -1)
                m_fullscreenUsedSortedList[index].updateInfo(*allItemItor);
        }
    }

    ItemInfoList_v1 list_toRemove;
    for (const ItemInfo_v1 &info : m_fullscreenUsedSortedList) {
        if (!contains(m_allAppInfoList, info))
            list_toRemove.append(info);
    }

    // 从全屏列表中移除不存在的应用
    for (const ItemInfo_v1 &info : list_toRemove) {
        // 当是文件夹且其列表内的所有应用本地都存在时，不予处理，避免缓存的文件夹数据被清除
        // 否则，将文件夹中本地不存在的应用删除
        if (info.m_isDir) {
            for (const ItemInfo_v1 &appInfo : info.m_appInfoList) {
                if (!contains(m_allAppInfoList, appInfo)) {
                    const int index = itemIndex(m_fullscreenUsedSortedList, info);
                    m_fullscreenUsedSortedList[index].m_appInfoList.removeOne(appInfo);
                }
            }
        } else {
            m_fullscreenUsedSortedList.removeOne(info);
        }
    }

    // 应用文件夹中的应用不显示在全屏所有应用列表中
    ItemInfoList_v1 dirAppInfoList;
    for (const ItemInfo_v1 &itemInfo : m_fullscreenUsedSortedList) {
        if (itemInfo.m_isDir)
            dirAppInfoList.append(itemInfo.m_appInfoList);
    }

    for (const ItemInfo_v1 &dirItemInfo : dirAppInfoList) {
        int index = itemIndex(m_fullscreenUsedSortedList, dirItemInfo);
        // 不移除文件夾，只移除之前代码逻辑异常留下的缓存应用
        if (index != -1 && !m_fullscreenUsedSortedList[index].m_isDir)
            m_fullscreenUsedSortedList.removeOne(dirItemInfo);
    }

    // 移除小窗口-所有应用列表中不存在的应用
    for (const ItemInfo_v1 &info : m_windowedUsedSortedList) {
        if (!contains(m_allAppInfoList, info))
            m_windowedUsedSortedList.removeOne(info);
    }

    // 移除收藏列表中不存在的应用
    for (const ItemInfo_v1 &info : m_favoriteSortedList) {
        if (!contains(m_allAppInfoList, info))
            m_favoriteSortedList.removeOne(info);
    }

    // 根据使用频率排序
    sortByUseFrequence(m_allAppInfoList);

    // 更新应用的打开次数等信息
    updateUsedListInfo();

    saveFullscreenUsedSortedList();
}

void AppsManager::saveAppCategoryInfoList()
{
    // 保存排序信息
    QHash<AppsListModel::AppCategory, ItemInfoList_v1>::iterator categoryAppsIter = m_appInfos.begin();
    for (; categoryAppsIter != m_appInfos.end(); ++categoryAppsIter) {
        int category = categoryAppsIter.key();
        m_categorySetting->setValue(QString("lists_%1").arg(category), getCacheMapData(categoryAppsIter.value()));
    }
}

void AppsManager::updateUsedListInfo()
{
    auto updateItemInfo = [ & ](ItemInfoList_v1 &list) {
        for (const ItemInfo_v1 &info : m_allAppInfoList) {
            const int index = itemIndex(list, info);

            if (index == -1)
                continue;

            list[index].updateInfo(info);
        }
    };

    updateItemInfo(m_fullscreenUsedSortedList);
    updateItemInfo(m_windowedUsedSortedList);
}

void AppsManager::generateCategoryMap()
{
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        // 梳理应用分类数据, 为后续小窗口标题模式提供数据
        const AppsListModel::AppCategory category = info.category();
        if (!m_appInfos.contains(category))
            m_appInfos.insert(category, ItemInfoList_v1());

        const int idx = itemIndex(m_appInfos[category], info);
        if (idx == -1)
            m_appInfos[category].append(info);
        else
            m_appInfos[category][idx].updateInfo(info);
    }

    getCategoryListAndSortCategoryId();
    generateTitleCategoryList();
    generateLetterCategoryList();
}

/** 生成标题分类列表
 * @brief AppsManager::generateTitleCategoryList
 */
void AppsManager::generateTitleCategoryList()
{
    m_appCategoryInfos.clear();
    for (int i = 0; i < m_categoryList.size(); i++) {
        int categoryId = static_cast<int>(m_categoryList.at(i).m_categoryId + AppsListModel::Internet);
        if (m_appInfos.value(AppsListModel::AppCategory(categoryId)).size() <= 0)
            continue;

        m_appCategoryInfos.append(m_categoryList.at(i));
        m_appCategoryInfos.append(m_appInfos.value(AppsListModel::AppCategory(categoryId)));
    }
}

/** 获取字典序分组列表
 * @brief AppsManager::generateLetterCategoryList
 */
void AppsManager::generateLetterCategoryList()
{
    // 字母排序
    ItemInfoList_v1 letterSortList = m_allAppInfoList;

    // 先按照通用规则分类
    sortByGeneralOrder(letterSortList);

    // 按照大写字母表顺序对应用列表进行分组排序
    // 对每个分组的应用列表按照字母表顺序排序
    m_appLetterModeInfos.clear();
    m_appLetterModeInfos = sortByLetterOrder(letterSortList);
}

void AppsManager::readCollectedCacheData()
{
    const QString &filePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
            + QString("/%1/dde-launcher-app-collect-list.json").arg(qApp->organizationName());

    if (!QFile::exists(filePath)) {
        // 获取小窗口默认收藏列表
        loadDefaultFavoriteList(m_allAppInfoList);

        // 缓存小窗口收藏列表
        saveCollectedSortedList();
    } else {
        m_favoriteSortedList = readCacheData(m_collectedSetting->value("lists").toMap());
    }
}

int AppsManager::appNums(const AppsListModel::AppCategory &category)
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

        // 记录插入的应用的desktop全路径，区分不同包格式的同一应用，避免都绘制出自启动的样式
        QStringList autostartList = getAutostartValue();
        if (type == "added") {
            APP_AUTOSTART_CACHE.insert(desktop_file_name);

            if (!autostartList.contains(desktpFilePath)) {
                autostartList.append(desktpFilePath);
                setAutostartValue(autostartList);
            }
        } else if (type == "deleted") {
            APP_AUTOSTART_CACHE.remove(desktop_file_name);

            if (autostartList.contains(desktpFilePath)) {
                autostartList.removeOne(desktpFilePath);
                setAutostartValue(autostartList);
            }
        }

        emit dataChanged(AppsListModel::FullscreenAll);
    }
}

void AppsManager::setAutostartValue(const QStringList &list)
{
    m_autostartDesktopListSetting->setValue(AUTOSTART_KEY, list);
}

QStringList AppsManager::getAutostartValue() const
{
    return m_autostartDesktopListSetting->value(AUTOSTART_KEY).toStringList();
}

/**
 * @brief AppsManager::handleItemChanged 处理应用安装、卸载、更新
 * @param operation 操作类型
 * @param appInfo 操作的应用对象信息
 * @param categoryNumber 暂时没有用
 */
void AppsManager::handleItemChanged(const QString &operation, const ItemInfo_v2 &appInfo, qlonglong categoryNumber)
{
    Q_UNUSED(categoryNumber);

    ItemInfo_v1 info(appInfo);

    //　更新应用到缓存
    emit loadItem(info, operation);

    if (operation == "created") {
        QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();
        if (fuzzyMatching(filters, info.m_key))
            return;

        m_allAppInfoList.append(info);
        m_fullscreenUsedSortedList.append(info);
        m_windowedUsedSortedList.insert(0, info);
    } else if (operation == "deleted") {
        m_allAppInfoList.removeOne(info);
        m_fullscreenUsedSortedList.removeOne(info);
    } else if (operation == "updated") {
        Q_ASSERT(contains(m_allAppInfoList, info));

        // 更新所有应用列表
        int index = itemIndex(m_allAppInfoList, info);
        if (index != -1)
            m_allAppInfoList[index].updateInfo(info);

        // 更新按照最近使用顺序排序的列表
        index = itemIndex(m_fullscreenUsedSortedList, info);
        if (index != -1)
            m_fullscreenUsedSortedList[index].updateInfo(info);

        // 更新按照最近使用顺序排序的列表
        index = itemIndex(m_windowedUsedSortedList, info);
        if (index != -1)
            m_windowedUsedSortedList[index].updateInfo(info);
    } else {
        qDebug() << "nonexistent condition, operation:" << operation;
        return;
    }

    saveFullscreenUsedSortedList();
    saveWidowedUsedSortedList();

    m_delayRefreshTimer->start();
}

void AppsManager::handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber)
{
    Q_UNUSED(categoryNumber);

    ItemInfo_v1 info(appInfo);
    //　更新应用到缓存
    emit loadItem(info, operation);

    if (operation == "created") {
        QStringList filters = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "filter-keys").toStringList();
        if (fuzzyMatching(filters, info.m_key))
            return;

        m_allAppInfoList.append(info);
        m_fullscreenUsedSortedList.append(info);
        m_windowedUsedSortedList.insert(0, info);
    } else if (operation == "deleted") {
        m_allAppInfoList.removeOne(info);
        m_fullscreenUsedSortedList.removeOne(info);
        m_windowedUsedSortedList.removeOne(info);
        //一般情况是不需要的，但是类似wps这样的程序有点特殊，删除一个其它的二进制程序也删除了，需要保存列表，否则刷新的时候会刷新出齿轮的图标
        //新增和更新则无必要
        saveFullscreenUsedSortedList();
        saveWidowedUsedSortedList();
    } else if (operation == "updated") {
        Q_ASSERT(contains(m_allAppInfoList, info));
        // 更新所有应用列表
        int index = itemIndex(m_allAppInfoList, info);
        if (index != -1)
            m_allAppInfoList[index].updateInfo(info);

        // 更新按照最近使用顺序排序的列表
        index = itemIndex(m_fullscreenUsedSortedList, info);
        if (index != -1)
            m_fullscreenUsedSortedList[index].updateInfo(info);

        // 更新按照最近使用顺序排序的列表
        index = itemIndex(m_windowedUsedSortedList, info);
        if (index != -1)
            m_windowedUsedSortedList[index].updateInfo(info);
    } else {
        qDebug() << "nonexistent condition, operation:" << operation;
        return;
    }

    m_delayRefreshTimer->start();
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
#ifdef USE_AM_API
    return m_amDbusLauncherInter->fullscreen();
#else
    return m_launcherInter->fullscreen();
#endif
}

int AppsManager::displayMode() const
{
#ifdef USE_AM_API
    return m_amDbusLauncherInter->displaymode();
#else
    return m_launcherInter->displaymode();
#endif
}

qreal AppsManager::getCurRatio()
{
    return m_calUtil->getCurRatio();
}

void AppsManager::uninstallApp(const QModelIndex &modelIndex)
{
    if (m_uninstallDlgIsShown)
        return;

    m_uninstallDlgIsShown = true;
    DDialog unInstallDialog;
    unInstallDialog.setAccessibleName("unInstallDialog");
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    const ItemInfo_v1 info = modelIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    const QString appKey = info.m_key;
    unInstallDialog.setTitle(QString(tr("Are you sure you want to uninstall %1 ?").arg(info.m_name)));

    QPixmap pixmap = modelIndex.data(AppsListModel::AppDialogIconRole).value<QPixmap>();
    QStringList buttons;
    buttons << tr("Cancel") << tr("Confirm");
    unInstallDialog.addButtons(buttons);
    unInstallDialog.setIcon(pixmap);

    connect(&unInstallDialog, &DDialog::buttonClicked, [&](int clickedResult) {
        // 0 means "cancel" button clicked
        if (clickedResult == 0) {
            return;
        }

        uninstallApp(info);
    });

    if (!fullscreen())
        Q_EMIT requestHideLauncher();
    else
        Q_EMIT requestHidePopup();

    unInstallDialog.exec();
    m_uninstallDlgIsShown = false;
}

bool AppsManager::uninstallDlgShownState() const
{
    return m_uninstallDlgIsShown;
}

void AppsManager::updateUsedSortData(QModelIndex dragIndex, QModelIndex dropIndex)
{
    /*
     * 1. 拖拽的对象是应用, 那么将拖拽应用的数据写入到dropItem对象的数据中,
       且应该将dropItem对象文件夹标识设置为true， 且原被拖拽的应用应从全屏列表中移除
       2. 命名规则, 拖拽对象与释放对象为同类时, 取该分类标题名称;如果不同, 则以释放对象(显示文件夹中第一位应用)的分类
       标题作为文件夹名称
    */

    auto saveAppDirData = [ & ](ItemInfoList_v1 &list) {
        ItemInfo_v1 dragItemInfo = dragIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
        ItemInfo_v1 dropItemInfo = dropIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
        bool dragItemIsDir = dragIndex.data(AppsListModel::ItemIsDirRole).toBool();
        bool dropItemIsDir = dropIndex.data(AppsListModel::ItemIsDirRole).toBool();

        int dropIndex = itemIndex(list, dropItemInfo);
        int dragIndex = itemIndex(list, dragItemInfo);
        if (dropIndex == -1 || dragIndex == -1)
            return;

        // 最初都不是应用文件夹, 当是文件夹了, 应用名称就不用变化了, 因此也无需处理
        // 伪造一个应用来作为文件夹(desktop全路径为.dir结尾，m_isDir为true), 当文件夹为空时，从全屏列表移除该伪应用
        if (!dropItemIsDir) {
            list[dropIndex].m_isDir = true;
            list[dropIndex].m_desktop += ".dir";

            // 不论拖拽对象与释放对象是否为同一类应用, 都把释放对象的分类标题作为文件夹名称
            int idIndex = static_cast<int>(dropItemInfo.m_categoryId);
            if (m_categoryTs.size() > idIndex)
                list[dropIndex].m_name = m_categoryTs[idIndex];
        }

        ItemInfoList_v1 itemList;
        // 释放的对象为应用时, 先将释放的对象写入到列表，再将拖拽的对象写入，保证被拖动的应用放在第二位
        // 释放的对象为文件夹时, 内容已经在前一次写入到了列表中, 因此无需处理
        if (!dropItemIsDir)
            itemList.append(dropItemInfo);

        if (!dragItemIsDir)
            itemList.append(dragItemInfo);

        list[dropIndex].m_appInfoList.append(itemList);

        // 被拖动的应用插入文件夹列表后，从全屏列表中移除
        list.removeAt(dragIndex);
    };

    saveAppDirData(m_fullscreenUsedSortedList);
}

void AppsManager::updateDrawerTitle(const QModelIndex &index,const QString &newTitle)
{
    if (!index.isValid())
        return;

    ItemInfo_v1 info = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    int idx = itemIndex(m_fullscreenUsedSortedList, info);
    if (idx != -1) {
        info.m_name = newTitle;
        m_fullscreenUsedSortedList.replace(idx, info);
    }
}

QList<QPixmap> AppsManager::getDirAppIcon(QModelIndex modelIndex)
{
    QList<QPixmap> pixmapList;
    ItemInfoList_v1 infoList;
    ItemInfo_v1 info = modelIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    if (contains(m_fullscreenUsedSortedList, info) && info.m_isDir)
        infoList.append(info.m_appInfoList);

    for (int i = 0; i < infoList.size(); i++) {
        ItemInfo_v1 itemInfo = infoList.at(i);
        int category = static_cast<const AppsListModel *>(modelIndex.model())->category();
        pixmapList << appIcon(itemInfo, m_calUtil->appIconSize(category).width());
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
    emit dataChanged(AppsListModel::FullscreenAll);
}
