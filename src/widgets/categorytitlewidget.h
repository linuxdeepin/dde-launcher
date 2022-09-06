// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATEGORYTITLEWIDGET_H
#define CATEGORYTITLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include "calculate_util.h"

class QPropertyAnimation;

class CategoryTitleWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CategoryTitleWidget(const QString &title, QWidget *parent = nullptr);

    Q_PROPERTY(qreal titleOpacity READ titleOpacity WRITE setTitleOpacity)

    QLabel *textLabel();

    qreal titleOpacity() const;
    void setTitleOpacity(const qreal &titleOpacity);
    void updatePosition(const QPoint pos, int w, int posType);

public slots:
    void setText(const QString &title);

private slots:
    void relayout();

private:
    CalculateUtil *m_calcUtil;
    QLabel *m_title;

    QPropertyAnimation *m_opacityAnimation;
    qreal m_titleOpacity;
};

#endif // CATEGORYTITLEWIDGET_H
