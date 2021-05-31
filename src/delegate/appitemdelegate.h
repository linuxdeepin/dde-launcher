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

#ifndef APPITEMDELEGATE_H
#define APPITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QPainter>

class CalculateUtil;
class AppItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AppItemDelegate(QObject *parent = nullptr);

    void setCurrentIndex(const QModelIndex &index);
    const QModelIndex &currentIndex() const {return CurrentIndex;}


signals:
    void requestUpdate(const QModelIndex &idx) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    const QRect itemBoundingRect(const QRect &itemRect) const;
    const QRect itemTextRect(const QRect &boundingRect, const QRect &iconRect, const bool extraWidthMargin) const;
    const QPair<QString, bool> holdTextInRect(const QFontMetrics &fm, const QString &text, const QRect &rect, const QFont font) const;

private:
    CalculateUtil *m_calcUtil;
    QPixmap m_blueDotPixmap;
    QPixmap m_autoStartPixmap;

    static QModelIndex CurrentIndex;
};

#endif // APPITEMDELEGATE_H
