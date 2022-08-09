/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef APPLISTDELEGATE_H
#define APPLISTDELEGATE_H

#include <QAbstractItemDelegate>

class CalculateUtil;
class AppListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AppListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    static QPixmap dropShadow(QPixmap pixmap, int radius, const QColor &color, const QPoint &offset);

public slots:
    void setActived(bool active);

private:
    bool m_actived;
    CalculateUtil *m_calcUtil;
    QPixmap m_blueDotPixmap;
    QPixmap m_autoStartPixmap;
};

#endif // APPLISTDELEGATE_H
