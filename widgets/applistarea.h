/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef APPLISTAREA_H
#define APPLISTAREA_H

#include <QScrollArea>

class AppListArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit AppListArea(QWidget *parent = 0);

signals:
    void mouseEntered();

protected:
    void enterEvent(QEvent *e);
};

#endif // APPLISTAREA_H
