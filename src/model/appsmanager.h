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

#ifndef APPSMANAGER_H
#define APPSMANAGER_H

#include "appslistmodel.h"
#include "dbustartmanager.h"
#include "calculate_util.h"
#include "common.h"

#ifdef USE_AM_API
#include "amdbuslauncherinterface.h"
#include "amdbusdockinterface.h"
#else
#include "dbuslauncher.h"
#include "dbusdock.h"
#endif

#include <DGuiApplicationHelper>
#include <DDialog>

#include <QHash>
#include <QSettings>
#include <QPixmap>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QList>

DGUI_USE_NAMESPACE

#define LEFT_PADDING 200
#define RIGHT_PADDING 200

#define MAX_VIEW_NUM    255
#define CATEGORY_COUNT    11

class CalculateUtil;
class QThread;
class IconCacheManager;

class AppsManager : public QObject
{
    Q_OBJECT

public:
    static AppsManager *instance();
    void dragdropStashItem(const QModelIndex &index, AppsListModel::AppCategory mode);
    void stashItem(const QModelIndex &index);
    void stashItem(const QString &appKey);
    void abandonStashedItem(const QString &appKey);
    void restoreItem(const QString &appKey, AppsListModel::AppCategory mode, const int pos = -1);
    int dockPosition() const;
    QRect dockGeometry() const;
    bool isHaveNewInstall() const { return !m_newInstalledAppsList.isEmpty(); }
    bool isVaild();
    void refreshAllList();
    int getPageCount(const AppsListModel::AppCategory category);
    const QScreen * currentScreen();
    int getVisibleCategoryCount();
    bool fullscreen() const;
    int displayMode() const;
    qreal getCurRatio();
    void uninstallApp(const QModelIndex &modelIndex);
    bool uninstallDlgShownState() const;

    void updateUsedSortData(QModelIndex dragIndex, QModelIndex dropIndex);
    void updateDrawerTitle(const QModelIndex &index, const QString &newTitle = QString());
    QList<QPixmap> getDirAppIcon(QModelIndex modelIndex);
    void showSearchedData(const AppInfoList &list);
    const ItemInfo_v1 getItemInfo(const QString &appKey);
    void dropToCollected(const ItemInfo_v1 &info, int row);

    static bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map);
    void registerSettingsFormat();

    QSettings::SettingsMap getCacheMapData(const ItemInfoList_v1 &list);
    const ItemInfoList_v1 readCacheData(const QSettings::SettingsMap &map);

signals:
    void itemDataChanged(const ItemInfo_v1 &info) const;
    void dataChanged(const AppsListModel::AppCategory category) const;
    void layoutChanged(const AppsListModel::AppCategory category) const;
    void requestTips(const QString &tips) const;
    void newInstallListChanged() const;
    void requestHideTips() const;
    void categoryListChanged() const;
    void IconSizeChanged() const;
    void dockGeometryChanged() const;

    void itemRedraw(const QModelIndex &index);

    void loadFullWindowIcon();
    void loadCurRationIcon(int mode);
    void loadOtherRatioIcon(int mode);

    void startLoadIcon();
    void loadOtherIcon();
    void loadItem(const ItemInfo_v1 &info, const QString &operationStr);
    void requestHideLauncher();
    void requestHidePopup();

public slots:
    void saveUsedSortedList();
    void saveCollectedSortedList();
    void searchApp(const QString &keywords);
    void launchApp(const QModelIndex &index);
    void uninstallApp(const QString &appKey, const int displayMode = ALL_APPS);
    void onEditCollected(const QModelIndex index, const bool isInCollected);
    void onMoveToFirstInCollected(const QModelIndex index);
    void setDirAppInfoList(const QModelIndex index);
    int appsInfoListSize(const AppsListModel::AppCategory &category);
    const ItemInfoList_v1 appsInfoList(const AppsListModel::AppCategory &category) const;
    const ItemInfo_v1 appsInfoListIndex(const AppsListModel::AppCategory &category,const int index);
    const ItemInfo_v1 appsCategoryListIndex(const int index);
    const ItemInfo_v1 appsLetterListIndex(const int index);
    const ItemInfoList_v1 &windowedCategoryList();
    const ItemInfoList_v1 &windowedFrameItemInfoList();
    const ItemInfoList_v1 &fullscreenItemInfoList();
    const ItemInfo_v1 dirAppInfo(int index);
    const QHash<AppsListModel::AppCategory, ItemInfoList_v1> &categoryList();

    bool appIsNewInstall(const QString &key);
    bool appIsAutoStart(const QString &desktop);
    bool appIsOnDock(const QString &desktop);
    bool appIsOnDesktop(const QString &desktop);
    bool appIsProxy(const QString &desktop);
    bool appIsEnableScaling(const QString &desktop);
    const QPixmap appIcon(const ItemInfo_v1 &info, const int size = 0);
    const QString appName(const ItemInfo_v1 &info, const int size);
    int appNums(const AppsListModel::AppCategory &category);

    // 为顺应数据应用数据结构的变动以及兼容性考虑, 对　handleItemChanged 接口进行了重载．
    void handleItemChanged(const QString &operation, const ItemInfo_v2 &appInfo, qlonglong categoryNumber);
    void handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber);
    const ItemInfo_v1 createOfCategory(qlonglong category);

private:
    explicit AppsManager(QObject *parent = nullptr);

    void sortByPresetOrder(ItemInfoList_v1 &processList);
    void sortByUseFrequence(ItemInfoList_v1 &processList);
    void filterCollectedApp(ItemInfoList_v1 &processList);
    void sortByLetterOrder(ItemInfoList_v1 &processList);
    void sortByInstallTimeOrder(ItemInfoList_v1 &processList);
    void refreshCategoryInfoList();
    void refreshUsedInfoList();
    void saveAppCategoryInfoList();
    void updateUsedListInfo();
    void generateCategoryMap();
    void generateTitleCategoryMap();
    void generateLetterCategoryMap();
    void readCollectedCacheData();
    void refreshAppAutoStartCache(const QString &type = QString(), const QString &desktpFilePath = QString());

private slots:
    void onIconThemeChanged();
    void markLaunched(QString appKey);
    void delayRefreshData();
    void refreshIcon();
    void updateTrashState();
    bool fuzzyMatching(const QStringList& list, const QString& key);
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);
    void onRefreshCalendarTimer();
    void onGSettingChanged(const QString & keyName);
    void stopThread();

public:
    QHash<AppsListModel::AppCategory, ItemInfoList_v1> m_appInfos;      // 应用分类容器
    ItemInfoList_v1 m_appCategoryInfos;                                 // 小窗口左侧带分类标题的应用列表
    ItemInfoList_v1 m_appLetterModeInfos;                               // 小窗口左侧字母排序模式列表
    ItemInfoList_v1 m_collectSortedList;                                // 小窗口收藏列表
    ItemInfoList_v1 m_categoryList;                                     // 小窗口应用分类目录列表
    ItemInfoList_v1 m_appSearchResultList;                              // 搜索结果列表
    ItemInfoList_v1 m_dirAppInfoList;                                   // 应用抽屉列表
    ItemInfoList_v1 m_usedSortedList;                                   // 全屏应用列表

private:
    DBusStartManager *m_startManagerInter;

#ifdef USE_AM_API
    AMDBusLauncherInter *m_amDbusLauncherInter;
    AMDBusDockInter *m_amDbusDockInter;
#else
    DBusLauncher *m_launcherInter;
    DBusDock *m_dockInter;
#endif

    QString m_searchText;
    ItemInfoList_v1 m_allAppInfoList;                                          // 所有app信息列表
    QStringList m_newInstalledAppsList;                                     // 新安装应用列表

    ItemInfoList_v1 m_stashList;
    ItemInfo_v1 m_unInstallItem;
    ItemInfo_v1 m_beDragedItem;

    CalculateUtil *m_calUtil;
    QTimer *m_delayRefreshTimer;                                            // 延迟刷新应用列表定时器指针对象
    QTimer *m_refreshCalendarIconTimer;

    QDate m_curDate;
    int m_lastShowDate;

    int m_tryNums;                                                          // 获取应用图标时尝试的次数
    int m_tryCount;                                                         // 超过10次停止遍历
    ItemInfo_v1 m_itemInfo;                                                    // 当前需要更新的应用信息

    static QPointer<AppsManager> INSTANCE;
    static QGSettings *m_launcherSettings;
    static QSet<QString> APP_AUTOSTART_CACHE;
    static QSettings APP_USER_SORTED_LIST;
    static QSettings APP_USED_SORTED_LIST;
    static QSettings APP_CATEGORY_USED_SORTED_LIST;

    QSettings *m_collectedSetting;
    QSettings *m_categorySetting;
    QSettings *m_usedSortSetting;

    QStringList m_categoryTs;
    QGSettings *m_filterSetting;

    bool m_iconValid;                                                       // 获取图标状态标示

    bool m_trashIsEmpty;
    QFileSystemWatcher *m_fsWatcher;

    IconCacheManager *m_iconCacheManager;
    QThread *m_iconCacheThread;
    QTimer *m_updateCalendarTimer;
    bool m_uninstallDlgIsShown;
};

#endif // APPSMANAGER_H
