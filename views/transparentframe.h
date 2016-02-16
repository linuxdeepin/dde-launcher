/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef TRANSPARENTFRAME_H
#define TRANSPARENTFRAME_H

#include <QFrame>
class QMouseEvent;

class TransparentFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TransparentFrame(QWidget *parent = 0);
    ~TransparentFrame();

signals:

public slots:

protected:
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // TRANSPARENTFRAME_H
