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

#include "miniframenavigation.h"

#include <QVBoxLayout>
#include <QProcess>
#include <QSignalMapper>
#include <QPainter>
#include <QPainterPath>

#include <DDesktopServices>

#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

#include <unistd.h>

DWIDGET_USE_NAMESPACE
#ifdef DUTIL_NAMESPACE
DUTIL_USE_NAMESPACE
#endif

UserButton::UserButton(QWidget *parent) :
    DIconButton(parent),
    m_accounts(new com::deepin::daemon::Accounts("com.deepin.daemon.Accounts",
                                                 "/com/deepin/daemon/Accounts",
                                                 QDBusConnection::systemBus(), this)),
    m_user(Q_NULLPTR)
{
    setFixedSize(60, 60);

    initUser();
}

void UserButton::initUser()
{
    setUserPath(QString("/com/deepin/daemon/Accounts/User%1").arg(getuid()));
}

void UserButton::setUserPath(const QString &path)
{
    if (m_user)
        m_user->deleteLater();

    m_user = new com::deepin::daemon::accounts::User("com.deepin.daemon.Accounts",
                                                     path,
                                                     QDBusConnection::systemBus(), this);
    m_user->setSync(false);
    setUserIconURL(m_user->iconFile());
    connect(m_user, &__User::IconFileChanged, this, &UserButton::setUserIconURL);
}

void UserButton::setUserIconURL(const QString &iconUrl)
{
    if (iconUrl.isEmpty())
        return;

    const QString path = QUrl(iconUrl).toLocalFile();

    setIcon(QIcon(path));
}

NavigationButton::NavigationButton(const QString &title, QWidget *parent) :
    QPushButton(title, parent)
{
}


MiniFrameNavigation::MiniFrameNavigation(QWidget *parent)
    : QWidget(parent)
{
    m_avatar = new UserButton;
    m_avatar->setFixedSize(60, 60);

    m_computer = new NavigationButton(tr("Computer"));
    m_document = new NavigationButton(tr("Documents"));
    m_video = new NavigationButton(tr("Videos"));
    m_music = new NavigationButton(tr("Music"));
    m_picture = new NavigationButton(tr("Pictures"));
    m_download = new NavigationButton(tr("Downloads"));
    m_toShutdown = new NavigationButton(tr("Shutdown"));

    QVBoxLayout *navigationLayout = new QVBoxLayout;
    navigationLayout->addWidget(m_computer);
    navigationLayout->addWidget(m_document);
    navigationLayout->addWidget(m_video);
    navigationLayout->addWidget(m_music);
    navigationLayout->addWidget(m_picture);
    navigationLayout->addWidget(m_download);
    navigationLayout->addStretch();
    navigationLayout->addWidget(m_toShutdown);

    m_navigationPanel = new QWidget;
    m_navigationPanel->setLayout(navigationLayout);

    m_lock = new NavigationButton(tr("Lock"));
    m_logout = new NavigationButton(tr("Logout"));
    m_reboot = new NavigationButton(tr("Reboot"));
    m_suspend = new NavigationButton(tr("Suspend"));
    m_shutdown = new NavigationButton(tr("Shutdown"));
    m_toNavigation = new NavigationButton(tr("Back"));

    QVBoxLayout *shutdownLayout = new QVBoxLayout;
    shutdownLayout->addWidget(m_lock);
    shutdownLayout->addWidget(m_logout);
    shutdownLayout->addWidget(m_reboot);
    shutdownLayout->addWidget(m_suspend);
    shutdownLayout->addWidget(m_shutdown);
    shutdownLayout->addStretch();
    shutdownLayout->addWidget(m_toNavigation);

    m_shutdownPanel = new QWidget;
    m_shutdownPanel->setLayout(shutdownLayout);

    m_panelStack = new QStackedLayout;
    m_panelStack->addWidget(m_navigationPanel);
    m_panelStack->addWidget(m_shutdownPanel);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_avatar);
    centralLayout->addSpacing(35);
    centralLayout->setAlignment(m_avatar, Qt::AlignHCenter);
    centralLayout->addLayout(m_panelStack);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 50, 0, 0);

    setLayout(centralLayout);

    connect(m_computer, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    connect(m_document, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    connect(m_video, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    connect(m_music, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    connect(m_picture, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    connect(m_download, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });

    connect(m_toShutdown, &QPushButton::clicked, this, [this] { m_panelStack->setCurrentWidget(m_shutdownPanel); });
    connect(m_toNavigation, &QPushButton::clicked, this, [this] { m_panelStack->setCurrentWidget(m_navigationPanel); });

    QSignalMapper *signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(m_logout, "Logout");
    signalMapper->setMapping(m_suspend, "Suspend");
    signalMapper->setMapping(m_shutdown, "Shutdown");
    signalMapper->setMapping(m_reboot, "Restart");
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(handleShutdownAction(QString)));

    connect(m_logout, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_suspend, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_shutdown, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_reboot, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_lock, SIGNAL(clicked()), this, SLOT(handleLockAction()));
}

void MiniFrameNavigation::openDirectory(const QString &dir)
{
    DDesktopServices::showFolder(dir);
}

void MiniFrameNavigation::openStandardDirectory(const QStandardPaths::StandardLocation &location)
{
    const QString dir = QStandardPaths::writableLocation(location);
    if (!dir.isEmpty())
        openDirectory(dir);
}

void MiniFrameNavigation::handleShutdownAction(const QString &action)
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.shutdownFront")
            .interface("com.deepin.dde.shutdownFront")
            .path("/com/deepin/dde/shutdownFront")
            .method(action)
            .call();
#else
    const QString command = QString("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront " \
                                    "/com/deepin/dde/shutdownFront " \
                                    "com.deepin.dde.shutdownFront.%1").arg(action);

    QProcess::startDetached(command);
#endif
}

void MiniFrameNavigation::handleLockAction()
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.lockFront")
            .interface("com.deepin.dde.lockFront")
            .path("/com/deepin/dde/lockFront")
            .method(QString())
            .call();
#else
    const QString command = QString("dbus-send --print-reply --dest=com.deepin.dde.lockFront " \
                                    "/com/deepin/dde/lockFront " \
                                    "com.deepin.dde.lockFront");

    QProcess::startDetached(command);
#endif
}
