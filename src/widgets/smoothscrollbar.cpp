/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "smoothscrollbar.h"
#include <QPropertyAnimation>

SmoothScrollBar::SmoothScrollBar(QWidget *parent) : QScrollBar(parent)
  ,m_propertyAnimation(new QPropertyAnimation(this,"value",this))
{
    m_targetValue = value();

    m_propertyAnimation->setDuration(800);
    m_propertyAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_propertyAnimation,&QPropertyAnimation::finished,this,&SmoothScrollBar::scrollFinish);
}

void SmoothScrollBar::setValueSmooth(int value)
{
    m_propertyAnimation->stop();
    m_propertyAnimation->setEndValue(value);
    m_propertyAnimation->start();
}

void SmoothScrollBar::scrollSmooth(int value)
{

    if(m_targetValue > maximum()){
        m_targetValue = maximum();
    }else if(m_targetValue < minimum()){
        m_targetValue = minimum();
    }
    m_targetValue += value;
    setValueSmooth(m_targetValue);
}

void SmoothScrollBar::stopScroll()
{
    m_propertyAnimation->stop();
    m_targetValue = value();
}

