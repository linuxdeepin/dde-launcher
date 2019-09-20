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

#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class QDBusInterface;
class DatetimeWidget : public QWidget
{
    Q_OBJECT

public:
    DatetimeWidget(QWidget *parent = nullptr);
    ~DatetimeWidget();

    int getDateTextWidth();

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private:
    void updateTime();

private:
    QLabel *m_currentTimeLabel;
    QLabel *m_currentDateLabel;
    QTimer *m_refreshDateTimer;
    QDBusInterface* m_24HourFormatInter;
};

#endif
