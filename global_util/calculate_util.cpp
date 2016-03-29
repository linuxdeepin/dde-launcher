#include "calculate_util.h"

#include <QDebug>
#include <QApplication>
#include <QScreen>

CalculateUtil *CalculateUtil::INSTANCE = nullptr;

CalculateUtil *CalculateUtil::instance(QObject *parent)
{
    if (!INSTANCE)
        INSTANCE = new CalculateUtil(parent);

    return INSTANCE;
}

void CalculateUtil::calculateAppLayout(const QSize &containerSize)
{
    const int screenWidth = qApp->primaryScreen()->geometry().width();
    const int column = screenWidth == 800 ? 5 : screenWidth > 800 ? 7 : 4;

    // calculate item size;
    int spacing = 20;
    int itemWidth = 140;

    int itemCalcWidth = int((double(containerSize.width()) - spacing * column * 2) / column + 0.5);
    if (itemCalcWidth < itemWidth)
        itemWidth = itemCalcWidth;

    spacing = (containerSize.width() - itemWidth * column) / (column * 2);

    m_appItemSpacing = spacing;
    m_appItemWidth = itemWidth;
    m_appItemHeight = m_appItemWidth;
    m_appColumnCount = column;

//    m_appItemWidth = 80;
    // calculate icon size;
//    m_appIconSize = int(m_appItemWidth * 0.5 / 16) * 16;
    m_appIconSize = m_appItemWidth > 64 * 2 ? 64 : 48;

    // calculate font size;
    m_appItemFontSize = m_appItemWidth >= 130 ? 12 : m_appItemWidth <= 80 ? 9 : 11;

    emit layoutChanged();
}

CalculateUtil::CalculateUtil(QObject *parent) : QObject(parent)
{

}
