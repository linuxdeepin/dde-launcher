#include "calculate_util.h"
#include "dbusinterface/monitorinterface.h"
#include "dbusinterface/dbusdisplay.h"

#include <QDebug>
#include <QScreen>
#include <QDesktopWidget>
#include <QApplication>

CalculateUtil *CalculateUtil::INSTANCE = nullptr;

CalculateUtil *CalculateUtil::instance(QObject *parent)
{
    if (!INSTANCE)
        INSTANCE = new CalculateUtil(parent);

    return INSTANCE;
}

int CalculateUtil::calculateBesidePadding(const int screenWidth)
{
    // static const int NAVIGATION_WIDGET_WIDTH = 180;
    if (screenWidth > 1366)
        return 180;
    return 130;
}

void CalculateUtil::calculateAppLayout(const QSize &containerSize)
{
    const int screenWidth = qApp->primaryScreen()->geometry().width();
    const int column = screenWidth <= 800 ? 5 : 7;

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
    m_appItemFontSize = m_appItemWidth >= 130 ? 13 : m_appItemWidth <= 80 ? 11 : 13;

    emit layoutChanged();
}

CalculateUtil::CalculateUtil(QObject *parent) : QObject(parent)
{

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
    if (itemWidth > 80)
        return 48;
    return 24;
}

void CalculateUtil::calculateTextSize(const int screenWidth)
{
    if (screenWidth >= 1366)
    {
        m_navgationTextSize = 14;
        m_titleTextSize = 15;
    } else {
        m_navgationTextSize = 11;
        m_titleTextSize = 13;
    }
}
