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
      m_enterIcon(new QLabel)
{
    const auto ratio = devicePixelRatioF();
    QPixmap enterPixmap = DSvgRenderer::render(":/widgets/images/enter_details_normal.svg",
                                               QSize(20, 20) * ratio);
    enterPixmap.setDevicePixelRatio(ratio);

    m_textLabel->setStyleSheet("QLabel {"
                               "font-size: 14px;"
                               "}");

    m_enterIcon->setFixedSize(20, 20);
    m_enterIcon->setPixmap(enterPixmap);
    m_enterIcon->setVisible(false);

    setLayout(new QHBoxLayout(this));
    setObjectName("MiniFrameButton");
    setFocusPolicy(Qt::NoFocus);
    setFixedHeight(40);

    layout()->addWidget(m_textLabel);
    layout()->addWidget(m_enterIcon);
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
