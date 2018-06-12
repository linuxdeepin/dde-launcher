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

#ifndef MINIFRAMENAVIGATION_H
#define MINIFRAMENAVIGATION_H

#include <QWidget>
#include <QPushButton>
#include <QStackedLayout>
#include <QStandardPaths>

#include <dimagebutton.h>

#include <com_deepin_daemon_accounts.h>
#include <com_deepin_daemon_accounts_user.h>

DWIDGET_USE_NAMESPACE

class UserButton : public DImageButton
{
    Q_OBJECT

public:
    explicit UserButton(QWidget *parent = Q_NULLPTR);

private:
    void paintEvent(QPaintEvent *e);

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
    explicit MiniFrameNavigation(QWidget *parent = 0);

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
