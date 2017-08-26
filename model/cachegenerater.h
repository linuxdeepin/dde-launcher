/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef CACHEGENERATER_H
#define CACHEGENERATER_H

#include <QObject>

class CacheGenerater : public QObject
{
    Q_OBJECT
public:
    explicit CacheGenerater(QObject *parent = 0);

signals:

public slots:
};

#endif // CACHEGENERATER_H