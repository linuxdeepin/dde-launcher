/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "searchlineedit.h"

#include <QWidget>

class SearchWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);

    QLineEdit *edit();

public slots:
    void clearSearchContent();

signals:
    void searchTextChanged(const QString &text) const;

private:
    SearchLineEdit* m_searchEdit;
};

#endif // SEARCHWIDGET_H
