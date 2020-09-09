/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include "calculate_util.h"
#include "src/dbusinterface/monitorinterface.h"
#include "src/dbusinterface/dbusdisplay.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

#include "src/global_util/constants.h"

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

void CalculateUtil::setDisplayMode(const int mode)
{
    m_launcherGsettings->set(DisplayModeKey, mode == ALL_APPS ? DisplayModeFree : DisplayModeCategory);
}

QSize CalculateUtil::appIconSize() const
{
    if (!isFullScreen)
        return QSize(24, 24) * qApp->devicePixelRatio();

    QSize s(m_appItemSize, m_appItemSize);
    const double ratio = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    return s * ratio;
}

double CalculateUtil::getScreenScaleX()
{
    int width  = currentScreen()->geometry().width();
    return double(width) / 1920;
}

double CalculateUtil::getScreenScaleY()
{
    int width = currentScreen()->geometry().height();
    return double(width) / 1080;
}

QSize CalculateUtil::getScreenSize() const
{
    return currentScreen()->geometry().size();
}

QSize CalculateUtil::getAppBoxSize()
{
    int height = currentScreen()->geometry().height() * 0.69;
    int width = currentScreen()->geometry().width() * 0.51;
    return  QSize(width, height);
}

bool CalculateUtil::increaseIconSize()
{
    const double value = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    const double ratio = std::min(0.6, value + 0.1);

    if (qFuzzyCompare(value, ratio)) return false;

    m_launcherGsettings->set("apps-icon-ratio", ratio);
    return true;
}
QStringList CalculateUtil::calendarSelectIcon() const
{
    QStringList iconList;
    iconList.clear();
    iconList.append(QString(":/icons/skin/icons/calendar_bg.svg"));
    int month_num = QDate::currentDate().month();
    switch(month_num)
    {
    case 1 ... 12:
        iconList.append(QString(":/icons/skin/icons/calendar_month/month%1.svg").arg(month_num));
        break;
    default:
        //default , if month is invalid
        iconList.append(QString(":/icons/skin/icons/calendar_month/month4.svg"));
        break;
    }
    int day_num = QDate::currentDate().day();
    switch(day_num)
    {
    case 1 ... 31:
        iconList.append(QString(":/icons/skin/icons/calendar_day/day%1.svg").arg(day_num));
        break;
    default:
        //default , if day is invalid
        iconList.append(QString(":/icons/skin/icons/calendar_day/day23.svg"));
        break;
    }
    int week_num = QDate::currentDate().dayOfWeek();
    switch(week_num)
    {
    case 1 ... 7:
        iconList.append(QString(":/icons/skin/icons/calendar_week/week%1.svg").arg(week_num));
        break;
    default:
        //default , if week is invalid
        iconList.append(QString(":/icons/skin/icons/calendar_week/week4.svg"));
        break;
    }
    return iconList;
}


bool CalculateUtil::decreaseIconSize()
{
    const double value = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    const double ratio = std::max(0.2, value - 0.1);

    if (qFuzzyCompare(value, ratio)) return false;

    m_launcherGsettings->set("apps-icon-ratio", ratio);
    return true;
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
    Q_UNUSED(dockPosition);
    double scaleX = getScreenScaleX();
    double scaleY = getScreenScaleY();
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;

    calculateTextSize();

    int rows = 1;
    int containerW = containerSize.width();
    int containerH = containerSize.height();

    if (m_launcherGsettings->get(DisplayModeKey).toString() == DisplayModeCategory) {
        m_appColumnCount = 4;
        rows = 3;

        containerW = getAppBoxSize().width();
        //BlurBoxWidget上边距24,　分组标题高度70 ,　MultiPagesView页面切换按钮高度20 * scale;
        containerH = containerSize.height() - 24 - 60 - 20 * scale - DLauncher::DRAG_THRESHOLD;
    } else {
        m_appColumnCount = 7;
        rows = 4;

        containerW = containerSize.width();
        containerH = containerSize.height() - 20 * scale - DLauncher::DRAG_THRESHOLD;
    }

    //默认边距保留最小５像素
    m_appMarginLeft = 5;
    m_appMarginTop = 5;

    //去年默认边距后，计算每个Item区域的宽高
    int perItemWidth  = (containerW - m_appMarginLeft * 2) / m_appColumnCount;
    int perItemHeight = (containerH - m_appMarginTop) / rows;

    //因为每个Item是一个正方形的，所以取宽高中最小的值
    int perItemSize = qMin(perItemHeight,perItemWidth);

    //图标大小取区域的4 / 5
    m_appItemSize = perItemSize * 4 / 5;
    //其他区域为间隔区域
    m_appItemSpacing = (perItemSize - m_appItemSize) / 2;

    //重新计算左右上边距
    m_appMarginLeft = (containerW - m_appItemSize * m_appColumnCount - m_appItemSpacing * m_appColumnCount * 2) / 2 - 1;
    m_appMarginTop =  (containerH - m_appItemSize * rows - m_appItemSpacing * rows * 2) / 2;

    //计算字体大小
    m_appItemFontSize = m_appItemSize <= 80 ? 8 : qApp->font().pointSize() + 3;

    emit layoutChanged();
}

CalculateUtil::CalculateUtil(QObject *parent)
    : QObject(parent),
      m_dockInter(new DBusDock(this)),
      m_launcherGsettings(new QGSettings("com.deepin.dde.launcher",
                                         "/com/deepin/dde/launcher/", this))
{
    m_launcherInter = new DBusLauncher(this);
    isFullScreen = m_launcherInter->fullscreen();
}

void CalculateUtil::calculateTextSize()
{
    if (currentScreen()->geometry().width() > 1366) {
        m_navgationTextSize = 14;
        m_titleTextSize = 40;
    } else {
        m_navgationTextSize = 11;
        m_titleTextSize = 38;
    }
}

QScreen *CalculateUtil::currentScreen() const
{
    QScreen * s = qApp->primaryScreen();
    const QRect dockRect = m_dockInter->frontendRect();
    const auto ratio = qApp->devicePixelRatio();

    for (auto *screen : qApp->screens()) {
        const QRect &sg = screen->geometry();
        const QRect &rg = QRect(sg.topLeft(), sg.size() * ratio);
        if (rg.contains(dockRect.topLeft())) {
            s = screen;
            break;
        }
    }

    return  s;
}
