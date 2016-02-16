/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef CATEGORYITEM_H
#define CATEGORYITEM_H

#include <QFrame>

class QLabel;

class CategoryItem : public QFrame
{
    Q_OBJECT
public:
    explicit CategoryItem(QString text, QWidget *parent = 0);
    ~CategoryItem();

    void initUI(QString text);

signals:

public slots:
    void addTextShadow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QLabel* m_titleLabel;
};

#endif // CATEGORYITEM_H
