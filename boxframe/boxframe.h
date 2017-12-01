/*
 * Copyright (C) 2015 ~ 2017 Deepin Technology Co., Ltd.
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

#ifndef BOXFRAME_H
#define BOXFRAME_H

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>

#include <QFrame>
#include <QLabel>

class BoxFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BoxFrame(QWidget* parent = 0);
    explicit BoxFrame(const QString &url, QWidget *parent = 0);
    ~BoxFrame();

    void setBackground(const QString &url, bool force = false);
    QPixmap backgroundPixmap();

signals:
    void backgroundChanged();

private:
    QString m_lastUrl;
    QPixmap m_pixmap;
    QPixmap m_cache;
};

#endif // BOXFRAME_H
