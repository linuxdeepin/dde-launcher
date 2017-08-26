/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QLabel>
#include <QSpacerItem>
#include <QPropertyAnimation>

#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit SearchLineEdit(QWidget *parent = nullptr);

public slots:
    void normalMode();
    void moveFloatWidget();

private slots:
    void editMode();
    void onTextChanged();

protected:
    bool event(QEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    DImageButton *m_icon;
    DImageButton *m_clear;
    QLabel *m_placeholderText;
    QWidget *m_floatWidget;
#ifndef ARCH_MIPSEL
    QPropertyAnimation *m_floatAni;
#endif
};

#endif // SEARCHLINEEDIT_H
