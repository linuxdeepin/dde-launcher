/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QListView>
#include <QPushButton>

class HistoryModel;
class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = 0);

private slots:
    void onItemClicked(const QModelIndex &index);

private:
    HistoryModel *m_historyModel;
    QListView *m_historyView;
    QPushButton *m_clearBtn;
};

#endif // HISTORYWIDGET_H
