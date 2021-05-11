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
#include "monitorinterface.h"
#include "dbusdisplay.h"
#include "util.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

#include "constants.h"

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

/**
 * @brief CalculateUtil::setDisplayMode 处理全屏自由模式和全屏分类模式切换
 * @param mode 分类模式
 */
void CalculateUtil::setDisplayMode(const int mode)
{
    if (m_launcherGsettings)
        m_launcherGsettings->set(DisplayModeKey, mode == ALL_APPS ? DisplayModeFree : DisplayModeCategory);
}

QSize CalculateUtil::appIconSize() const
{
    if (!isFullScreen)
        return QSize(24, 24);

    QSize s(m_appItemSize, m_appItemSize);
    double ratio = m_launcherGsettings ? m_launcherGsettings->get("apps-icon-ratio").toDouble() : 0.6;
    return s * ratio;
}

/**
 * @brief CalculateUtil::getScreenScaleX
 * 获取屏幕宽度为1920的倍数
 * @return 获取屏幕宽度为1920的倍数
 */
double CalculateUtil::getScreenScaleX()
{
    int width  = currentScreen()->geometry().width();
    return double(width) / 1920;
}

/**
 * @brief CalculateUtil::getScreenScaleY
 * 取屏幕高度为1080的倍数
 * @return 取屏幕高度为1080的倍数
 */
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
    int height = int(currentScreen()->geometry().height() * 0.69);
    int width = int(currentScreen()->geometry().width() * 0.51);
    return  QSize(width, height);
}

bool CalculateUtil::increaseIconSize()
{
    if (!m_launcherGsettings)
        return false;

    const double value = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    const double ratio = std::min(0.6, value + 0.1);

    if (qFuzzyCompare(value, ratio)) return false;

    m_launcherGsettings->set("apps-icon-ratio", ratio);

    return true;
}

/**
 * @brief CalculateUtil::calendarSelectIcon
 * 根据系统时间设置日历app的月、周、日样式
 * @return 返回日历app的月、周、日资源路径list
 */
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
    if (!m_launcherGsettings)
        return false;

    const double value = m_launcherGsettings->get("apps-icon-ratio").toDouble();
    const double ratio = std::max(0.2, value - 0.1);

    if (qFuzzyCompare(value, ratio)) return false;

    m_launcherGsettings->set("apps-icon-ratio", ratio);

    return true;
}

/**
 * @brief CalculateUtil::displayMode 获取当前视图的展示模式
 * 两种模式: 全屏app自由模式、全屏app分类模式
 * @return
 */
int CalculateUtil::displayMode() const
{
    if (!m_launcherGsettings)
        return ALL_APPS;

    const QString displayMode = m_launcherGsettings->get(DisplayModeKey).toString();

    if (displayMode == DisplayModeCategory) {
        return GROUP_BY_CATEGORY;
    }

    return ALL_APPS;
}

/**
 * @brief CalculateUtil::calculateAppLayout
 * 计算app列表布局中控件大小参数
 * @param containerSize 控件存放的容器大小
 * @param currentmode 列表展示的模式
 */
void CalculateUtil::calculateAppLayout(const QSize &containerSize, const int currentmode)
{
    double scaleX = getScreenScaleX();
    double scaleY = getScreenScaleY();
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;

    calculateTextSize();

    int rows = 1;
    int containerW = containerSize.width();
    int containerH = containerSize.height();

    // 全屏App模式或者正在搜索列表以4行7列模式排布，全屏分类模式以4行3列模式排布
    if (!m_launcherGsettings || ((m_launcherGsettings->get(DisplayModeKey).toString() == DisplayModeFree) || currentmode == SEARCH)) {
        m_appColumnCount = 7;
        rows = 4;

        containerW = containerSize.width();
        containerH = containerSize.height() - 20 * scale - DLauncher::DRAG_THRESHOLD;
    } else {
        m_appColumnCount = 4;
        rows = 3;

        containerW = getAppBoxSize().width();
        //BlurBoxWidget上边距24,　分组标题高度70 ,　MultiPagesView页面切换按钮高度20 * scale;
        containerH = containerSize.height() - 24 - 60 - 20 * scale - DLauncher::DRAG_THRESHOLD;
    }

    // 默认边距保留最小５像素
    m_appMarginLeft = 5;
    m_appMarginTop = 5;

    // 去年默认边距后，计算每个Item区域的宽高
    int perItemWidth  = (containerW - m_appMarginLeft * 2) / m_appColumnCount;
    int perItemHeight = (containerH - m_appMarginTop) / rows;

    // 因为每个Item是一个正方形的，所以取宽高中最小的值
    int perItemSize = qMin(perItemHeight,perItemWidth);

    // 图标大小取区域的4 / 5
    m_appItemSize = perItemSize * 4 / 5;
    // 其他区域为间隔区域
    m_appItemSpacing = (perItemSize - m_appItemSize) / 2;

    // 重新计算左右上边距
    m_appMarginLeft = (containerW - m_appItemSize * m_appColumnCount - m_appItemSpacing * m_appColumnCount * 2) / 2 - 1;
    m_appMarginTop =  (containerH - m_appItemSize * rows - m_appItemSpacing * rows * 2) / 2;

    // 计算字体大小
    m_appItemFontSize = m_appItemSize <= 80 ? 8 : qApp->font().pointSize() + 3;

    emit layoutChanged();
}

/**
 * @brief CalculateUtil::CalculateUtil
 * 计算屏幕、应用列表布局、日历app等大小、间隔等
 * @param parent
 */
CalculateUtil::CalculateUtil(QObject *parent)
    : QObject(parent)
    , m_dockInter(new DBusDock(this))
    , m_launcherGsettings(SettingsPtr("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", this))
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
