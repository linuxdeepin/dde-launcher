// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLISTAREA_H
#define APPLISTAREA_H

#include <QScrollArea>

class AppListArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit AppListArea(QWidget *parent = nullptr);

signals:
    void increaseIcon();
    void decreaseIcon();

protected:
    void wheelEvent(QWheelEvent *e) override;
};

#endif // APPLISTAREA_H
