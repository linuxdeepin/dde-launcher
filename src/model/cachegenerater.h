// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CACHEGENERATER_H
#define CACHEGENERATER_H

#include <QObject>

class CacheGenerater : public QObject
{
    Q_OBJECT
public:
    explicit CacheGenerater(QObject *parent = nullptr);

signals:

public slots:
};

#endif // CACHEGENERATER_H
