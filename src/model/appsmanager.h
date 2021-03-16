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
#include "dbusfileinfo.h"
#include "dbustartmanager.h"
#include "dbusdock.h"
#include "dbusdisplay.h"
#include "calculate_util.h"

#include <QHash>
#include <QSettings>
#include <QPixmap>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDBusArgument>
#include <QList>
#include <memory>

#define DOCK_POS_RIGHT  1
#define DOCK_POS_BOTTOM 2
#define DOCK_POS_LEFT   3
#define DOCK_POS_TOP    0

#define LEFT_PADDING 200
#define RIGHT_PADDING 200

#define MAX_VIEW_NUM    255
#define CATEGORY_COUNT    11

class CalculateUtil;
class AppsManager : public QObject
{
    Q_OBJECT

public:
    enum CacheType {
        TextType,
        ImageType
    };

    static AppsManager *instance();
    void stashItem(const QModelIndex &index);
    void stashItem(const QString &appKey);
    void abandonStashedItem(const QString &appKey);
    void restoreItem(const QString &appKey, const int pos = -1);
    int dockPosition() const;
    int dockWidth() const;
    QRect dockGeometry() const;
    bool isHaveNewInstall() const { return !m_newInstalledAppsList.isEmpty(); }
    bool isVaild();
    void refreshAllList();
    const QPixmap getThemeIcon(const ItemInfo &itemInfo, const int size);
    int getPageCount(const AppsListModel::AppCategory category);
    void pushPixmap();
    void pushPixmap(const ItemInfo &itemInfo);
    const QScreen * currentScreen();
    int getVisibleCategoryCount();

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

    void iconLoadFinished();

public slots:
    void saveUserSortedList();
    void saveUsedSortedList();
    void searchApp(const QString &keywords);
    void launchApp(const QModelIndex &index);
    void uninstallApp(const QString &appKey, const int displayMode = ALL_APPS);
    const ItemInfoList appsInfoList(const AppsListModel::AppCategory &category) const;
    int appsInfoListSize(const AppsListModel::AppCategory &category) const;
    const ItemInfo appsInfoListIndex(const AppsListModel::AppCategory &category,const int index) const;

    bool appIsNewInstall(const QString &key);
    bool appIsAutoStart(const QString &desktop);
    bool appIsOnDock(const QString &desktop);
    bool appIsOnDesktop(const QString &desktop);
    bool appIsProxy(const QString &desktop);
    bool appIsEnableScaling(const QString &desktop);
    const QPixmap appIcon(const ItemInfo &info, const int size);
    const QString appName(const ItemInfo &info, const int size);
    int appNums(const AppsListModel::AppCategory &category) const;
    inline void clearCache() { m_CacheData.clear(); }

    void handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber);

private:
    explicit AppsManager(QObject *parent = nullptr);

    void appendSearchResult(const QString &appKey);
    void sortCategory(const AppsListModel::AppCategory category);
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
    void refreshNotFoundIcon();
    void refreshAppListIcon();
    QString cacheKey(const ItemInfo &itemInfo, CacheType size);

private slots:
    void onIconThemeChanged();
    void searchDone(const QStringList &resultList);
    void markLaunched(QString appKey);
    void delayRefreshData();
    void updateTrashState();
    /**
     * @brief 模糊匹配，反向查询key是否包含list任一个元素
     * 
     * @param list 关键字列表
     * @param key 要模糊匹配的关键词
     * @return true 表示匹配成功
     * @return false 表示匹配失败
     */
    bool fuzzyMatching(const QStringList& list, const QString& key);

private:
    const ItemInfo createOfCategory(qlonglong category);

private:
    DBusLauncher *m_launcherInter;
    DBusStartManager *m_startManagerInter;
    DBusDock *m_dockInter;
    std::unique_ptr<QTimer> m_iconRefreshTimer;

    QString m_searchText;
    QStringList m_newInstalledAppsList;
    ItemInfoList m_allAppInfoList;
    ItemInfoList m_usedSortedList; // FullScreen
    ItemInfoList m_userSortedList; // Mini
    ItemInfoList m_appSearchResultList;
    ItemInfoList m_stashList;
    ItemInfoList m_categoryList;
    QHash<AppsListModel::AppCategory, ItemInfoList> m_appInfos; // 全屏分类模式下保存的应用

    ItemInfo m_unInstallItem = ItemInfo();
    ItemInfo m_beDragedItem = ItemInfo();

    CalculateUtil *m_calUtil;
    QTimer *m_searchTimer;
    QTimer *m_delayRefreshTimer;
    QTimer *m_RefreshCalendarIconTimer;

    QDate m_curDate;
    int m_lastShowDate;
    bool m_trashIsEmpty;

    static QPointer<AppsManager> INSTANCE;
    static QGSettings LAUNCHER_SETTINGS;
    static QSet<QString> APP_AUTOSTART_CACHE;
    static QSettings APP_USER_SORTED_LIST;
    static QSettings APP_USED_SORTED_LIST;
    static QSettings APP_CATEGORY_USED_SORTED_LIST;
    QHash<QPair<QString, int>, QVariant> m_CacheData;
    std::map<std::pair<ItemInfo, int>, int> m_notExistIconMap;
    QStringList m_categoryTs;
    QStringList m_categoryIcon;
    QGSettings* m_filterSetting = nullptr;
    QFileSystemWatcher *m_fsWatcher;
};

#endif // APPSMANAGER_H
