// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MYMENU_H
#define MYMENU_H

#include <QMenu>
#include <QWidget>

#include <com_deepin_api_xeventmonitor.h>

using XEventMonitor = com::deepin::api::XEventMonitor;

class Menu : public QMenu
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    void moveDown(int size = 0);
    void moveUp(int size = 0);
    void openItem();

Q_SIGNALS:
    void notifyMenuDisplayState(bool);

private Q_SLOTS:
    void onButtonPress();

private:
    XEventMonitor *m_monitor;
};

#endif //MYMENU_H
