// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorinterface.h"

/*
 * Implementation of interface class MonitorInterface
 */

QDBusArgument &operator<<(QDBusArgument &argument, const MonitorMode &mode)
{
    argument.beginStructure();
    argument << mode.id << mode.width << mode.height << mode.rate;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, MonitorMode &mode)
{
    argument.beginStructure();
    argument >> mode.id >> mode.width >> mode.height >> mode.rate;
    argument.endStructure();
    return argument;
}

MonitorInterface::MonitorInterface(const QString &path, QObject *parent)
    : QDBusAbstractInterface(staticServiceName(), path, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    qDBusRegisterMetaType<UshortList>();
    qDBusRegisterMetaType<MonitorMode>();
    qDBusRegisterMetaType<MonitorModeList>();

    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

MonitorInterface::~MonitorInterface()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}

