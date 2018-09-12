/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef GRADIENTLABEL_H
#define GRADIENTLABEL_H

#include <QLabel>

class QPaintEvent;
class GradientLabel : public QLabel
{
    Q_OBJECT
public:
    explicit GradientLabel(QWidget *parent = 0);

    enum Direction {
        TopToBottom,
        BottomToTop
    };

    void setText(const QString &);
    void setPixmap(QPixmap pixmap);

    Direction direction() const;
    void setDirection(const Direction &direction);

protected:
    void paintEvent(QPaintEvent* event);

private:
    Direction m_direction;
    QPixmap m_pixmap;
};

#endif // GRADIENTLABEL_H
