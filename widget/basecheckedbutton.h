/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef BASECHECKEDBUTTON_H
#define BASECHECKEDBUTTON_H

#include <QPushButton>

class QEvent;

class BaseCheckedButton : public QPushButton
{
    Q_OBJECT
public:
    explicit BaseCheckedButton(QWidget *parent = 0);
    explicit BaseCheckedButton(QString text, QWidget *parent = 0);
    ~BaseCheckedButton();

signals:
    void mouseEnterToggled(bool flag);

public slots:

protected:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
};

#endif // BASECHECKEDBUTTON_H
