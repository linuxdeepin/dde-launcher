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

#include "datetimewidget.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <QMouseEvent>

DatetimeWidget::DatetimeWidget(QWidget *parent)
    : QWidget(parent),
      m_currentTimeLabel(new QLabel),
      m_currentDateLabel(new QLabel),
      m_refreshDateTimer(new QTimer)
{
    m_refreshDateTimer->setInterval(1000);
    m_refreshDateTimer->start();

    m_currentTimeLabel->setStyleSheet("QLabel { font-size: 40px;}");
    m_currentTimeLabel->setFixedHeight(40);
    m_currentTimeLabel->setAlignment(Qt::AlignVCenter);
    m_currentDateLabel->setStyleSheet("QLabel { color: rgba(255, 255, 255, 0.6); }");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 0, 0, 0);
    layout->addWidget(m_currentTimeLabel);
    layout->addSpacing(2);
    layout->addWidget(m_currentDateLabel);

    setLayout(layout);

    updateTime();

    connect(m_refreshDateTimer, &QTimer::timeout, this, &DatetimeWidget::updateTime);
}

DatetimeWidget::~DatetimeWidget()
{
}

int DatetimeWidget::getDateTextWidth()
{
    return m_currentDateLabel->fontMetrics().boundingRect(m_currentDateLabel->text()).width();
}

void DatetimeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton) {
        emit clicked();
    }
}

void DatetimeWidget::updateTime()
{
    const QDateTime dateTime = QDateTime::currentDateTime();
    m_currentTimeLabel->setText(dateTime.toString("HH:mm"));
    m_currentDateLabel->setText(dateTime.date().toString(Qt::SystemLocaleLongDate));
}
