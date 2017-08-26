/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef CATEGORYTITLEWIDGET_H
#define CATEGORYTITLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include "global_util/calculate_util.h"

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
