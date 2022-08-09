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

#include <DGuiApplicationHelper>

#include <QWidget>
#include <QLabel>

DGUI_USE_NAMESPACE

class MiniFrameButton;
class QButtonGroup;

class MiniFrameRightBar : public QWidget
{
    Q_OBJECT

public:
    enum ButtonType {
        Setting,
        Power,
        Unknow
    };

    explicit MiniFrameRightBar(QWidget *parent = nullptr);
    ~MiniFrameRightBar() override;

    void setButtonChecked(const ButtonType buttonId) const;
    bool isButtonChecked(const ButtonType buttonId) const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void initUi();
    void initConnection();
    void initAccessibleName();

signals:
    void requestFrameHide();

public slots:
    void showShutdown();
    void showSettings();

private slots:
    void updateIcon(DGuiApplicationHelper::ColorType themeType);

private:
    MiniFrameButton *m_settingsBtn;
    MiniFrameButton *m_powerBtn;
    QButtonGroup *m_buttonGroup;
    QStringList m_hideList;
};

#endif // MINIFRAMERIGHTBAR_H
