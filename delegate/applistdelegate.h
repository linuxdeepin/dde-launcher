/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef APPLISTDELEGATE_H
#define APPLISTDELEGATE_H

#include <QAbstractItemDelegate>

class AppListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AppListDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

public slots:
    void setActived(bool active);

private:
    bool m_actived;
    QPixmap m_blueDotPixmap;
    QPixmap m_autoStartPixmap;
};

#endif // APPLISTDELEGATE_H
