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
#include "src/dbusinterface/dbuslauncher.h"
#include "src/dbusinterface/dbusfileinfo.h"
#include "src/dbusinterface/dbustartmanager.h"
#include "src/dbusinterface/dbusdock.h"
#include "src/dbusinterface/dbusdisplay.h"
#include "src/global_util/calculate_util.h"

#include <QMap>
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
    int getPageCount(const AppsListModel::AppCategory category){return m_pageCount[category];}
    void setPageIndex(const AppsListModel::AppCategory category,int pageIndex){m_pageIndex[category] = pageIndex;}
    int getPageIndex(const AppsListModel::AppCategory category){return m_pageIndex[category];}

signals:
    void itemDataChanged(const ItemInfo &info) const;
    void dataChanged(const AppsListModel::AppCategory category) const;
    void layoutChanged(const AppsListModel::AppCategory category) const;
    void requestTips(const QString &tips) const;
    void newInstallListChanged() const;
    void requestHideTips() const;
    void categoryListChanged() const;
    void dockGeometryChanged() const;

public slots:
    void saveUserSortedList();
    void saveUsedSortedList();
    void searchApp(const QString &keywords);
    void launchApp(const QModelIndex &index);
    void uninstallApp(const QString &appKey);
    const ItemInfoList appsInfoList(const AppsListModel::AppCategory &category, int pageIndex=0) const;

    bool appIsNewInstall(const QString &key);
    bool appIsAutoStart(const QString &desktop);
    bool appIsOnDock(const QString &desktop);
    bool appIsOnDesktop(const QString &desktop);
    bool appIsProxy(const QString &desktop);
    bool appIsEnableScaling(const QString &desktop);
    const QPixmap appIcon(const ItemInfo &info, const int size);
    int appNums(const AppsListModel::AppCategory &category) const;
    inline void clearCache() { m_iconCache.clear(); }

    void handleItemChanged(const QString &operation, const ItemInfo &appInfo, qlonglong categoryNumber);

private:
    explicit AppsManager(QObject *parent = 0);

    void appendSearchResult(const QString &appKey);
    void sortCategory(const AppsListModel::AppCategory category);
    void sortByPresetOrder(ItemInfoList &processList);
    void refreshCategoryInfoList();
    void refreshUsedInfoList();
    void refreshUserInfoList();
    void updateUsedListInfo();
    void generateCategoryMap();
    void refreshAppAutoStartCache(const QString &type = QString(), const QString &desktpFilePath = QString());
    void onSearchTimeOut();
    void refreshNotFoundIcon();
    void ReflashSortList();
    void refreshAppListIcon();
    void ReflashCategorySortList(const AppsListModel::AppCategory& category);

private slots:
    void onIconThemeChanged();
    void searchDone(const QStringList &resultList);
    void markLaunched(QString appKey);
    void delayRefreshData();

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
    ItemInfoList m_usedSortedListVec[MAX_VIEW_NUM];
    ItemInfoList m_userSortedList; // Mini
    ItemInfoList m_appSearchResultList;
    ItemInfoList m_stashList;
    ItemInfoList m_categoryList;
    QMap<AppsListModel::AppCategory, ItemInfoList> m_appInfos;
    QMap<AppsListModel::AppCategory, ItemInfoList> m_appInfosVec[MAX_VIEW_NUM];

    ItemInfo m_unInstallItem = ItemInfo();
    ItemInfo m_beDragedItem = ItemInfo();

    CalculateUtil *m_calUtil;
    int m_pageCount[CATEGORY_COUNT+4] = {0};
    int m_pageIndex[CATEGORY_COUNT+4] = {0};
    QTimer *m_searchTimer;
    QTimer *m_delayRefreshTimer;

    static QPointer<AppsManager> INSTANCE;
    static QGSettings LAUNCHER_SETTINGS;
    static QSet<QString> APP_AUTOSTART_CACHE;
    static QSettings APP_USER_SORTED_LIST;
    static QSettings APP_USED_SORTED_LIST;
    QMap<QPair<QString, int>, QPixmap> m_iconCache;
    std::map<std::pair<ItemInfo, int>, int> m_notExistIconMap;
    QStringList m_categoryTs;
    QStringList m_categoryIcon;
};

#endif // APPSMANAGER_H
