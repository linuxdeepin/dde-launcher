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

MiniFrameRightBar::MiniFrameRightBar(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(0)
    , m_hideList(SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "mini-frame-right-bar-hide-list", QStringList()).toStringList())
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    m_settingsBtn = new MiniFrameButton(tr("Control Center"));
    m_settingsBtn->setFixedHeight(36);
    m_settingsBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_settingsBtn->setIconSize(QSize(20, 20));
    m_settingsBtn->setAccessibleName("settingsbtn");

    m_powerBtn = new MiniFrameButton(tr("Power"));
    m_powerBtn->setFixedHeight(36);
    m_settingsBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_powerBtn->setIconSize(QSize(20, 20));
    m_powerBtn->setAccessibleName("powerbtn");

    uint index = 0;
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

    setAccessibleName("buttonwidget");
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->addStretch();
    hLayout->addWidget(m_powerBtn);
    hLayout->addStretch();
    hLayout->addWidget(m_settingsBtn);
    hLayout->addStretch();
    hLayout->setContentsMargins(0, 6, 0, 6);
    setLayout(hLayout);

    connect(m_settingsBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showSettings);
    connect(m_powerBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showShutdown);
}

MiniFrameRightBar::~MiniFrameRightBar()
{
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

void MiniFrameRightBar::showEvent(QShowEvent *event)
{
    return QWidget::showEvent(event);
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
     } else {
         m_settingsBtn->setIcon(QIcon(":/widgets/images/setting.svg"));
         m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown.svg"));
    }
}
