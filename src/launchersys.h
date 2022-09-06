// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LAUNCHERSYS_H
#define LAUNCHERSYS_H

#include "dbusinterface/dbuslauncher.h"
#include "calculate_util.h"

#include <DRegionMonitor>

#include <QObject>
#include <QTimer>

#include <com_deepin_sessionmanager.h>

DGUI_USE_NAMESPACE

class LauncherInterface;
class WindowedFrame;
class FullScreenFrame;

class LauncherSys : public QObject
{
    Q_OBJECT

public:
    explicit LauncherSys(QObject *parent = nullptr);
    ~LauncherSys() override;

    bool visible();
    void showLauncher();
    void hideLauncher();
    void uninstallApp(const QString &appKey);
    void setClickState(bool state);
    bool clickState() const;
    void aboutToShowLauncher();
    void show();

signals:
    void visibleChanged(bool visible);

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void displayModeChanged();
    void onAutoExitTimeout();
    void onVisibleChanged();
    void onDisplayModeChanged();
    void onFrontendRectChanged();
    void onButtonPress(const QPoint &p, const int flag);
    void updateLauncher();

private:
    void registerRegion();
    void unRegisterRegion();
    void preloadIcon();

private:
    AppsManager *m_appManager;
    LauncherInterface *m_launcherInter;                     // 启动器界面处理基类
    DBusLauncher *m_dbusLauncherInter;                      // dbus访问远程服务类 数据初始化
    com::deepin::SessionManager *m_sessionManagerInter;     // dbus访问远程服务类 业务逻辑处理

    WindowedFrame *m_windowLauncher;                        // 启动器小窗口界面处理类
    FullScreenFrame *m_fullLauncher;                        // 启动器全屏界面处理类
    DRegionMonitor *m_regionMonitor;                        // deepin tool kit中core模块的内容
    QTimer *m_autoExitTimer;
    QTimer *m_ignoreRepeatVisibleChangeTimer;               // 添加200ms延时操作，避开重复显示、隐藏启动器
    QMetaObject::Connection m_regionMonitorConnect;         // 信号和槽连接返回的对象
    CalculateUtil *m_calcUtil;                              // 界面布局计算处理类
    DBusDock *m_dockInter;
    bool m_clicked;                                         // 人的点击操作状态
};

#endif // LAUNCHERSYS_H
