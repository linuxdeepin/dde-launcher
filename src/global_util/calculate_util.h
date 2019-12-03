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

#include "src/dbusinterface/dbuslauncher.h"

#define ALL_APPS            0
#define GROUP_BY_CATEGORY   1
#define SEARCH              2

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
    inline int appPageItemCount() const {return m_appPageItemCount;}
    inline QSize appItemSize() const { return QSize(m_appItemSize, m_appItemSize); }
    QSize appIconSize() const;
    int displayMode() const;
    void setDisplayMode(const int mode);

    bool increaseIconSize();
    bool decreaseIconSize();
    inline void increaseItemSize() { m_appItemSize += 16; }
    inline void decreaseItemSize() { m_appItemSize -= 16; }
    inline int navigationHeight() { return 117; }
public slots:
    void calculateAppLayout(const QSize &containerSize, const int dockPosition);

private:
    explicit CalculateUtil(QObject *parent);
    void calculateTextSize(const int screenWidth);

private:
    static QPointer<CalculateUtil> INSTANCE;

    int m_appItemFontSize = 12;
    int m_appItemSpacing = 10;
    int m_appItemSize = 130;
    int m_appColumnCount = 7;
    int m_navgationTextSize = 14;
    int m_titleTextSize = 15;
    int m_appPageItemCount = 28;

    DBusLauncher *m_launcherInter;
    QGSettings *m_launcherGsettings;
};

#endif // CALCULATE_UTIL_H
