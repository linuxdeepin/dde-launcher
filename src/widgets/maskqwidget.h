/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     niecheng <niejiashan@163.com>
 *
 * Maintainer: niecheng <niejiashan@163.com>
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
#ifndef MASKQWIDGET_H
#define MASKQWIDGET_H

#include <QWidget>

class MaskQWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MaskQWidget(QWidget *parent = nullptr);

    void setColor(QColor* color);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor* m_color;

signals:

public slots:
};

#endif // MASKQWIDGET_H
