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

#include "appslistmodel.h"
#include "constants.h"

#include <DSysInfo>

#include <QObject>
#include <QGSettings>
#include <QScreen>

DCORE_USE_NAMESPACE

#ifdef USE_AM_API
class AMDBusLauncherInter;
class AMDBusDockInter;
#else
class DBusLauncher;
class DBusDock;
#endif

class CalculateUtil : public QObject
{
    Q_OBJECT

signals:
    void layoutChanged() const;

public:
    static CalculateUtil *instance();

    inline int titleTextSize() const { return m_titleTextSize; }
    inline int navgationTextSize() const { return double(m_navgationTextSize) / 1.2; }
    inline int appColumnCount() const { return DLauncher::DEFAULT_COLUMNS; }
    inline int appItemFontSize() const { return m_appItemFontSize; }
    inline int appItemSpacing() const { return m_appItemSpacing; }
    inline int appMarginLeft() const { return m_appMarginLeft; }
    inline int appMarginTop() const { return m_appMarginTop; }
    inline int appPageItemCount(AppsListModel::AppCategory category) const { return category == AppsListModel::Dir ? m_categoryAppPageItemCount : m_appPageItemCount; }
    inline int appCategoryCount() const { return m_categoryCount; }
    inline QSize appItemSize() const { return QSize(m_appItemSize, m_appItemSize); }
    inline bool fullscreen() const { return m_isFullScreen; }
    inline int currentCategory() const  { return m_currentCategory; }

    void setCurrentCategory(int category) { m_currentCategory = category; }
    void setFullScreen(bool bFullScreen) { m_isFullScreen = bFullScreen; }

    qreal getCurRatio();
    QSize appIconSize(int modelMode) const;
    QSize getSearchWidgetSizeHint() const { return  m_searchWidgetHintSize; }
    void setSearchWidgetSizeHint(const QSize &size) { m_searchWidgetHintSize = size; }

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

#ifdef USE_AM_API
    AMDBusLauncherInter *m_amDbusLauncher;
    AMDBusDockInter *m_amDbusDockInter;
#else
    DBusLauncher *m_launcherInter;
    DBusDock *m_dockInter;
#endif

    bool m_isFullScreen;
    QGSettings *m_launcherGsettings;
    QSize m_searchWidgetHintSize;

    // TODO: 后面可能会删减的变量, 暂时使用普通数值初始化
    int m_appItemFontSize;
    int m_appItemSpacing;
    int m_appMarginLeft;
    int m_appMarginTop;
    int m_appItemSize;
    int m_navgationTextSize;
    int m_appPageItemCount;
    int m_titleTextSize;
    int m_categoryAppPageItemCount;
    int m_categoryCount;
    int m_currentCategory;
};

#endif // CALCULATE_UTIL_H
