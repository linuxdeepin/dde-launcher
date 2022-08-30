// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smoothscrollbar.h"
#include <QPropertyAnimation>

SmoothScrollBar::SmoothScrollBar(QWidget *parent)
    : QScrollBar(parent)
    , m_propertyAnimation(new QPropertyAnimation(this, "value", this))
{
    m_targetValue = value();

    m_propertyAnimation->setDuration(800);
    m_propertyAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_propertyAnimation, &QPropertyAnimation::finished, this, &SmoothScrollBar::scrollFinished);
}

void SmoothScrollBar::setValueSmooth(int value)
{
    m_propertyAnimation->stop();
    m_propertyAnimation->setEndValue(value);
    m_propertyAnimation->start();
}

void SmoothScrollBar::scrollSmooth(int value)
{
    // 这里先对m_targetValue限制是为了实现撞停的效果，否则滚动到上下边缘会减速
    m_targetValue = qBound(minimum(), m_targetValue, maximum());
    m_targetValue += value;
    setValueSmooth(m_targetValue);
}

void SmoothScrollBar::stopScroll()
{
    m_propertyAnimation->stop();
    m_targetValue = value();
}

