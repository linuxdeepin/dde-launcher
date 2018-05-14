/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * Maintainer: rekols <rekols@foxmail.com>
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

#include "miniframerightbar.h"
#include "miniframebutton.h"
#include "avatar.h"
#include <DDesktopServices>
#include <DDBusSender>
#include <QVBoxLayout>
#include <QPainter>

MiniFrameRightBar::MiniFrameRightBar(QWidget *parent)
    : QWidget(parent),
      m_modeToggleBtn(new DImageButton),
      m_avatar(new Avatar),
      m_datetimeWidget(new DatetimeWidget)
{
    m_modeToggleBtn->setNormalPic(":/icons/skin/icons/fullscreen_normal.png");
    m_modeToggleBtn->setHoverPic(":/icons/skin/icons/fullscreen_hover.png");
    m_modeToggleBtn->setPressPic(":/icons/skin/icons/fullscreen_press.png");

    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    MiniFrameButton *computerBtn = new MiniFrameButton(tr("Computer"));
    MiniFrameButton *documentBtn = new MiniFrameButton(tr("Document"));
    MiniFrameButton *videoBtn = new MiniFrameButton(tr("Video"));
    MiniFrameButton *musicBtn = new MiniFrameButton(tr("Music"));
    MiniFrameButton *pictureBtn = new MiniFrameButton(tr("Picture"));
    MiniFrameButton *downloadBtn = new MiniFrameButton(tr("Downloads"));
    MiniFrameButton *manualBtn = new MiniFrameButton(tr("Manual"));
    MiniFrameButton *settingsBtn = new MiniFrameButton(tr("Settings"));
    MiniFrameButton *shutdownBtn = new MiniFrameButton(tr("Shutdown"));

    settingsBtn->setIcon(QIcon(":/widgets/images/settings.svg"));
    shutdownBtn->setIcon(QIcon(":/widgets/images/power.svg"));

    settingsBtn->setStyleSheet(settingsBtn->styleSheet() + "font-size: 15px;");
    shutdownBtn->setStyleSheet(settingsBtn->styleSheet() + "font-size: 15px;");

    bottomLayout->addWidget(settingsBtn);
    bottomLayout->addWidget(shutdownBtn);

    layout->addWidget(m_modeToggleBtn, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addWidget(m_avatar);
    layout->addSpacing(30);
    layout->addWidget(computerBtn);
    layout->addWidget(documentBtn);
    layout->addWidget(videoBtn);
    layout->addWidget(musicBtn);
    layout->addWidget(pictureBtn);
    layout->addWidget(downloadBtn);
    layout->addWidget(manualBtn);
    layout->addSpacing(36);
    layout->addWidget(m_datetimeWidget);
    layout->addStretch();
    layout->addLayout(bottomLayout);
    layout->setContentsMargins(20, 12, 12, 22);

    setFixedWidth(240);

    connect(m_modeToggleBtn, &DImageButton::clicked, this, &MiniFrameRightBar::modeToggleBtnClicked);

    connect(computerBtn, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    connect(documentBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    connect(videoBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    connect(musicBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    connect(pictureBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    connect(downloadBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });
    connect(manualBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showManual);
    connect(settingsBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showSettings);
    connect(shutdownBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showShutdown);
    connect(m_avatar, &Avatar::clicked, this, &MiniFrameRightBar::handleAvatarClicked);
    connect(m_datetimeWidget, &DatetimeWidget::clicked, this, &MiniFrameRightBar::handleTimedateOpen);
}

MiniFrameRightBar::~MiniFrameRightBar()
{
}

void MiniFrameRightBar::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setPen(QColor(255, 255, 255, 0.1 * 255));
    painter.drawLine(QPoint(0, 0),
                     QPoint(0, rect().height()));
}

void MiniFrameRightBar::openDirectory(const QString &dir)
{
    DDesktopServices::showFolder(QUrl(dir));

    emit requestFrameHide();
}

void MiniFrameRightBar::openStandardDirectory(const QStandardPaths::StandardLocation &location)
{
    const QString dir = QStandardPaths::writableLocation(location);

    if (!dir.isEmpty())
        openDirectory(dir);
}

void MiniFrameRightBar::handleShutdownAction(const QString &action)
{
    DDBusSender()
            .service("com.deepin.dde.shutdownFront")
            .interface("com.deepin.dde.shutdownFront")
            .path("/com/deepin/dde/shutdownFront")
            .method(action)
            .call();

    emit requestFrameHide();
}

void MiniFrameRightBar::handleTimedateOpen()
{
    DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QStringLiteral("ShowModule"))
            .arg(QStringLiteral("datetime"))
            .call();

    emit requestFrameHide();
}

void MiniFrameRightBar::handleAvatarClicked()
{
    DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QStringLiteral("ShowModule"))
            .arg(QStringLiteral("accounts"))
            .call();

    emit requestFrameHide();
}

void MiniFrameRightBar::showShutdown()
{
    QProcess::startDetached("dde-shutdown");
}

void MiniFrameRightBar::showSettings()
{
    DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QString("Toggle"))
            .call();

    emit requestFrameHide();
}

void MiniFrameRightBar::showManual()
{
    QProcess::startDetached("dman");

    emit requestFrameHide();
}
