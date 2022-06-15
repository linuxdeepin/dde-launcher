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

#include <DGuiApplicationHelper>

#include <diconbutton.h>
#include "avatar.h"

DWIDGET_USE_NAMESPACE

class MiniFrameButton;
class MiniFrameRightBar : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameRightBar(QWidget *parent = nullptr);
    ~MiniFrameRightBar() override;

    void setCurrentCheck(bool checked) const;
    void setCurrentIndex(int index) { m_currentIndex = index; }
    void moveUp();
    void moveDown();
    void execCurrent();
    void hideAllHoverState() const;

signals:
    void modeToggleBtnClicked();
    void requestFrameHide();

protected:
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void showShutdown();
    void showSettings();
    void updateIcon();

private:
    int m_currentIndex;
    QStringList m_hideList;
    QMap<uint, MiniFrameButton*> m_btns;
    MiniFrameButton *m_settingsBtn;
    MiniFrameButton *m_powerBtn;
};

#endif // MINIFRAMERIGHTBAR_H
