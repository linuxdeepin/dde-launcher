/*
 * Copyright (C) 2022 Deepin Technology Co., Ltd.
 * Copyright (C) 2022 Maicss <maicss@126.com>
 *
 * Author:     Maicss <maicss@126.com>
 *
 * Maintainer: Maicss <maicss@126.com>
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

#ifndef SMOOTHSCROLLBAR_H
#define SMOOTHSCROLLBAR_H

#include <QWidget>
#include <QScrollBar>
#include <QPropertyAnimation>

class SmoothScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    explicit SmoothScrollBar(QWidget *parent = nullptr);

signals:
    void scrollFinished();

public slots:
    void setValueSmooth(int value);
    void scrollSmooth(int value);
    void stopScroll();

private:
    int m_targetValue;

    QPropertyAnimation *m_propertyAnimation;
};

#endif // SMOOTHSCROLLBAR_H
