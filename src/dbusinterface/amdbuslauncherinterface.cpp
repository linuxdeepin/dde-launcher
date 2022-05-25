/*
 * Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     songwentao <songwentao@uniontech.com>
 *
 * Maintainer: songwentao <songwentao@uniontech.com>
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

#include "amdbuslauncherinterface.h"

/*
 * Implementation of interface class AMDBusLauncherInter
 */

AMDBusLauncherInter::AMDBusLauncherInter(QObject *parent)
    : QDBusAbstractInterface(INTERFACE_NAME, SERVICE_PATH, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    CategoryInfo::registerMetaType();
    FrequencyInfo::registerMetaType();
    ItemInfo_v2::registerMetaType();
    InstalledTimeInfo::registerMetaType();

    QDBusConnection::sessionBus().connect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

AMDBusLauncherInter::~AMDBusLauncherInter()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

