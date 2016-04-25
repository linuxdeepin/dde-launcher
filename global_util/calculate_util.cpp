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

void CalculateUtil::calculateAppLayout(const QSize &containerSize)
{
    const int screenWidth = qApp->primaryScreen()->geometry().width();
    const int column = screenWidth < 800 ? 5 : 7;

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
    viewMarginRation();
    //scale the icon when the resolution is 800*600
    if (bestSize.width()<=800) {
        m_appIconSize = 48*2/3;
    }
    // calculate font size;
    m_appItemFontSize = m_appItemWidth >= 130 ? 13 : m_appItemWidth <= 80 ? 9 : 11;

    emit layoutChanged();
}

double CalculateUtil::viewMarginRation() {
    DBusDisplay* m_dbusDisplay = new DBusDisplay(this);
    QList<QDBusObjectPath> pathList = m_dbusDisplay->monitors();
    if (pathList.length()!=0) {
        MonitorInterface* m_displayMoniterface = new MonitorInterface(pathList[0].path(), this);
        MonitorMode bestMode = m_displayMoniterface->bestMode();

        MonitorMode currentMode = m_displayMoniterface->currentMode();
        bestSize.setWidth(currentMode.width);

        if (bestMode.width == currentMode.width && bestMode.height == currentMode.height) {
            m_viewMarginRation = 1.00;
        } else if (bestMode.width == 600) {
            m_viewMarginRation = double(currentMode.width)/double(bestMode.width);
        } else {
            m_viewMarginRation = double(currentMode.width)/double(bestMode.width);
        }
    } else {
        m_viewMarginRation = 1.00;
    }

    return m_viewMarginRation;
}
CalculateUtil::~CalculateUtil(){}
CalculateUtil::CalculateUtil(QObject *parent) : QObject(parent)
{

}
