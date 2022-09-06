// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HSEPARATOR_H
#define HSEPARATOR_H

#include <QWidget>

class HSeparator : public QWidget
{
    Q_OBJECT

public:
    explicit HSeparator(QWidget *parent = nullptr);
    ~HSeparator();

protected:
    void paintEvent(QPaintEvent *e);
};

#endif
