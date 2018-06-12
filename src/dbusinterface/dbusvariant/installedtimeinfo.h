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

#ifndef INSTALLEDTIMEINFO_H
#define INSTALLEDTIMEINFO_H

#include <QtDBus>

class InstalledTimeInfo
{
public:
    InstalledTimeInfo();
    ~InstalledTimeInfo();

    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const InstalledTimeInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const InstalledTimeInfo &info);
    friend QDataStream &operator<<(QDataStream &argument, const InstalledTimeInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, InstalledTimeInfo &info);
    friend const QDataStream &operator>>(QDataStream &argument, InstalledTimeInfo &info);

public:
    QString m_key;
    qlonglong m_installedTime;
};

typedef QList<InstalledTimeInfo> InstalledTimeInfoList;

Q_DECLARE_METATYPE(InstalledTimeInfo)
Q_DECLARE_METATYPE(InstalledTimeInfoList)

#endif // INSTALLEDTIMEINFO_H
