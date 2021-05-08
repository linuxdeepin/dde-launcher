#include "AppIconFreshThread.h"
#include "util.h"
#include "appsmanager.h"

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

DWIDGET_USE_NAMESPACE

static QSemaphore semaphore(5);

/**
 * @brief AppIconFreshThread::AppIconFreshThread 给应用从系统主题中查找图标资源
 * @param parent 父对象
 */
AppIconFreshThread::AppIconFreshThread(QObject *parent) :
    QThread (parent),
    m_tryNums(0),
    m_itemInfo(ItemInfo()),
    m_runningState(true)
{
    m_InstallAppInfo.clear();
}

void AppIconFreshThread::pushPixmap(const ItemInfo &itemInfo)
{
    const int arraySize = 8;
    const int iconArray[arraySize] = { 16, 18, 24, 32, 64, 96, 128, 256 };

    for (int i = 0; i < arraySize; i++) {
        QPair<QString, int> tmpKey { cacheKey(itemInfo, CacheType::ImageType), iconArray[i]};
        if (AppsManager::m_CacheData[tmpKey].isNull()) {
            QPixmap pixmap;

            bool iconValid = getThemeIcon(pixmap, itemInfo, iconArray[i], true);

            if (!iconValid) {
                if (m_tryNums < 10) {
                    ++m_tryNums;

                    // 当无法找到文件时，看下文件是否存在
                    if (!QFile::exists(itemInfo.m_iconKey))
                        // 当程序已从缓存中获取图标，应用程序才安装完毕，此时需要强行对缓存中图标的是否为空，进行验证
                        QIcon::setThemeSearchPaths(QIcon::themeSearchPaths());

                    m_itemInfo = itemInfo;

                    // 10秒从缓存中取一次
                    QTimer::singleShot(10 * 1000, this, [=]() { refreshIcon(); });
                    break;
                } else {
                    m_itemInfo = itemInfo;

                    // 如果图标获取失败，每隔1分钟(1 * 60 * 1000)刷新一次
                    QTimer::singleShot(60 * 1000, this, [=]() { refreshIcon(); });
                    break;
                }
            } else {
                if (m_tryNums > 0)
                    m_tryNums = 0;

                break;
            }
        }
    }
}

void AppIconFreshThread::releaseSemo()
{
    semaphore.release(6);
}

void AppIconFreshThread::setQueue(QQueue<ItemInfo> &info)
{
    m_InstallAppInfo.append(info);
}

void AppIconFreshThread::refreshIcon()
{
    pushPixmap(m_itemInfo);
}

void AppIconFreshThread::releaseThread()
{
    m_runningState.store(false);
    releaseSemo();
}

void AppIconFreshThread::run()
{
    // 系统退出时清理资源，没有使用事件循环的线程类无法调用quit释放资源
    while (m_runningState.load()) {
        while (m_InstallAppInfo.size()) {
            const ItemInfo &info = m_InstallAppInfo.dequeue();
            // 从系统主题中查找，走定时器继续找，找到后，加入缓存
            pushPixmap(info);
        }

        // 等待数据到来...
        semaphore.acquire(6);
    }
}


