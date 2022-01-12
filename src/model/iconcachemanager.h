/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     songwentao <songwentao@uniontech.com>
 *
 * Maintainer: songwentao <songwentao@uniontech.com>
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
 *
 * This program aims to cache the the icon and name of apps to the hash table,
 * which can decrease the repeated resource consumption of loading the app info in the
 * running time.
 */
#ifndef ICONCACHEMANAGER_H
#define ICONCACHEMANAGER_H

#include "iteminfo.h"

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QReadWriteLock>

class DBusLauncher;

class IconCacheManager : public QObject
{
    Q_OBJECT

public:
    static IconCacheManager *instance();
    static bool iconLoadState();
    static void setIconLoadState(bool state);

    static bool existInCache(const QPair<QString, int> &tmpKey);
    static void getPixFromCache(QPair<QString, int> &tmpKey, QPixmap &pix);
    static void insertCache(const QPair<QString, int> &tmpKey, const QPixmap &pix);
    static void removeItemFromCache(const ItemInfo &info);

private:
    explicit IconCacheManager(QObject *parent = nullptr);

    void createPixmap(const ItemInfo &itemInfo, int size);

    void setFullFreeLoadState(bool state);
    bool fullFreeLoadState();

    void setFullCategoryLoadState(bool state);
    bool fullCategoryLoadState();

    void setSmallWindowLoadState(bool state);
    bool smallWindowLoadState();

signals:
    void iconLoaded();

public slots:
    void loadWindowIcon();

    void loadFullWindowIcon();
    void loadFullWindowIcon(double ratio);

    void loadCategoryWindowIcon();
    void loadCategoryWindowIcon(double ratio);

    void preloadCategory();
    void preloadFullFree();

    void ratioChange(double ratio);

    void loadItem(const ItemInfo &info, const QString &operationStr);

    void removeSmallWindowCache();

    void updateCanlendarIcon();

private:
    static QReadWriteLock m_cacheDataLock;
    static QHash<QPair<QString, int>, QVariant> m_CacheData;
    static std::atomic<bool> m_loadState;
    static std::atomic<bool> m_fullFreeLoadState;
    static std::atomic<bool> m_fullCategoryLoadState;
    static std::atomic<bool> m_smallWindowLoadState;

    DBusLauncher *m_launcherInter;

    ItemInfo m_itemInfo;
    ItemInfo m_calendarInfo;
    int m_iconSize;
    int m_calendarSize;
    bool m_iconValid;
    int m_tryNums;
    int m_tryCount;
};

#endif // ICONCACHEMANAGER_H
