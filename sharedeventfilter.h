/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#ifndef SHAREDEVENTFILTER_H
#define SHAREDEVENTFILTER_H

#include <QObject>
#include <QKeyEvent>

#include "launcherinterface.h"

class SharedEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit SharedEventFilter(QObject *parent = 0);

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    LauncherInterface *m_frame;

    bool handleKeyEvent(QKeyEvent *event);
};

#endif // SHAREDEVENTFILTER_H
