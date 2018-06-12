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

#include <QPainter>
#include <QDebug>
#include <QLinearGradient>

#include "gradientlabel.h"

GradientLabel::GradientLabel(QWidget *parent) :
    QLabel(parent),
    m_direction(GradientLabel::TopToBottom)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

GradientLabel::Direction GradientLabel::direction() const
{
    return m_direction;
}

void GradientLabel::setDirection(const GradientLabel::Direction &direction)
{
    m_direction = direction;
}

void setText(const QString &)
{
    // do nothing !
}

void GradientLabel::paintEvent(QPaintEvent*)
{
//    QPainter painter;
//    painter.begin(this);

//    painter.fillRect(rect(), Qt::red);

//    painter.end();

    // process the pixmap
    const QPixmap *thisPix = pixmap();
    QPixmap pix(thisPix->rect().size());
    pix.fill(Qt::transparent);

    QPainter pixPainter;
    pixPainter.begin(&pix);
    pixPainter.drawPixmap(0, 0, *thisPix);

    pixPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

    QLinearGradient gradient(pix.rect().topLeft(),
                             pix.rect().bottomLeft());

    if (m_direction == TopToBottom) {
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, Qt::transparent);
    } else if (m_direction == BottomToTop) {
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::white);
    }

    pixPainter.fillRect(pix.rect(), gradient);

    pixPainter.end();

    // draw the pixmap
    QPainter painter;
    painter.begin(this);

    painter.drawPixmap(0, 0, pix);

    painter.end();
}
