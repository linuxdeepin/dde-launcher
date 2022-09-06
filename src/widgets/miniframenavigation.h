// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIFRAMENAVIGATION_H
#define MINIFRAMENAVIGATION_H

#include <QWidget>
#include <QPushButton>
#include <QStackedLayout>
#include <QStandardPaths>

#include <diconbutton.h>

#include <com_deepin_daemon_accounts.h>
#include <com_deepin_daemon_accounts_user.h>

DWIDGET_USE_NAMESPACE

class UserButton : public DIconButton
{
    Q_OBJECT

public:
    explicit UserButton(QWidget *parent = Q_NULLPTR);

private:
    com::deepin::daemon::Accounts *m_accounts;
    com::deepin::daemon::accounts::User *m_user;

    void initUser();
    void setUserPath(const QString &path);
    void setUserIconURL(const QString &iconUrl);
};

class NavigationButton : public QPushButton
{
    Q_OBJECT

public:
    explicit NavigationButton(const QString &title, QWidget *parent = Q_NULLPTR);
};

class MiniFrameNavigation : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameNavigation(QWidget *parent = nullptr);

private slots:
    void openDirectory(const QString &dir);
    void openStandardDirectory(const QStandardPaths::StandardLocation &location);
    void handleShutdownAction(const QString &action);
    void handleLockAction();

private:
    UserButton *m_avatar;

    NavigationButton *m_computer;
    NavigationButton *m_document;
    NavigationButton *m_video;
    NavigationButton *m_music;
    NavigationButton *m_picture;
    NavigationButton *m_download;
    NavigationButton *m_toShutdown;

    NavigationButton *m_lock;
    NavigationButton *m_logout;
    NavigationButton *m_reboot;
    NavigationButton *m_suspend;
    NavigationButton *m_shutdown;
    NavigationButton *m_toNavigation;

    QWidget *m_navigationPanel;
    QWidget *m_shutdownPanel;

    QStackedLayout *m_panelStack;
};

#endif // MINIFRAMENAVIGATION_H
