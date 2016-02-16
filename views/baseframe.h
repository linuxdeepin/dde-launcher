/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef BASEFRAME_H
#define BASEFRAME_H

#include <QFrame>

class QEvent;

class BaseFrame : public QFrame
{
    Q_OBJECT
public:
    explicit BaseFrame(QWidget *parent = 0);
    ~BaseFrame();

signals:
    void mouseEnterToggled(bool flag);

public slots:

protected:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
};

#endif // BASEFRAME_H
