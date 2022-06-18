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
    m_collectedSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-collect-list");
    m_categorySetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-category-used-sorted-list");
    m_usedSortSetting = new QSettings(JsonFormat, QSettings::UserScope, "deepin","dde-launcher-app-used-sorted-list");
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
    , m_filterSetting(nullptr)
    , m_iconValid(true)
    , m_trashIsEmpty(false)
    , m_fsWatcher(new QFileSystemWatcher(this))
    , m_iconCacheThread(new QThread(this))
    , m_updateCalendarTimer(new QTimer(this))
    , m_uninstallDlgIsShown(false)
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

    m_iconCacheManager = IconCacheManager::instance();

    IconCacheManager::setIconLoadState(true);

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

    m_updateCalendarTimer->setInterval(1000);// 1s
    m_updateCalendarTimer->start();

    updateTrashState();
    refreshAllList();
    refreshAppAutoStartCache();

    m_delayRefreshTimer->setSingleShot(true);
    m_delayRefreshTimer->setInterval(500);

    m_refreshCalendarIconTimer->setInterval(1000);
    m_refreshCalendarIconTimer->setSingleShot(false);
    onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());

#ifdef USE_AM_API
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::NewAppLaunched, this, &AppsManager::markLaunched);
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::UninstallFailed, [this](const QString &appKey) {
        restoreItem(appKey, AppsListModel::All);
        emit dataChanged(AppsListModel::All);
    });
    connect(m_amDbusLauncherInter, &AMDBusLauncherInter::ItemChanged, this, qOverload<const QString &, const ItemInfo_v2 &, qlonglong>(&AppsManager::handleItemChanged));

    connect(m_amDbusDockInter, &AMDBusDockInter::IconSizeChanged, this, &AppsManager::IconSizeChanged, Qt::QueuedConnection);
    connect(m_amDbusDockInter, &AMDBusDockInter::FrontendWindowRectChanged, this, &AppsManager::dockGeometryChanged, Qt::QueuedConnection);
#else
    connect(m_launcherInter, &DBusLauncher::NewAppLaunched, this, &AppsManager::markLaunched);
    connect(m_launcherInter, &DBusLauncher::UninstallSuccess, this, &AppsManager::abandonStashedItem);
    connect(m_launcherInter, &DBusLauncher::UninstallFailed, [this](const QString &appKey) {
        restoreItem(appKey, AppsListModel::All);
        emit dataChanged(AppsListModel::All);
    });
    connect(m_launcherInter, &DBusLauncher::ItemChanged, this, qOverload<const QString &, const ItemInfo &, qlonglong>(&AppsManager::handleItemChanged));
    connect(m_dockInter, &DBusDock::IconSizeChanged, this, &AppsManager::IconSizeChanged, Qt::QueuedConnection);
    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &AppsManager::dockGeometryChanged, Qt::QueuedConnection);
#endif

    // TODO：自启动/打开应用这个接口后期sprint2时再改，目前 AM 未做处理
    connect(m_startManagerInter, &DBusStartManager::AutostartChanged, this, &AppsManager::refreshAppAutoStartCache);

    connect(qApp, &DApplication::iconThemeChanged, this, &AppsManager::onIconThemeChanged, Qt::QueuedConnection);
    connect(m_delayRefreshTimer, &QTimer::timeout, this, &AppsManager::delayRefreshData);
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &AppsManager::updateTrashState, Qt::QueuedConnection);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AppsManager::onThemeTypeChanged);
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
            const QString pinYinStr = Chinese2Pinyin(info.m_name);
            QChar firstKey = pinYinStr.front();
            if (firstKey.isNumber()) {
                if (!groupList.contains(titleInfo) && (!letterGroupList.contains(titleInfo)))
                    groupList.append(titleInfo);

                if (!letterGroupList.contains(info))
                    groupList.append(info);
            } else if (pinYinStr.startsWith(titleChar, Qt::CaseInsensitive)) {
                if (!groupList.contains(titleInfo))
                    groupList.append(titleInfo);

                groupList.append(info);
            } else {
                if (j == list.size() - 1) {
                    // 数字不变, 在首字母相同的且都为中文的分组中比较全拼音，按升序排列
                    sortByPinyinOrder(groupList);
                }
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
    std::sort(processList.begin(), processList.end(), [](const ItemInfo_v1 &info1, const ItemInfo_v1 &info2) {
        QString appPinyinName1 = Chinese2Pinyin(info1.m_name);
        QString appPinyinName2 = Chinese2Pinyin(info2.m_name);

        // 1. 若是标题则不执行交换位置;二者中只有一个以字母或者数字开头，则不交换位置, 维持通用排序的顺序不变
        // 2. 二者中都以字母或者数字开头则进行比对
        // 3. 二者中的其他情况则进行比对, 如果不是从小到大顺序排列，则交换位置

        if (info1.isTitle() || info2.isTitle()
                || (info1.startWithLetter() && !info2.startWithLetter())
                || (!info1.startWithLetter() && info2.startWithLetter()))
            return false;

        if ((info1.startWithLetter() && info2.startWithLetter()) || (info1.startWithNum() && info2.startWithNum())) {
            // 交换位置
            return (appPinyinName1.compare(appPinyinName2, Qt::CaseSensitive) < 0);
        }

        // 交换位置
        return (appPinyinName1.compare(appPinyinName2, Qt::CaseSensitive) < 0);
    });
}

const ItemInfo_v1 AppsManager::getItemInfo(const QString &appKey)
{
    ItemInfo_v1 itemInfo;
    for (const ItemInfo_v1 &info : m_allAppInfoList) {
        if (info.m_key == appKey) {
            itemInfo = info;
            break;
        }
    }

    return itemInfo;
}

void AppsManager::dropToCollected(const ItemInfo_v1 &info, int row)
{
    if (m_collectSortedList.contains(info))
        return;

    // 越过视图列表区域,默认加入到最后
    if (row == -1)
        m_collectSortedList.append(info);
    else
        m_collectSortedList.insert(row, info);

    saveCollectedSortedList();
    emit dataChanged(AppsListModel::Collect);
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
        const ItemInfo_v1 info = list.at(i);
        fillMapData(itemMap, info);

        for (int j = 0; j < info.m_appInfoList.size(); j++) {
            const ItemInfo_v1 dirInfo = info.m_appInfoList.at(j);
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

    if ((mode == AppsListModel::All) || (mode == AppsListModel::Dir))
        handleData(m_usedSortedList);

    if (mode == AppsListModel::Collect)
        handleData(m_collectSortedList);

    saveAppCategoryInfoList();
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
    foreach (ItemInfo_v1 info, m_allAppInfoList) {
        if (info.m_key != appKey)
            continue;

        m_stashList.append(info);
        m_allAppInfoList.removeOne(info);

        generateCategoryMap();
        refreshUsedInfoList();
        saveAppCategoryInfoList();
        break;
    }
}

/**
 * @brief AppsManager::abandonStashedItem 卸载应用更新列表
 * @param appKey 应用的key
 */
void AppsManager::abandonStashedItem(const QString &appKey)
{
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

void AppsManager::restoreItem(const QString &appKey, AppsListModel::AppCategory mode, const int pos)
{
    if (pos == -1)
        return;

    foreach (ItemInfo_v1 info, m_stashList) {
        if (info.m_key != appKey)
            continue;

        if (mode == AppsListModel::All) {
            m_usedSortedList.insert(pos, info);
        } else if (mode == AppsListModel::Collect) {
            m_collectSortedList.insert(pos, info);
        }

        m_allAppInfoList.append(info);
        m_stashList.removeOne(info);

        generateCategoryMap();
        saveAppCategoryInfoList();
        saveUsedSortedList();
        saveCollectedSortedList();
        break;
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
#ifdef USE_AM_API
    m_newInstalledAppsList = m_amDbusLauncherInter->GetAllNewInstalledApps().value();
#else
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();
#endif
    refreshCategoryInfoList();
    refreshUsedInfoList();
    saveAppCategoryInfoList();
    readCollectedCacheData();

    // 全屏全屏自由模式都需要界面计算,小窗口直接加载
#ifdef USE_AM_API
    if (!m_amDbusLauncherInter->fullscreen())
#else
    if (!m_launcherInter->fullscreen())
#endif
        emit startLoadIcon();
}

void AppsManager::saveUsedSortedList()
{
    m_usedSortSetting->setValue("lists", getCacheMapData(m_usedSortedList));
}

void AppsManager::saveCollectedSortedList()
{
    m_collectedSetting->setValue("lists", QVariant::fromValue(getCacheMapData(m_collectSortedList)));
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
#ifdef USE_AM_API
    m_amDbusLauncherInter->RequestUninstall(appKey, false);
#else
    m_launcherInter->RequestUninstall(appKey, false);
#endif

    // 刷新各列表的分页信息
    emit dataChanged(AppsListModel::All);
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

    ItemInfo_v1 info = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    if (!isInCollected) {
        if (m_collectSortedList.indexOf(info) == -1)
            m_collectSortedList.append(info);
    } else {
        if (m_collectSortedList.indexOf(info) != -1)
            m_collectSortedList.removeOne(info);
    }

    saveCollectedSortedList();
    emit dataChanged(AppsListModel::Collect);
}

void AppsManager::onMoveToFirstInCollected(const QModelIndex index)
{
    if (!index.isValid() || (index.row() <= 0))
        return;

    ItemInfo_v1 info = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    if (m_collectSortedList.indexOf(info) != -1) {
        m_collectSortedList.removeOne(info);
        m_collectSortedList.insert(0, info);
    }

    saveCollectedSortedList();
    emit dataChanged(AppsListModel::Collect);
}

void AppsManager::markLaunched(QString appKey)
{
    if (appKey.isEmpty() || !m_newInstalledAppsList.contains(appKey))
        return;

    m_newInstalledAppsList.removeOne(appKey);

    emit newInstallListChanged();
}

void AppsManager::delayRefreshData()
{
#ifdef USE_AM_API
    m_newInstalledAppsList = m_amDbusLauncherInter->GetAllNewInstalledApps().value();
#else
    m_newInstalledAppsList = m_launcherInter->GetAllNewInstalledApps().value();
#endif

    generateCategoryMap();

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
        return m_usedSortedList.size();
    case AppsListModel::PluginSearch:
        return m_appSearchResultList.size();
    case AppsListModel::Collect:
        return m_collectSortedList.size();
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
    case AppsListModel::Collect:
        Q_ASSERT(m_collectSortedList.size() > index);
        return m_collectSortedList[index];
    case AppsListModel::All:
        Q_ASSERT(m_usedSortedList.size() > index);
        return m_usedSortedList[index];
    case AppsListModel::Search:
        Q_ASSERT(m_usedSortedList.size() > index);
        return m_usedSortedList[index];
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
    QPair<QString, int> tmpKey { cacheKey(info) , iconSize};

    // TODO: 直接从主线程加载图标, 避免多线程加载图标影响v23调试效果, 暂时不删除预加载图标缓存模块,
    // 因为避免龙芯设备上,全屏应用时, 首次切换存在翻页卡顿问题.先解决这个问题,后面优化.
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

    // 兼容历史版本, 1050以前使用list, v23即以后使用lists作为键值
    if (APP_USED_SORTED_LIST.contains("list")) {
        ItemInfoList oldUsedSortedList;
        QByteArray usedBuf = APP_USED_SORTED_LIST.value("list").toByteArray();
        QDataStream in(&usedBuf, QIODevice::ReadOnly);
        in >> oldUsedSortedList;
        m_usedSortedList = ItemInfo_v1::itemListToItemV1List(oldUsedSortedList);
    } else {
        m_usedSortedList.append(readCacheData(m_usedSortSetting->value("lists").toMap()));
    }

    foreach(ItemInfo_v1 info , m_usedSortedList) {
        bool bContains = fuzzyMatching(filters, info.m_key);
        if (bContains) {
            m_usedSortedList.removeOne(info);
        }
    }

#ifdef USE_AM_API
    const ItemInfoList_v1 &datas = ItemInfo_v1::itemV2ListToItemV1List(reply.value());
#else
    const ItemInfoList_v1 &datas = ItemInfo_v1::itemListToItemV1List(reply.value());
#endif

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
            itemInfoList_v1 = ItemInfo_v1::itemListToItemV1List(itemInfoList);
        } else if (m_categorySetting->contains(QString("lists_%1").arg(startIndex))) {
            itemInfoList_v1 << readCacheData(m_categorySetting->value(QString("lists_%1").arg(startIndex)).toMap());
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

    m_usedSortedList.append(readCacheData(m_usedSortSetting->value("lists").toMap()));

    if (m_usedSortedList.isEmpty()) {
        m_usedSortedList = m_allAppInfoList;
    }

    ItemInfoList_v1::ConstIterator allItemItor = m_allAppInfoList.constBegin();
    for (; allItemItor != m_allAppInfoList.constEnd(); ++allItemItor) {
        if (!m_usedSortedList.contains(*allItemItor)) {
            m_usedSortedList.append(*allItemItor);
        }
    }

    // 更新所有应用列表
    foreach (const ItemInfo_v1 info, m_usedSortedList) {
           if (!m_allAppInfoList.contains(info))
               m_usedSortedList.removeOne(info);
    }

    // 更新收藏列表
    foreach (const ItemInfo_v1 info, m_collectSortedList) {
           if (!m_allAppInfoList.contains(info))
               m_collectSortedList.removeOne(info);
    }

    updateUsedListInfo();
    saveUsedSortedList();
    saveCollectedSortedList();
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

    // 更新分类应用列表, 小窗口左侧标题模式的数据源
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

    // 移除 m_usedSortedList 所有应用中不存在的应用
    foreach (ItemInfo_v1 info, m_usedSortedList) {
        int idx = m_allAppInfoList.indexOf(info);
        if (idx == -1) {
            m_usedSortedList.removeOne(info);
        }
    }

    // 移除 m_collectSortedList 收藏应用中不存在的应用
    foreach (ItemInfo_v1 info, m_collectSortedList) {
        int idx = m_allAppInfoList.indexOf(info);
        if (idx == -1) {
            m_collectSortedList.removeOne(info);
        }
    }

    // 获取应用分类ID列表
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

    // 获取标题分类分类列表
    generateTitleCategoryMap();

    // 获取字母排序分类
    generateLetterCategoryMap();
}

void AppsManager::generateTitleCategoryMap()
{
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
}

void AppsManager::generateLetterCategoryMap()
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
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QString("/%1/dde-launcher-app-collect-list.json").arg(qApp->organizationName());

    if (!QFile::exists(filePath)) {
        // 获取小窗口默认收藏列表
        ItemInfoList_v1 tempData = m_allAppInfoList;
        m_collectSortedList.clear();
        filterCollectedApp(tempData);
    } else {
        m_collectSortedList.append(readCacheData(m_collectedSetting->value("lists").toMap()));
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
        m_usedSortedList.append(info);
    } else if (operation == "deleted") {
        m_allAppInfoList.removeOne(info);
        m_usedSortedList.removeOne(info);
        //一般情况是不需要的，但是类似wps这样的程序有点特殊，删除一个其它的二进制程序也删除了，需要保存列表，否则刷新的时候会刷新出齿轮的图标
        //新增和更新则无必要
        saveUsedSortedList();
    } else if (operation == "updated") {
        Q_ASSERT(m_allAppInfoList.contains(info));

        // 更新所有应用列表
        int appIndex = m_allAppInfoList.indexOf(info);
        if (appIndex != -1)
            m_allAppInfoList[appIndex].updateInfo(info);

        // 更新按照最近使用顺序排序的列表
        int sortAppIndex = m_usedSortedList.indexOf(info);
        if (sortAppIndex != -1)
            m_usedSortedList[sortAppIndex].updateInfo(info);
    } else {
        qDebug() << "nonexistent condition, operation:" << operation;
        return;
    }

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
        m_usedSortedList.append(info);
    } else if (operation == "deleted") {
        m_allAppInfoList.removeOne(info);
        m_usedSortedList.removeOne(info);
        //一般情况是不需要的，但是类似wps这样的程序有点特殊，删除一个其它的二进制程序也删除了，需要保存列表，否则刷新的时候会刷新出齿轮的图标
        //新增和更新则无必要
        saveUsedSortedList();
    } else if (operation == "updated") {
        Q_ASSERT(m_allAppInfoList.contains(info));
        // 更新所有应用列表
        int appIndex = m_allAppInfoList.indexOf(info);
        if (appIndex != -1)
            m_allAppInfoList[appIndex].updateInfo(info);

        // 更新按照最近使用顺序排序的列表
        int sortAppIndex = m_usedSortedList.indexOf(info);
        if (sortAppIndex != -1)
            m_usedSortedList[sortAppIndex].updateInfo(info);
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
    int size = (pixmap.size() / qApp->devicePixelRatio()).width();

    QPair<QString, int> tmpKey { cacheKey(info), size};

    // 命令行安装应用后，卸载应用的确认弹框偶现左上角图标呈齿轮的情况
    QPixmap appIcon;
    if (IconCacheManager::existInCache(tmpKey)) {
        IconCacheManager::getPixFromCache(tmpKey, appIcon);
        unInstallDialog.setIcon(appIcon);
    } else {
        static int tryNum = 0;
        m_uninstallDlgIsShown = false;
        ++tryNum;
        if (tryNum <= 5) {
            QTimer::singleShot(100, this, [ = ]() { uninstallApp(modelIndex); });
            return;
        } else {
            QIcon icon = QIcon(":/widgets/images/application-x-desktop.svg");
            appIcon = icon.pixmap(QSize(size, size));
            unInstallDialog.setIcon(appIcon);
        }
    }

    QStringList buttons;
    buttons << tr("Cancel") << tr("Confirm");
    unInstallDialog.addButtons(buttons);

    connect(&unInstallDialog, &DDialog::buttonClicked, [&](int clickedResult) {
        // 0 means "cancel" button clicked
        if (clickedResult == 0) {
            return;
        }

        uninstallApp(appKey);
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
     * 1. 如果拖拽的对象是文件夹, 那么文件夹及其内部的应用都需要写入到dropItem对象的数据中,
       且拖拽的对象文件夹标识应该修改为普通应用, 并写入到本地数据中.
       2. 如果拖拽的对象是应用, 那么将拖拽应用的数据写入到dropItem对象的数据中,
       且应该将dropItem对象文件夹标识设置为true.
       3. 命名规则, 拖拽对象与释放对象为同类时, 取该分类标题名称;如果不同, 则以释放对象(显示文件夹中第一位应用)的分类
       标题作为文件夹名称
    */

    ItemInfo_v1 dragItemInfo = dragIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    ItemInfo_v1 dropItemInfo = dropIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    bool dragItemIsDir = dragIndex.data(AppsListModel::ItemIsDirRole).toBool();
    bool dropItemIsDir = dropIndex.data(AppsListModel::ItemIsDirRole).toBool();

    auto saveAppDirData = [ & ](ItemInfoList_v1 &list) {
        int index = list.indexOf(dropItemInfo);
        if (index == -1)
            return;

        // 最初都不是应用文件夹, 当是文件夹了, 应用名称就不用变化了, 因此也无需处理
        if (!dropItemIsDir) {
            list[index].m_isDir = true;

            // 不论拖拽对象与释放对象是否为同一类应用, 都把释放对象的分类标题作为文件夹名称
            int idIndex = static_cast<int>(dropItemInfo.m_categoryId);
            if (m_categoryTs.size() > idIndex)
                list[index].m_name = m_categoryTs[idIndex];
        }

        ItemInfoList_v1 itemList;
        // 释放的对象为应用时, 写入到列表中.
        // 释放的对象为文件夹时, 内容已经在前一次写入到了列表中, 因此无需处理
        if (!dropItemIsDir)
            itemList.append(dropItemInfo);

        // 释放的对象中不包含拖拽的对象时
        if (!list[index].m_appInfoList.contains(dragItemInfo)) {
            if (!dragItemIsDir) {
                // 仅仅更新dropItem的文件夹标识即可
                // 被拖动的应用放在第二位
                itemList.append(dragItemInfo);
            } else {
                // 当拖动的文件夹进入到另一个文件夹中时, 确保拖动的这个文件夹标识调整为普通应用标识,
                // 同时保持拖动之前的排列顺序插入到新的文件夹结构中
                dragItemInfo.m_isDir = false;
                int dragIndex = list.indexOf(dragItemInfo);
                if (dragIndex != -1) {
                    list[dragIndex] = dragItemInfo;
                }
                itemList.append(dragItemInfo.m_appInfoList);
            }

            list[index].m_appInfoList.append(itemList);
        } else {
            // 释放的对象中包含拖拽的对象属于异常情况
            return;
        }
    };

    saveAppDirData(m_usedSortedList);
}

void AppsManager::updateDrawerTitle(const QModelIndex &index,const QString &newTitle)
{
    if (!index.isValid())
        return;

    ItemInfo_v1 info = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    int itemIndex = m_usedSortedList.indexOf(info);
    if (itemIndex != -1) {
        info.m_name = newTitle;
        m_usedSortedList.replace(itemIndex, info);
    }
}

QList<QPixmap> AppsManager::getDirAppIcon(QModelIndex modelIndex)
{
    QList<QPixmap> pixmapList;
    ItemInfoList_v1 infoList;
    ItemInfo_v1 info = modelIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    if (m_usedSortedList.contains(info) && info.m_isDir)
        infoList.append(info.m_appInfoList);

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
