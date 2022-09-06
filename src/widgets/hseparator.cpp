// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hseparator.h"
#include <QPainter>

HSeparator::HSeparator(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedHeight(1);
}

HSeparator::~HSeparator()
{
}

void HSeparator::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 0.1 * 255));
    painter.drawRect(rect());
}
