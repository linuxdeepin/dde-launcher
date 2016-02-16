/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dbusworker.h"
#include "app/global.h"

DBusWorker::DBusWorker(QObject *parent) :
    QObject(parent)
{
    initConnect();
}

void DBusWorker::initConnect(){
    connect(signalManager, SIGNAL(requestData()),
            this, SLOT(requsetData()));
}

void DBusWorker::requsetData(){
    dbusController->init();
}
