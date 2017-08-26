/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef APPLISTVIEW_H
#define APPLISTVIEW_H

#include <QListView>

class AppListView : public QListView
{
    Q_OBJECT

public:
    explicit AppListView(QWidget *parent = 0);

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;

protected:
    void mousePressEvent(QMouseEvent *e);
};

#endif // APPLISTVIEW_H
