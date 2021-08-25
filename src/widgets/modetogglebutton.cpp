/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     suyihang <suyihang@uniontech.com>
*
* Maintainer: suyihang <suyihang@uniontech.com>
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

#include "modetogglebutton.h"

#include <QPainter>
#include <QPainterPath>

/**
 * @brief ModeToggleButton::ModeToggleButton
 * 启动器小窗口和全屏窗口切换控件
 * @param parent
 */
ModeToggleButton::ModeToggleButton(QWidget *parent)
    : DToolButton(parent)
{
    setFocusPolicy(Qt::NoFocus);

    resetStyle();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ModeToggleButton::resetStyle);
}

void ModeToggleButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制背景
    if (hasFocus() || m_hover) {
        QPainterPath path;
        path.addRoundedRect(rect(), 8, 8);
        painter.fillPath(path, m_colorHover);
    }

    // 绘制图标
    if (!icon().isNull()) {
        painter.drawPixmap(rect().center().x() - iconSize().width() / 2 + 1 , rect().center().y() - iconSize().height() / 2 + 1, icon().pixmap(iconSize()));
    }
}

void ModeToggleButton::enterEvent(QEvent *event)
{
    DToolButton::enterEvent(event);

    m_hover = true;
    update();
}

void ModeToggleButton::leaveEvent(QEvent *event)
{
    DToolButton::leaveEvent(event);

    m_hover = false;
    update();
}

void ModeToggleButton::resetStyle()
{
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        m_colorHover.setRgb(255, 255, 255, 25);
        setIcon(QIcon(":/icons/skin/icons/fullscreen_dark.svg"));
    } else {
        m_colorHover.setRgb(0, 0, 0, 25);
        setIcon(QIcon(":/icons/skin/icons/fullscreen_normal.svg"));
    }
    update();
}
