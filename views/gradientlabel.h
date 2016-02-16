/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

    Direction direction() const;
    void setDirection(const Direction &direction);

private:
    Direction m_direction;

    void paintEvent(QPaintEvent* event);
};

#endif // GRADIENTLABEL_H
