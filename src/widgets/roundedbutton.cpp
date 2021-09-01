/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#include "roundedbutton.h"

RoundedButton::RoundedButton(QWidget *parent)
    : QPushButton(parent)
{
    setObjectName("RoundedButton");
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(60, 30);
}

void RoundedButton::setText(const QString &text)
{
    const QFontMetrics fm(fontMetrics());
    const int padding = 10;
    const QString elidedText = fm.elidedText(text, Qt::ElideRight, rect().width() - padding);

    QPushButton::setText(elidedText);
}
