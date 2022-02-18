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
#include "dbuslauncher.h"
#include "dbustartmanager.h"
#include "dbusdock.h"
#include "calculate_util.h"

#include <DGuiApplicationHelper>

#include <QHash>
#include <QSettings>
#include <QPixmap>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDBusArgument>
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
    void stashItem(const QModelIndex &index);
    void stashItem(const QString &appKey);
    void abandonStashedItem(const QString &appKey);
    void restoreItem(const QString &appKey, const int pos = -1);
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

signals:
    void itemDataChanged(const ItemInfo &info) const;
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
    void loadItem(const ItemInfo &info, const QString &operationStr);

public slots:
    void saveUserSortedList();
    void saveUsedSortedList();
    void searchApp(const QString &keywords);
    void launchApp(const QModelIndex &index);
    void uninstallApp(const QString &appKey, const int displayMode = ALL_APPS);
    const ItemInfoList appsInfoList(const AppsListModel::AppCategory &category) const;
    static int appsInfoListSize(const AppsListModel::AppCategory &category);
    static const ItemInfo appsInfoListIndex(const AppsListModel::AppCategory &category,const int index);
    static const ItemInfoList &windowedCategoryList();
    static const ItemInfoList &windowedFrameItemInfoList();
    static const ItemInfoList &fullscreenItemInfoList();
    static const QHash<AppsListModel::AppCategory, ItemInfoList> &categoryList();

    bool appIsNewInstall(const QString &key);
    bool appIsAutoStart(const QString &desktop);
    bool appIsOnDock(const QString &desktop);
    bool appIsOnDesktop(const QString &desktop);
    bool appIsProxy(const QString &desktop);
    bool appIsEnableScaling(const QString &desktop);
    const QPixmap appIcon(const ItemInfo &info, const int size = 0);
    const QString appName(const ItemInfo &info, const int size);
    int appNums(const AppsListModel::AppCategory &category) const;

    void handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber);
    static QHash<AppsListModel::AppCategory, ItemInfoList> getAllAppInfo();

private:
    explicit AppsManager(QObject *parent = nullptr);

    void appendSearchResult(const QString &appKey);
    void sortByPresetOrder(ItemInfoList &processList);
    void sortByInstallTimeOrder(ItemInfoList &processList);
    void refreshCategoryInfoList();
    void refreshUsedInfoList();
    void refreshCategoryUsedInfoList();
    void refreshUserInfoList();
    void updateUsedListInfo();
    void generateCategoryMap();
    void refreshAppAutoStartCache(const QString &type = QString(), const QString &desktpFilePath = QString());
    void onSearchTimeOut();
    void refreshAppListIcon(DGuiApplicationHelper::ColorType themeType);
    const ItemInfo createOfCategory(qlonglong category);

private slots:
    void onIconThemeChanged();
    void searchDone(const QStringList &resultList);
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
    static QReadWriteLock m_appInfoLock;
    static QHash<AppsListModel::AppCategory, ItemInfoList> m_appInfos;      // 应用分类容器

private:
    DBusLauncher *m_launcherInter;
    DBusStartManager *m_startManagerInter;
    DBusDock *m_dockInter;

    QString m_searchText;
    QStringList m_newInstalledAppsList;                                     // 新安装应用列表
    ItemInfoList m_allAppInfoList;                                          // 所有app信息列表
    static ItemInfoList m_usedSortedList;                                   // 全屏应用列表
    static ItemInfoList m_userSortedList;                                   // 小窗口应用列表
    static ItemInfoList m_appSearchResultList;                              // 搜索结果列表
    ItemInfoList m_stashList;
    static ItemInfoList m_categoryList;                                     // 小窗口应用分类目录列表

    ItemInfo m_unInstallItem = ItemInfo();
    ItemInfo m_beDragedItem = ItemInfo();

    CalculateUtil *m_calUtil;
    QTimer *m_searchTimer;
    QTimer *m_delayRefreshTimer;                                            // 延迟刷新应用列表定时器指针对象
    QTimer *m_RefreshCalendarIconTimer;

    QDate m_curDate;
    int m_lastShowDate;

    int m_tryNums;                                                          // 获取应用图标时尝试的次数
    int m_tryCount;                                                         // 超过10次停止遍历
    ItemInfo m_itemInfo;                                                    // 当前需要更新的应用信息

    static QPointer<AppsManager> INSTANCE;
    static QGSettings *m_launcherSettings;
    static QSet<QString> APP_AUTOSTART_CACHE;
    static QSettings APP_USER_SORTED_LIST;
    static QSettings APP_USED_SORTED_LIST;
    static QSettings APP_CATEGORY_USED_SORTED_LIST;
    QStringList m_categoryTs;
    QStringList m_categoryIcon;
    QGSettings *m_filterSetting;

    bool m_iconValid;                                                       // 获取图标状态标示

    bool m_trashIsEmpty;
    QFileSystemWatcher *m_fsWatcher;

    IconCacheManager *m_iconCacheManager;
    QThread *m_iconCacheThread;
    QTimer *m_updateCalendarTimer;
};

#endif // APPSMANAGER_H
