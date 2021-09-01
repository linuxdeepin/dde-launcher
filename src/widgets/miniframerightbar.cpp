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
#include "util.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>

#include <DDesktopServices>
#include <DDBusSender>
#include <QtConcurrent>

/**
 * @brief MiniFrameRightBar::MiniFrameRightBar 启动器小窗口模式最左侧按钮组
 * @param parent
 */
MiniFrameRightBar::MiniFrameRightBar(QWidget *parent)
    : QWidget(parent)
    , m_avatar(new Avatar(this))
    , m_currentIndex(0)
    , m_hideList(SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "mini-frame-right-bar-hide-list", QStringList()).toStringList())
    , m_hasCompterIcon(false)
    , m_hasDocumentIcon(false)
    , m_hasPictureIcon(false)
    , m_hasMusicIcon(false)
    , m_hasVideoIcon(false)
    , m_hasDownloadIcon(false)
{
    setFixedWidth(60);
    QSize m_size(36, 36);
//    m_modeToggleBtn->raise();
    // 测试
    m_avatar->setAccessibleName("useravatat");

    QVBoxLayout *    layout       = new QVBoxLayout(this);
    QVBoxLayout *    bottomLayout = new QVBoxLayout;

    if (!m_hideList.contains("Computer")) {
        m_computerBtn  = new MiniFrameButton(tr("Computer"));
        m_computerBtn->setFixedSize(m_size);
        m_computerBtn->setAccessibleName("computerbtn");
        m_hasCompterIcon = true;
    }

    if (!m_hideList.contains("Videos")) {
        m_videoBtn = new MiniFrameButton(tr("Videos"));
        m_videoBtn->setFixedSize(m_size);
        m_videoBtn->setAccessibleName("videobtn");
        m_hasVideoIcon = true;
    }

    if (!m_hideList.contains("Music")) {
        m_musicBtn = new MiniFrameButton(tr("Music"));
        m_musicBtn->setFixedSize(m_size);
        m_musicBtn->setAccessibleName("musicbtn");
        m_hasMusicIcon = true;
    }

    if (!m_hideList.contains("Pictures")) {
        m_pictureBtn = new MiniFrameButton(tr("Pictures"));
        m_pictureBtn->setFixedSize(m_size);
        m_pictureBtn->setAccessibleName("picturebtn");
        m_hasPictureIcon = true;
    }

    if (!m_hideList.contains("Documents")) {
        m_documentBtn = new MiniFrameButton(tr("Documents"));
        m_documentBtn->setFixedSize(m_size);
        m_documentBtn->setAccessibleName("documentbtn");
        m_hasDocumentIcon = true;
    }

    if (!m_hideList.contains("Downloads")) {
        m_downloadBtn = new MiniFrameButton(tr("Downloads"));
        m_downloadBtn->setFixedSize(m_size);
        m_downloadBtn->setAccessibleName("downloadbtn");
        m_hasDownloadIcon = true;
    }

    m_settingsBtn = new MiniFrameButton(tr("Control Center"));
    m_settingsBtn->setFixedSize(m_size);
    m_settingsBtn->setAccessibleName("settingsbtn");
    m_powerBtn = new MiniFrameButton(tr("Power"));
    m_powerBtn->setFixedSize(m_size);
    m_powerBtn->setIconSize(QSize(20, 20));
    m_powerBtn->setAccessibleName("powerbtn");

    uint index = 0;
    if (m_hasCompterIcon)
        m_btns[index++] = m_computerBtn;
    if (m_hasDocumentIcon)
        m_btns[index++] = m_documentBtn;
    if (m_hasPictureIcon)
        m_btns[index++] = m_pictureBtn;
    if (m_hasMusicIcon)
        m_btns[index++] = m_musicBtn;
    if (m_hasVideoIcon)
        m_btns[index++] = m_videoBtn;
    if (m_hasDownloadIcon)
        m_btns[index++] = m_downloadBtn;
    m_btns[index++] = m_settingsBtn;
    m_btns[index++] = m_powerBtn;

    for (auto it = m_btns.constBegin(); it != m_btns.constEnd(); ++it) {
        it.value()->setCheckable(true);
        connect(it.value(), &MiniFrameButton::entered, this, [=] {
            hideAllHoverState();
            m_currentIndex = m_btns.key(it.value());
            setCurrentCheck(true);
        }, Qt::QueuedConnection);
    }

    updateIcon();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MiniFrameRightBar::updateIcon);

    bottomLayout->addWidget(m_settingsBtn, 0, Qt::AlignCenter);
    bottomLayout->addWidget(m_powerBtn,0 ,Qt::AlignCenter);
    bottomLayout->setSpacing(2);

    QWidget *center_widget = new QWidget;
    QVBoxLayout *center_layout = new QVBoxLayout;
    center_widget->setAccessibleName("centerwidget");
    center_layout->setMargin(0);
    center_widget->setLayout(center_layout);
    center_layout->setSpacing(10);
    center_layout->addWidget(m_avatar, 0, Qt::AlignCenter);
    center_layout->addSpacing(10);

    if (m_hasCompterIcon)
        center_layout->addWidget(m_computerBtn, 0, Qt::AlignCenter);
    if (m_hasDocumentIcon)
        center_layout->addWidget(m_documentBtn, 0, Qt::AlignCenter);
    if (m_hasPictureIcon)
        center_layout->addWidget(m_pictureBtn, 0, Qt::AlignCenter);
    if (m_hasMusicIcon)
        center_layout->addWidget(m_musicBtn, 0, Qt::AlignCenter);
    if (m_hasVideoIcon)
        center_layout->addWidget(m_videoBtn, 0, Qt::AlignCenter);
    if (m_hasDownloadIcon)
        center_layout->addWidget(m_downloadBtn, 0, Qt::AlignCenter);

    center_layout->addStretch();

    QWidget *bottom_widget = new QWidget;
    bottom_widget->setAccessibleName("bottonwidget");
    QVBoxLayout *bottom_layout = new QVBoxLayout;
    bottom_layout->setMargin(0);
    bottom_layout->addLayout(bottomLayout);
    bottom_widget->setLayout(bottom_layout);

    layout->addSpacing(13);
    layout->addWidget(center_widget);
    layout->addWidget(bottom_widget, 0, Qt::AlignBottom);
    layout->setContentsMargins(0, 0, 0, 6);

    if (m_hasCompterIcon)
        connect(m_computerBtn, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    if (m_hasDocumentIcon)
        connect(m_documentBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    if (m_hasVideoIcon)
        connect(m_videoBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    if (m_hasMusicIcon)
        connect(m_musicBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    if (m_hasPictureIcon)
        connect(m_pictureBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    if (m_hasDownloadIcon)
        connect(m_downloadBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });

    connect(m_settingsBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showSettings);
    connect(m_powerBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showShutdown);
    connect(m_avatar, &Avatar::clicked, this, &MiniFrameRightBar::handleAvatarClicked);
}

void MiniFrameRightBar::setCurrentCheck(bool checked) const
{
    m_btns[m_currentIndex]->setChecked(checked);
}

void MiniFrameRightBar::moveUp()
{
    hideAllHoverState();

    // move to bottom
    if (--m_currentIndex < 0) {
        m_currentIndex = m_btns.size() - 1;
        m_btns.last()->setChecked(true);
        return;
    }

    setCurrentCheck(true);
}

void MiniFrameRightBar::moveDown()
{
    hideAllHoverState();

    // move to head
    if (++m_currentIndex > m_btns.size() - 1) {
        m_currentIndex = 0;
        m_btns.first()->setChecked(true);
        return;
    }

    setCurrentCheck(true);
}

void MiniFrameRightBar::execCurrent()
{
    emit m_btns[m_currentIndex]->clicked();
}

void MiniFrameRightBar::showEvent(QShowEvent *event) {
    return QWidget::showEvent(event);
}

void MiniFrameRightBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    // 初始化绘制工具
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景颜色
    QColor backgroundColor;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        backgroundColor = QColor(0, 0, 0); // 浅色模式为#000000 透明度0.1%
        backgroundColor.setAlphaF(0.1);
    } else {
        backgroundColor = QColor(255, 255, 255); // 深色模式为#FFFFFF 透明度0.08%
        backgroundColor.setAlphaF(0.08);
    }
    painter.fillRect(rect(), backgroundColor);
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
    DDBusSender()
            .service("com.deepin.dde.shutdownFront")
            .interface("com.deepin.dde.shutdownFront")
            .path("/com/deepin/dde/shutdownFront")
            .method("Show")
            .call();

    emit requestFrameHide();
}

void MiniFrameRightBar::showSettings()
{
    QtConcurrent::run([] {
        DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QString("Toggle"))
            .call();
    });

    emit requestFrameHide();
}

void MiniFrameRightBar::showManual()
{
    QProcess::startDetached("dman");

    emit requestFrameHide();
}

void MiniFrameRightBar::hideAllHoverState() const
{
    for (auto it = m_btns.constBegin(); it != m_btns.constEnd(); ++it) {
        it.value()->setChecked(false);
    }
}

void MiniFrameRightBar::updateIcon()
{
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
         m_settingsBtn->setIcon(QIcon(":/widgets/images/setting_dark.svg"));
         m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown_dark.svg"));
         if (m_hasCompterIcon)
             m_computerBtn->setIcon(QIcon(":/widgets/images/computer-symbolic_dark.svg"));
         if (m_hasVideoIcon)
             m_videoBtn->setIcon(QIcon(":/widgets/images/folder-videos-symbolic_dark.svg"));
         if (m_hasMusicIcon)
             m_musicBtn->setIcon(QIcon(":/widgets/images/folder-music-symbolic_dark.svg"));
         if (m_hasPictureIcon)
             m_pictureBtn->setIcon(QIcon(":/widgets/images/folder-pictures-symbolic_dark.svg"));
         if (m_hasDocumentIcon)
             m_documentBtn->setIcon(QIcon(":/widgets/images/folder-documents-symbolic_dark.svg"));
         if (m_hasDownloadIcon)
             m_downloadBtn->setIcon(QIcon(":/widgets/images/folder-downloads-symbolic_dark.svg"));
     } else {
         m_settingsBtn->setIcon(QIcon(":/widgets/images/setting.svg"));
         m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown.svg"));
         if (m_hasCompterIcon)
             m_computerBtn->setIcon(QIcon(":/widgets/images/computer-symbolic.svg"));
         if (m_hasVideoIcon)
             m_videoBtn->setIcon(QIcon(":/widgets/images/folder-videos-symbolic.svg"));
         if (m_hasMusicIcon)
             m_musicBtn->setIcon(QIcon(":/widgets/images/folder-music-symbolic.svg"));
         if (m_hasPictureIcon)
             m_pictureBtn->setIcon(QIcon(":/widgets/images/folder-pictures-symbolic.svg"));
         if (m_hasDocumentIcon)
             m_documentBtn->setIcon(QIcon(":/widgets/images/folder-documents-symbolic.svg"));
         if (m_hasDownloadIcon)
             m_downloadBtn->setIcon(QIcon(":/widgets/images/folder-downloads-symbolic.svg"));
    }

    update();
}
