// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
      m_currentTimeLabel(new QLabel(this)),
      m_currentDateLabel(new QLabel(this)),
      m_refreshDateTimer(new QTimer(this))
{
    m_refreshDateTimer->setInterval(1000);
    m_refreshDateTimer->start();

    m_24HourFormatInter = new QDBusInterface("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", "com.deepin.daemon.Timedate",
                                             QDBusConnection::sessionBus(), this);

    DFontSizeManager::instance()->bind(m_currentTimeLabel, DFontSizeManager::T3);
    m_currentTimeLabel->setFixedHeight(40);
    m_currentTimeLabel->setAlignment(Qt::AlignVCenter);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DatetimeWidget::onThemeTypeChange);

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

void DatetimeWidget::onThemeTypeChange(DGuiApplicationHelper::ColorType themeType)
{
    Q_UNUSED(themeType);

    QPalette pa = m_currentTimeLabel->palette();
    pa.setBrush(QPalette::WindowText, pa.brightText());
    m_currentTimeLabel->setPalette(pa);

    pa = m_currentDateLabel->palette();
    pa.setBrush(QPalette::WindowText, pa.brightText());
    m_currentDateLabel->setPalette(pa);
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
