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
#include <QWidget>
#include <QLabel>

class MiniFrameSwitchBtn : public QWidget
{
    Q_OBJECT

public:
    MiniFrameSwitchBtn(QWidget *parent = nullptr);
    ~MiniFrameSwitchBtn() override;

    void updateStatus(int status);
    void click();

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QLabel *m_textLabel;
    QLabel *m_enterIcon;
    QLabel *m_allIconLabel;
    bool m_hover = false;

};

#endif
