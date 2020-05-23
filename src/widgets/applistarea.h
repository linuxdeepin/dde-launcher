/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef APPLISTAREA_H
#define APPLISTAREA_H

#include <QScrollArea>
#include <QSet>

class AppListArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit AppListArea(QWidget *parent = 0);
    void addWidget(QWidget * view);

signals:
    void mouseEntered();
    void increaseIcon();
    void decreaseIcon();

protected:
    void wheelEvent(QWheelEvent *e)override;
    void enterEvent(QEvent *e)override;
    bool eventFilter(QObject *watched, QEvent *e) override;

private:
    QTimer *m_updateEnableSelectionByMouseTimer;
    QPoint m_lastTouchBeginPos;
    int touchTapDistance = -1;
    QSet<QWidget*> m_views;
};

#endif // APPLISTAREA_H
