#ifndef APPICONFRESHTHREAD_H
#define APPICONFRESHTHREAD_H

#include "iteminfo.h"

#include <QThread>
#include <QQueue>
#include <QSemaphore>

class AppIconFreshThread : public QThread
{
    Q_OBJECT

public:
    AppIconFreshThread(QObject *parent = nullptr);

    void setQueue(QQueue<ItemInfo>& info);
    void pushPixmap(const ItemInfo &itemInfo);
    void releaseSemo();

protected:
    virtual void run();

signals:
    void appInfoFresh(const ItemInfo &info);

public slots:
    void refreshIcon();
    void releaseThread();

private:
    int m_tryNums;                                       // 获取应用图标时尝试的次数
    ItemInfo m_itemInfo;                                 // 当前需要更新的应用信息
    QQueue<ItemInfo> m_InstallAppInfo;                   // 所有应用信息
    std::atomic<bool> m_runningState;                    // 没有事件循环的类，无法直接掉接口停止，因此设置线程运行标识位
};
#endif
