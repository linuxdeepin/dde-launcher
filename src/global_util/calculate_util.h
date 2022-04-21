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

#ifndef CALCULATE_UTIL_H
#define CALCULATE_UTIL_H

#include <QObject>
#include <QSize>
#include <QtCore>
#include <QGSettings>
#include <QScreen>
#include <DSysInfo>

#include "dbuslauncher.h"
#include "dbusdock.h"

#define ALL_APPS            0       // 全屏自由模式
#define GROUP_BY_CATEGORY   1       // 全屏分类模式
#define SEARCH              2       // 全屏搜索模式

DCORE_USE_NAMESPACE
class CalculateUtil : public QObject
{
    Q_OBJECT

signals:
    void layoutChanged() const;

public:
    static CalculateUtil *instance();

    inline int titleTextSize() const {return m_titleTextSize;}
    // NOTE: navgation text size animation max zoom scale is 1.2
    inline int navgationTextSize() const {return double(m_navgationTextSize) / 1.2;}
    inline int appColumnCount() const {return m_appColumnCount;}
    inline int appItemFontSize() const {return m_appItemFontSize;}
    inline int appItemSpacing() const {return m_appItemSpacing;}
    inline int appMarginLeft() const {return m_appMarginLeft;}
    inline int appMarginTop() const {return m_appMarginTop;}
    inline int appPageItemCount(AppsListModel::AppCategory category) const {return category >= AppsListModel::Dir ? m_categoryAppPageItemCount : m_appPageItemCount;}
    inline int appCategoryCount() const {return m_categoryCount;}
    inline QSize appItemSize() const { return QSize(m_appItemSize, m_appItemSize); }
    inline bool fullscreen() const {return isFullScreen;}
    inline int currentCategory() const {return m_currentCategory;}
    void setCurrentCategory(int category){m_currentCategory = category;}
    void setFullScreen(bool bFullScreen){isFullScreen = bFullScreen;}

    qreal getCurRatio();
    QSize appIconSize(int modelMode) const;
    QSize appIconSize() const;
    int displayMode() const;
    void setDisplayMode(const int mode);
    int calculateIconSize(const int mode);
    QSize getSearchWidgetSizeHint() const { return  m_searchWidgetHintSize; }
    void setSearchWidgetSizeHint(const QSize &size) { m_searchWidgetHintSize = size; }
    QSize getNavigationWidgetSizeHint() const { return m_navigationWidgetHintSize; }
    void setNavigationWidgetSizeHint(const QSize &size) { m_navigationWidgetHintSize = size; }

    bool increaseIconSize();
    bool decreaseIconSize();
    inline void increaseItemSize() { m_appItemSize += 16; }
    inline void decreaseItemSize() { m_appItemSize -= 16; }
    const DSysInfo::DeepinType DeepinType = DSysInfo::deepinType();
    const bool IsServerSystem = (DSysInfo::DeepinServer == DeepinType);

    inline int navigationHeight() { return 90; }
    QSize getAppBoxSize() ;
    QSize getScreenSize() const;
    double getScreenScaleX();
    double getScreenScaleY();

    QStringList calendarSelectIcon() const;

signals:
     void ratioChanged(double ratio);
     void loadWindowIcon();

public slots:
    void calculateAppLayout(const QSize &containerSize, const int currentmode = 0);

private:
    explicit CalculateUtil(QObject *parent);
    void calculateTextSize();
    QScreen *currentScreen() const;

private:
    static QPointer<CalculateUtil> INSTANCE;

    int m_appItemFontSize = 12;
    int m_appItemSpacing = 10;
    int m_appMarginLeft = 0;
    int m_appMarginTop = 0;
    int m_appItemSize = 130;
    int m_appColumnCount = 7;
    int m_navgationTextSize = 14;
    int m_appPageItemCount = 28;
    int m_titleTextSize = 40;
    int m_categoryAppPageItemCount = 12;
    int m_categoryCount = 11;
    int m_currentCategory = 4;
    bool isFullScreen;

    DBusLauncher *m_launcherInter;
    DBusDock *m_dockInter;

    QGSettings *m_launcherGsettings;

    QSize m_searchWidgetHintSize;
    QSize m_navigationWidgetHintSize;
};

#endif // CALCULATE_UTIL_H
