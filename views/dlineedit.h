/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DLINEEDIT_H
#define DLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>

class DLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit DLineEdit(QWidget *parent = 0);

signals:

public slots:

protected:
    void keyPressEvent(QKeyEvent* event);

};

#endif // DLINEEDIT_H
