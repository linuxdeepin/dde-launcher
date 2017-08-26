/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "calculate_util.h"
#include "dbusinterface/monitorinterface.h"
#include "dbusinterface/dbusdisplay.h"

#include <QDebug>
#include <QScreen>
#include <QDesktopWidget>
#include <QApplication>

static const QString DisplayModeKey = "display-mode";
static const QString DisplayModeFree = "free";
static const QString DisplayModeCategory = "category";

QPointer<CalculateUtil> CalculateUtil::INSTANCE = nullptr;

CalculateUtil *CalculateUtil::instance()
{
    if (INSTANCE.isNull())
        INSTANCE = new CalculateUtil(nullptr);

    return INSTANCE;
}

int CalculateUtil::calculateBesidePadding(const int screenWidth)
{
    // static const int NAVIGATION_WIDGET_WIDTH = 180;
    if (screenWidth > 1366)
        return 180;
    return 130;
}

void CalculateUtil::setDisplayMode(const int mode)
{
    m_launcherGsettings->set(DisplayModeKey, mode == ALL_APPS ? DisplayModeFree : DisplayModeCategory);
}

int CalculateUtil::displayMode() const
{
    const QString displayMode = m_launcherGsettings->get(DisplayModeKey).toString();

    if (displayMode == DisplayModeCategory) {
        return GROUP_BY_CATEGORY;
    }

    return ALL_APPS;
}

void CalculateUtil::calculateAppLayout(const QSize &containerSize, const int dockPosition)
{
    // mini mode
    if (!m_launcherInter->fullscreen())
    {
        m_appItemSpacing = 6;
        m_appItemWidth = 120;
        m_appItemHeight = 120;
        m_appIconSize = displayMode() == ALL_APPS ? 48 : 32;
        m_appItemFontSize = 11;
        m_appColumnCount = displayMode() == ALL_APPS ? 4 : 1;

        emit layoutChanged();
        return;
    }

    // NOTE(hualet): DPI default to 96.
    static auto PtToPx = [] (float pt) -> int {
        return pt * 96 / 72.0;
    };

    const QScreen *screen = qApp->primaryScreen();
    const qreal ratio = screen->devicePixelRatio();
    const int screenWidth = screen->geometry().width() / ratio;
    const int column = screenWidth <= 800 ? 5 : screenWidth <= 1024 && dockPosition == 3 ? 6 : 7;

    calculateTextSize(screenWidth);

    // calculate item size;
    int spacing = itemSpacing(containerSize.width());
    int itemWidth = 140;

    const int itemCalcWidth = (double(containerSize.width()) - spacing * column * 2) / column + 0.5;
    itemWidth = qMin(itemWidth, itemCalcWidth);

    spacing = (double(containerSize.width()) - itemWidth * column) / (column * 2) - 1;

    m_appItemSpacing = spacing;
    m_appItemWidth = itemWidth;
    m_appItemHeight = m_appItemWidth;
    m_appColumnCount = column;

    // calculate icon size;
    m_appIconSize = itemIconWidth(m_appItemWidth);

    // calculate font size;
    QFont systemFont;
    m_appItemFontSize = m_appItemWidth <= 80 ? 11 : PtToPx(systemFont.pointSizeF());

    emit layoutChanged();
}

CalculateUtil::CalculateUtil(QObject *parent)
    : QObject(parent),
      m_launcherGsettings(new QGSettings("com.deepin.dde.launcher",
                                         "/com/deepin/dde/launcher/", this))
{
    m_launcherInter = new DBusLauncher(this);
}

int CalculateUtil::itemSpacing(const int containerWidth) const
{
    if (containerWidth <= 500)
        return 6;
    if (containerWidth <= 1000)
        return 15;
    return 20;
}

int CalculateUtil::itemIconWidth(const int itemWidth) const
{
    //    m_appIconSize = qMin(64, int(m_appItemWidth * 0.65 / 16) * 16);
    //    m_appIconSize = m_appItemWidth > 64 * 2 ? 64 : 48;

    if (itemWidth > 64 * 2)
        return 64;
    if (itemWidth > 85)
        return 48;
    if (itemWidth > 60)
        return 32;
    return 24;
}

void CalculateUtil::calculateTextSize(const int screenWidth)
{
    if (screenWidth > 1366)
    {
        m_navgationTextSize = 14;
        m_titleTextSize = 15;
    } else {
        m_navgationTextSize = 11;
        m_titleTextSize = 13;
    }
}
