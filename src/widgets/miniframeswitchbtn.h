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

#ifndef MINIFRAMESWITCHBTN_H
#define MINIFRAMESWITCHBTN_H

#include "roundedbutton.h"
#include <QPushButton>
#include <QLabel>

class MiniFrameSwitchBtn : public QPushButton
{
    Q_OBJECT

public:
    MiniFrameSwitchBtn(QWidget *parent = nullptr);
    ~MiniFrameSwitchBtn();

    void updateStatus(int status);

Q_SIGNALS:
    void jumpButtonClicked();

public slots:
    void showJumpBtn();
    void hideJumpBtn();

private:
    QLabel *m_textLabel;
    QLabel *m_enterIcon;
    RoundedButton *m_jumpButton;
};

#endif
