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

#include "miniframebottombar.h"

#include <DDesktopServices>

#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

DWIDGET_USE_NAMESPACE
#ifdef DUTIL_NAMESPACE
DUTIL_USE_NAMESPACE
#endif

MiniFrameBottomBar::MiniFrameBottomBar(QWidget *parent)
    : QWidget(parent)
{
    m_computer = new NavigationButton(tr("Computer"));
    m_document = new NavigationButton(tr("Documents"));
    m_video = new NavigationButton(tr("Videos"));
    m_music = new NavigationButton(tr("Music"));
    m_picture = new NavigationButton(tr("Pictures"));
    m_download = new NavigationButton(tr("Downloads"));
    m_shutdownBtn = new DImageButton;
    m_shutdownBtn->setNormalPic(":/icons/skin/icons/poweroff_normal.png");
    m_shutdownBtn->setHoverPic(":/icons/skin/icons/poweroff_hover.png");
    m_shutdownBtn->setPressPic(":/icons/skin/icons/poweroff_press.png");
#ifdef SYS_INFO_BUTTON
    m_sysInfoBtn = new QPushButton("系统信息");
#endif

    QHBoxLayout *navigationLayout = new QHBoxLayout;
    navigationLayout->addStretch();
    navigationLayout->addWidget(m_computer);
    navigationLayout->addWidget(m_document);
    navigationLayout->addWidget(m_video);
    navigationLayout->addWidget(m_music);
    navigationLayout->addWidget(m_picture);
    navigationLayout->addWidget(m_download);
    navigationLayout->addStretch();
#ifdef SYS_INFO_BUTTON
    navigationLayout->addWidget(m_sysInfoBtn);
#endif
    navigationLayout->addWidget(m_shutdownBtn);
    navigationLayout->setMargin(0);

    m_navigationPanel = new QWidget;
    m_navigationPanel->setLayout(navigationLayout);

    m_lock = new NavigationButton(tr("Lock"));
    m_logout = new NavigationButton(tr("Logout"));
    m_reboot = new NavigationButton(tr("Reboot"));
    m_suspend = new NavigationButton(tr("Suspend"));
    m_shutdown = new NavigationButton(tr("Shutdown"));
    m_toNavigation = new NavigationButton(tr("Back"));

    QHBoxLayout *shutdownLayout = new QHBoxLayout;
    shutdownLayout->addStretch();
    shutdownLayout->addWidget(m_shutdown);
    shutdownLayout->addWidget(m_reboot);
    shutdownLayout->addWidget(m_suspend);
    shutdownLayout->addWidget(m_lock);
    shutdownLayout->addWidget(m_logout);
    shutdownLayout->addStretch();
    shutdownLayout->addWidget(m_toNavigation);
    shutdownLayout->setMargin(0);

    m_shutdownPanel = new QWidget;
    m_shutdownPanel->setLayout(shutdownLayout);

    m_panelStack = new QStackedLayout;
    m_panelStack->addWidget(m_navigationPanel);
    m_panelStack->addWidget(m_shutdownPanel);

    setLayout(m_panelStack);

    connect(m_computer, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    connect(m_document, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    connect(m_video, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    connect(m_music, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    connect(m_picture, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    connect(m_download, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });

    connect(m_shutdownBtn, &DImageButton::clicked, this, &MiniFrameBottomBar::showShutdown);
#ifdef SYS_INFO_BUTTON
    connect(m_sysInfoBtn, &QPushButton::clicked, this, &MiniFrameBottomBar::showSysInfo);
#endif
//    connect(m_shutdownBtn, &DImageButton::clicked, this, [this] { m_panelStack->setCurrentWidget(m_shutdownPanel); });
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

void MiniFrameBottomBar::openDirectory(const QString &dir)
{
    DDesktopServices::showFolder(QUrl(dir));

    emit requestFrameHide();
}

void MiniFrameBottomBar::openStandardDirectory(const QStandardPaths::StandardLocation &location)
{
    const QString dir = QStandardPaths::writableLocation(location);
    if (!dir.isEmpty())
        openDirectory(dir);
}

void MiniFrameBottomBar::handleShutdownAction(const QString &action)
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

void MiniFrameBottomBar::handleLockAction()
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.lockFront")
            .interface("com.deepin.dde.lockFront")
            .path("/com/deepin/dde/lockFront")
            .method("Show")
            .call();
#else
    const QString command = QString("dbus-send --print-reply --dest=com.deepin.dde.lockFront " \
                                    "/com/deepin/dde/lockFront " \
                                    "com.deepin.dde.lockFront.Show");

    QProcess::startDetached(command);
#endif
}

void MiniFrameBottomBar::showShutdown()
{
    QProcess::startDetached("dde-shutdown");
}

void MiniFrameBottomBar::showSysInfo()
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method("ShowPage")
            .arg(QString("systeminfo"))
            .call();
#else
    const QString command = QString("dbus-send "
                                    "--type=method_call "
                                    "--dest=com.deepin.dde.ControlCenter "
                                    "/com/deepin/dde/ControlCenter "
                                    "com.deepin.dde.ControlCenter.ShowModule "
                                    "string:systeminfo");

    QProcess::startDetached(command);
#endif
}
