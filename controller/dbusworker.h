/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DBUSWORKER_H
#define DBUSWORKER_H

#include <QObject>

class DBusWorker : public QObject
{
    Q_OBJECT
public:
    explicit DBusWorker(QObject *parent = 0);
    void initConnect();
signals:

public slots:
    void requsetData();

};

#endif // DBUSWORKER_H
