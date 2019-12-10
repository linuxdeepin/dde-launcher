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

#ifndef CATEGORYTITLEWIDGET_H
#define CATEGORYTITLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include "src/global_util/calculate_util.h"


class QPropertyAnimation;

class CategoryTitleWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CategoryTitleWidget(const QString &title, QWidget *parent = 0);

    Q_PROPERTY(qreal titleOpacity READ titleOpacity WRITE setTitleOpacity)

    QLabel *textLabel();

    qreal titleOpacity() const;
    void setTitleOpacity(const qreal &titleOpacity);

public slots:
    void setTextVisible(const bool visible, const bool animation = false);
    void setText(const QString &title);

private:
    void addTextShadow();

private slots:
    void relayout();

private:
    CalculateUtil *m_calcUtil;
    QLabel *m_title;

    QPropertyAnimation *m_opacityAnimation;
    qreal m_titleOpacity;
};

#endif // CATEGORYTITLEWIDGET_H
