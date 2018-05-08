/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * Maintainer: rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MINIFRAMERIGHTBAR_H
#define MINIFRAMERIGHTBAR_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

#include <dimagebutton.h>
#include "avatar.h"

DWIDGET_USE_NAMESPACE

class MiniFrameRightBar : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameRightBar(QWidget *parent = nullptr);
    ~MiniFrameRightBar();

signals:
    void modeToggleBtnClicked();
    void requestFrameHide();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void updateTime();
    void openDirectory(const QString &dir);
    void openStandardDirectory(const QStandardPaths::StandardLocation &location);
    void handleShutdownAction(const QString &action);
    void handleAvatarClicked();
    void handleTimedateOpen();
    void showShutdown();
    void showSettings();
    void showManual();

private:
    DImageButton *m_modeToggleBtn;
    Avatar *m_avatar;
    QLabel *m_currentTimeLabel;
    QLabel *m_currentDateLabel;
    QTimer *m_refreshDateTimer;
};

#endif // MINIFRAMERIGHTBAR_H
