// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

void ModeToggleButton::setHovered(bool hover)
{
    m_hover = hover;
    update();
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
