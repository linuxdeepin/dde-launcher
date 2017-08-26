/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SHAREDEVENTFILTER_H
#define SHAREDEVENTFILTER_H

#include <QObject>
#include <QKeyEvent>

#include "launcherinterface.h"

class SharedEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit SharedEventFilter(QObject *parent = 0);

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    LauncherInterface *m_frame;

    bool handleKeyEvent(QKeyEvent *event);
};

#endif // SHAREDEVENTFILTER_H
