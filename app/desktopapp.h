/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DESKTOPAPP_H
#define DESKTOPAPP_H

#include <QtCore>
#include "views/gridmanager.h"

class DesktopBox;
class AppController;

class DesktopApp : public QObject
{
    Q_OBJECT
public:
    explicit DesktopApp(QObject *parent = 0);
    ~DesktopApp();

    void initConnect();

signals:

public slots:
    void loadSettings();
    void saveSettings();

    void saveGridOn(bool mode);
    void saveSizeType(SizeType type);
    void saveSortFlag(QDir::SortFlag flag);

    void show();

private:
    DesktopBox* m_desktopBox;
    AppController* m_appController;
};

#endif // DESKTOPAPP_H
