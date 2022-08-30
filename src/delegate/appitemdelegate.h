// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    const QPair<QString, bool> holdTextInRect(const QFontMetrics &fm, const QString &text, const QRect &rect) const;

private:
    CalculateUtil *m_calcUtil;
    QPixmap m_blueDotPixmap;   // 新安装的app样式
    QPixmap m_autoStartPixmap; // 自启动的app样式

    static QModelIndex CurrentIndex;
};

#endif // APPITEMDELEGATE_H
