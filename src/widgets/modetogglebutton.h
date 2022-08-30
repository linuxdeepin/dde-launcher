// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODETOGGLEBUTTON_H
#define MODETOGGLEBUTTON_H

#include <DToolButton>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class ModeToggleButton : public DToolButton
{
    Q_OBJECT

public:
    explicit ModeToggleButton(QWidget *parent = nullptr);
    void setHovered(bool hover);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void resetStyle();

private:
    bool m_hover = false;
    QColor m_colorHover;
};

#endif
