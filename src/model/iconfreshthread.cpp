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

#include "iconfreshthread.h"
#include "util.h"
#include "appsmanager.h"
#include "appslistmodel.h"

#include <DHiDPIHelper>
#include <DGuiApplicationHelper>

#include <QIconEngine>
#include <QIcon>
#include <QApplication>
#include <QScreen>
#include <QPixmap>

#include <private/qguiapplication_p.h>
#include <private/qiconloader_p.h>
#include <qpa/qplatformtheme.h>

static QSemaphore semaphore(5);
QReadWriteLock IconFreshThread::m_installAppListLock;
QQueue<ItemInfo> IconFreshThread::m_InstallAppInfo;
std::atomic<bool> IconFreshThread::m_runningState;
std::atomic<bool> IconFreshThread::m_loadAllIconState;
std::atomic<bool> IconFreshThread::m_iconValid;

IconFreshThread::IconFreshThread(QObject *parent) :
    QThread(parent),
    m_tryNums(0)
{
    m_runningState.store(true);
    m_loadAllIconState.store(true);
    m_iconValid.store(false);
}

void IconFreshThread::releaseSemo()
{
    semaphore.release(6);
}

void IconFreshThread::setQueue(QQueue<ItemInfo> &info)
{
    m_installAppListLock.lockForWrite();
    m_InstallAppInfo.append(info);
    m_installAppListLock.unlock();
}

bool IconFreshThread::getThreadState()
{
    return m_runningState.load();
}

void IconFreshThread::refreshIcon()
{
    createPixmap(m_itemInfo);
}

void IconFreshThread::createPixmap(const ItemInfo &itemInfo, int size)
{
    const int iconSize = perfectIconSize(size);
    QPair<QString, int> tmpKey { cacheKey(itemInfo, CacheType::ImageType), iconSize};

    if (!AppsManager::existInCache(tmpKey)) {
        QPixmap pixmap;

        // 日历应用只让它执行一次,为保证时间信息长期更新,不写入缓存,因此也不用多次重复调用,减少系统资源消耗
        if (itemInfo.m_iconKey == "dde-calendar") {
            m_iconValid.store(getThemeIcon(pixmap, itemInfo, size, false));
        } else {
            m_iconValid.store(getThemeIcon(pixmap, itemInfo, size, !m_iconValid.load()));

            if (!m_iconValid.load()) {
                if (m_tryNums < 10) {
                    ++m_tryNums;

                    // 当无法找到文件时，看下文件是否存在
                    if (!QFile::exists(itemInfo.m_iconKey))
                        // 当程序已从缓存中获取图标，应用程序才安装完毕，此时需要强行对缓存中图标的是否为空，进行验证
                        QIcon::setThemeSearchPaths(QIcon::themeSearchPaths());

                    m_itemInfo = itemInfo;

                    // 10秒从缓存中取一次
                    QTimer::singleShot(10 * 1000, this, [=]() { refreshIcon(); });
                } else {
                    m_itemInfo = itemInfo;

                    // 如果图标获取失败，每隔1分钟(1 * 60 * 1000)刷新一次
                    QTimer::singleShot(60 * 1000, this, [=]() { refreshIcon(); });
                }
            } else {
                if (m_tryNums > 0)
                    m_tryNums = 0;
            }
        }
    }
}

void IconFreshThread::releaseInstallAppSemo()
{
    m_runningState.store(true);
    releaseSemo();
}

void IconFreshThread::releaseReloadAppSemo()
{
    // 关闭缓存应用信息线程
    m_runningState.store(false);
    m_loadAllIconState.store(false);

    releaseSemo();

    // 为开始缓存应用信息线程准备状态
    m_runningState.store(true);
    m_loadAllIconState.store(true);
}

void IconFreshThread::releaseThread()
{
    releaseSemo();
    m_loadAllIconState.store(false);
    m_runningState.store(false);
    wait(2000);
}

void IconFreshThread::run()
{
    /* 1. 处理应用启动时载入系统所有应用图标
     * 上次退出应用后的状态，进程启动时就加载全屏模式, 全屏模式按下Ctrl和鼠标滚轮缩放的五种比率以及小窗口模式的应用信息。
     * 系统中有缩放的情况也要考虑在其中,也就是说缩放的范围如果确定了,那么图片的大小也就随之确定了.
    */

    // 系统全屏模式下缩放时的五类屏幕像素比数据
    double ratioArray[5] = {0.2, 0.3, 0.4, 0.5, 0.6};

    // 各类模式下图标大小值
    int appSize = 0;

    while (m_loadAllIconState.load()) {
        // 小窗口模式,item的真实大小有默认值QSize(24, 24)
        for (int i = AppsListModel::All; i < AppsListModel::Others; i++) {
            const ItemInfoList &itemList = AppsManager::getAllAppInfo().value((AppsListModel::AppCategory)(i));

            for (int j = 0; j < itemList.size(); j++) {
                const ItemInfo &info = itemList.at(j);
                for (int k = 0; k < 5; k++) {
                    m_iconValid.store(false);

                    // 多种类型尺寸的图标，appslistmodel::data()接口中可以看到
                    const qreal ratio =  ratioArray[k];
                    int iconSize = CalculateUtil::instance()->appIconSize(false, ratio).width();
                    int dlgIconSize = 36 * ratio;
                    int dragIconSize = CalculateUtil::instance()->appIconSize(false, ratio, 0).width() * 1.2;
                    int listIconSize = 18 * ratio;

                    // 生成各个场景下的应用缓存数据
                    m_iconValid.store(false);
                    createPixmap(info, (iconSize));

                    m_iconValid.store(false);
                    createPixmap(info, (dlgIconSize));

                    m_iconValid.store(false);
                    createPixmap(info, (dragIconSize));

                    m_iconValid.store(false);
                    createPixmap(info, (listIconSize));

                    // 程序出现异常,加载缓存的过程中就出现了crash,加上可以直接退出线程
                    if (!m_loadAllIconState.load())
                        return;
                }
            }
        }

        // 全屏自由模式,重新计算item的真实大小
        appSize = CalculateUtil::instance()->calculateIconSize(0);
        int nSize = AppsManager::appsInfoListSize(AppsListModel::All);

        for (int i = 0; i < nSize; i++) {
            const ItemInfo &info = AppsManager::appsInfoListIndex((AppsListModel::AppCategory)0, i);

            for (int k = 0; k < 5; k++) {
                // 多种类型尺寸的图标，appslistmodel::data()接口中可以看到
                const qreal ratio =  ratioArray[k];
                int iconWidth= CalculateUtil::instance()->appIconSize(true, ratio, appSize).width();
                int dlgIconSize = 36 * ratio;
                int dragIconSize = CalculateUtil::instance()->appIconSize(true, ratio, appSize).width() * 1.2;
                int listIconSize = 18 * ratio;

                // 生成各个场景下的应用缓存数据
                m_iconValid.store(false);
                createPixmap(info, (iconWidth));

                m_iconValid.store(false);
                createPixmap(info, (listIconSize));

                m_iconValid.store(false);
                createPixmap(info, (dlgIconSize));

                m_iconValid.store(false);
                createPixmap(info, (dragIconSize));

                // 程序出现异常,加载缓存的过程中就出现了crash,加上可以直接退出线程
                if (!m_loadAllIconState.load())
                    return;
            }
        }

        // 全屏分类模式,重新计算item的真实大小
        appSize = CalculateUtil::instance()->calculateIconSize(1);

        for (int i = 0; i < nSize; i++) {
            const ItemInfo &info = AppsManager::appsInfoListIndex((AppsListModel::AppCategory)0, i);

            for (int k = 0; k < 5; k++) {
                // 多种类型尺寸的图标，appslistmodel::data()接口中可以看到
                const qreal ratio =  ratioArray[k];
                int iconWidth= CalculateUtil::instance()->appIconSize(true, ratio, appSize).width();
                int dlgIconSize = 36 * ratio;
                int dragIconSize = CalculateUtil::instance()->appIconSize(true, ratio, appSize).width() * 1.2;
                int listIconSize = 18 * ratio;

                // 生成各个场景下的应用缓存数据
                m_iconValid.store(false);
                createPixmap(info, (iconWidth));

                m_iconValid.store(false);
                createPixmap(info, (listIconSize));

                m_iconValid.store(false);
                createPixmap(info, (dlgIconSize));

                m_iconValid.store(false);
                createPixmap(info, (dragIconSize));

                // 程序出现异常,加载缓存的过程中就出现了crash,加上可以直接退出线程
                if (!m_loadAllIconState.load())
                    return;
            }
        }

        m_loadAllIconState.store(false);
        break;
    }

    // 2.处理应用安装时的图标加载
    while (m_runningState.load()) {
        m_installAppListLock.lockForWrite();

        while (m_InstallAppInfo.size()) {
            const ItemInfo &info = m_InstallAppInfo.dequeue();

            for (int k = 0; k < 5; k++) {
                const qreal ratio =  ratioArray[k];
                int iconSize = CalculateUtil::instance()->appIconSize().width();
                int dlgIconSize = 36 * ratio;
                int dragIconSize = CalculateUtil::instance()->appIconSize().width() * 1.2;
                int listIconSize = 18 * ratio;

                // 生成各个场景下的应用缓存数据
                m_iconValid.store(false);
                createPixmap(info, (iconSize));

                m_iconValid.store(false);
                createPixmap(info, (dlgIconSize));

                m_iconValid.store(false);
                createPixmap(info, (dragIconSize));

                m_iconValid.store(false);
                createPixmap(info, (listIconSize));
            }

            // 程序出现异常,加载缓存的过程中就出现了crash,加上可以直接退出线程
            if (!m_runningState) {
                m_installAppListLock.unlock();
                return;
            }
        }

        m_installAppListLock.unlock();

        if (!m_runningState)
            return;

        // 等待数据到来...
        semaphore.acquire(6);
    }
}
