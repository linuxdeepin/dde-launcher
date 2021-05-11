/*
 * Copyright (C) 2021 ~ 2023 Deepin Technology Co., Ltd.
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

#ifndef ICONFRESHTHREAD_H
#define ICONFRESHTHREAD_H

#include "iteminfo.h"

#include <QThread>
#include <QQueue>
#include <QSemaphore>

class IconFreshThread : public QThread
{
    Q_OBJECT

public:
    IconFreshThread(QObject *parent = nullptr);

    static void setQueue(QQueue<ItemInfo>& info);
    static bool getThreadState();
    static void releaseSemo();
    static void releaseInstallAppSemo();
    static void releaseReloadAppSemo();

    void createPixmap(const ItemInfo &itemInfo, int size = 0);

protected:
    virtual void run();

signals:
    void iconFresh();

public slots:
    void refreshIcon();
    void releaseThread();

public:
    static QReadWriteLock m_installAppListLock;          // 新安装应用列表锁

private:
    int m_tryNums;                                       // 获取应用图标时尝试的次数
    ItemInfo m_itemInfo;                                 // 当前需要更新的应用信息
    static QQueue<ItemInfo> m_InstallAppInfo;            // 所有应用信息
    static std::atomic<bool> m_runningState;             // 没有事件循环的类，无法直接quit()停止，因此设置线程运行标识位
    static std::atomic<bool> m_loadAllIconState;         // 加载应用资源的标识，系统主题变化后再加载一次
    static std::atomic<bool> m_iconValid;
};

#endif
