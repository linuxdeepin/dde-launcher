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
#include <DFontSizeManager>
#include <QDBusInterface>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE

DatetimeWidget::DatetimeWidget(QWidget *parent)
    : QWidget(parent),
      m_currentTimeLabel(new QLabel),
      m_currentDateLabel(new QLabel),
      m_refreshDateTimer(new QTimer)
{
    m_refreshDateTimer->setInterval(1000);
    m_refreshDateTimer->start();

    m_24HourFormatInter = new QDBusInterface("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", "com.deepin.daemon.Timedate",
                                             QDBusConnection::sessionBus(), this);

    DFontSizeManager::instance()->bind(m_currentTimeLabel, DFontSizeManager::T3);
    m_currentTimeLabel->setFixedHeight(40);
    m_currentTimeLabel->setAlignment(Qt::AlignVCenter);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        QPalette pa = m_currentTimeLabel->palette();
        pa.setBrush(QPalette::WindowText, pa.brightText());
        m_currentTimeLabel->setPalette(pa);

        pa = m_currentDateLabel->palette();
        pa.setBrush(QPalette::WindowText, pa.brightText());
        m_currentDateLabel->setPalette(pa);
    });

    DFontSizeManager::instance()->bind(m_currentDateLabel, DFontSizeManager::T8);

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
    QString format;
    if (m_24HourFormatInter->property("Use24HourFormat").toBool())
        format = "hh:mm";
    else
        format = "hh:mm AP";
    m_currentTimeLabel->setText(dateTime.toString(format));
    m_currentDateLabel->setText(dateTime.date().toString(Qt::SystemLocaleLongDate));
}
