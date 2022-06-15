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
#include "util.h"

#include <DDesktopServices>
#include <DDBusSender>

#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QButtonGroup>
#include <QtConcurrent>

MiniFrameRightBar::MiniFrameRightBar(QWidget *parent)
    : QWidget(parent)
    , m_settingsBtn(new MiniFrameButton(tr("Control Center"), this))
    , m_powerBtn(new MiniFrameButton(tr("Power"), this))
    , m_buttonGroup(new QButtonGroup(this))
    , m_hideList(SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "mini-frame-right-bar-hide-list", QStringList()).toStringList())
{
    initUi();
    initConnection();
    initAccessibleName();
}

MiniFrameRightBar::~MiniFrameRightBar()
{
}

void MiniFrameRightBar::initUi()
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_settingsBtn->setFixedHeight(36);
    m_powerBtn->setFixedHeight(36);
    m_settingsBtn->setIconSize(QSize(20, 20));
    m_powerBtn->setIconSize(QSize(20, 20));

    m_settingsBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_settingsBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_buttonGroup->addButton(m_settingsBtn);
    m_buttonGroup->addButton(m_powerBtn);
    m_settingsBtn->setCheckable(true);
    m_powerBtn->setCheckable(true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 6, 0, 6);
    hLayout->addStretch();
    hLayout->addWidget(m_powerBtn);
    hLayout->addStretch();
    hLayout->addWidget(m_settingsBtn);
    hLayout->addStretch();
    setLayout(hLayout);

    updateIcon(DGuiApplicationHelper::instance()->themeType());
}

void MiniFrameRightBar::initConnection()
{
    connect(m_settingsBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showSettings);
    connect(m_powerBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showShutdown);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MiniFrameRightBar::updateIcon);
}

void MiniFrameRightBar::initAccessibleName()
{
    setAccessibleName("buttonwidget");
    m_settingsBtn->setAccessibleName("settingsbtn");
    m_powerBtn->setAccessibleName("powerbtn");
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
            .method(QString("Show"))
            .call();
    });

    emit requestFrameHide();
}

void MiniFrameRightBar::updateIcon(DGuiApplicationHelper::ColorType themeType)
{
    if (DGuiApplicationHelper::DarkType == themeType) {
         m_settingsBtn->setIcon(QIcon(":/widgets/images/setting_dark.svg"));
         m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown_dark.svg"));
     } else {
         m_settingsBtn->setIcon(QIcon(":/widgets/images/setting.svg"));
         m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown.svg"));
    }
}

void MiniFrameRightBar::changeEvent(QEvent *event)
{
#define ICON_WIDTH 20
#define GAP_WIDTH 20

    Q_UNUSED(event);

    QFontMetrics metrics(m_settingsBtn->font());
    QSize textSize = metrics.boundingRect(m_settingsBtn->text()).size();
    m_settingsBtn->setFixedSize(textSize + QSize(ICON_WIDTH, 10) + QSize(GAP_WIDTH, 0));

    textSize = metrics.boundingRect(m_settingsBtn->text()).size();
    m_powerBtn->setFixedSize(textSize + QSize(ICON_WIDTH, 10) + QSize(GAP_WIDTH, 0));
}
