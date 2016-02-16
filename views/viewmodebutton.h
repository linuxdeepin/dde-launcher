/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef VIEWMODEBUTTON_H
#define VIEWMODEBUTTON_H

#include "basecheckedbutton.h"

class ViewModeButton : public BaseCheckedButton
{
    Q_OBJECT
public:
    explicit ViewModeButton(QWidget *parent = 0);
    ~ViewModeButton();

signals:

public slots:
    void setNormalIconByMode(int mode);
    void setHoverIconByMode(int mode);
};

#endif // VIEWMODEBUTTON_H
