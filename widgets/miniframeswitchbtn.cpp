/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#include "miniframeswitchbtn.h"
#include "../newframe.h"

#include <DSvgRenderer>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

MiniFrameSwitchBtn::MiniFrameSwitchBtn(QWidget *parent)
    : QPushButton(parent),
      m_textLabel(new QLabel),
      m_enterIcon(new QLabel),
      m_jumpButton(new RoundedButton)
{
    const auto ratio = devicePixelRatioF();
    QPixmap enterPixmap = DSvgRenderer::render(":/widgets/images/enter_details_normal.svg",
                                               QSize(20, 20) * ratio);
    QPixmap allPixmap = DSvgRenderer::render(":/widgets/images/all.svg",
                                             QSize(24, 24) * ratio);
    enterPixmap.setDevicePixelRatio(ratio);
    allPixmap.setDevicePixelRatio(ratio);

    m_textLabel->setStyleSheet("QLabel {"
                               "font-size: 14px;"
                               "}");

    m_jumpButton->setText(tr("New Intalled"));
    m_jumpButton->setVisible(false);

    m_enterIcon->setFixedSize(20, 20);
    m_enterIcon->setPixmap(enterPixmap);
    m_enterIcon->setVisible(false);

    QLabel *allIconLabel = new QLabel;
    allIconLabel->setPixmap(allPixmap);
    allIconLabel->setFixedSize(24, 24);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
    setObjectName("MiniFrameButton");
    setFocusPolicy(Qt::NoFocus);
    setFixedHeight(48);

    mainLayout->addWidget(allIconLabel);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_textLabel);
    mainLayout->addWidget(m_jumpButton);
    mainLayout->addWidget(m_enterIcon);

    connect(m_jumpButton, &QPushButton::clicked, this, &MiniFrameSwitchBtn::jumpButtonClicked);
}

MiniFrameSwitchBtn::~MiniFrameSwitchBtn()
{
}

void MiniFrameSwitchBtn::updateStatus(int status)
{
    if (status == NewFrame::Used) {
        m_textLabel->setText(tr("All programes"));
        m_enterIcon->setVisible(true);
    } else {
        m_textLabel->setText(tr("Return"));
        m_enterIcon->setVisible(false);
    }
}

void MiniFrameSwitchBtn::showJumpBtn()
{
    m_jumpButton->setVisible(true);
}

void MiniFrameSwitchBtn::hideJumpBtn()
{
    m_jumpButton->setVisible(false);
}
