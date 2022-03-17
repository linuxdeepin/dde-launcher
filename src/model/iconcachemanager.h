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
    static void resetIconData();

    static bool iconLoadState();
    static void setIconLoadState(bool state);

    static bool existInCache(const QPair<QString, int> &tmpKey);
    static void getPixFromCache(QPair<QString, int> &tmpKey, QPixmap &pix);
    static void insertCache(const QPair<QString, int> &tmpKey, const QPixmap &pix);

private:
    explicit IconCacheManager(QObject *parent = nullptr);

    void createPixmap(const ItemInfo &itemInfo, int size);
    void removeItemFromCache(const ItemInfo &info);
    double getCurRatio();

signals:
    void iconLoaded();

public slots:
    void loadWindowIcon();
    void loadOtherIcon();

    void loadCurRatioIcon(int mode);
    void loadOtherRatioIcon(int mode);
    void loadFullWindowIcon();

    void loadItem(const ItemInfo &info, const QString &operationStr);
    void removeSmallWindowCache();
    void updateCanlendarIcon();

private:
    static QReadWriteLock m_iconLock;
    static QHash<QPair<QString, int>, QVariant> m_iconCache;
    static std::atomic<bool> m_loadState;

    DBusLauncher *m_launcherInter;

    ItemInfo m_calendarInfo;
    bool m_iconValid;
    int m_tryNums;
    int m_tryCount;
};

#endif // ICONCACHEMANAGER_H
