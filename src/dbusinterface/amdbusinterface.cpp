// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "amdbusinterface.h"

/*
 * Implementation of interface class AMDBusInter
 */
static void registerType()
{
    qRegisterMetaType<PropMap>("PropMap");
    qDBusRegisterMetaType<PropMap>();

    qRegisterMetaType<ObjectMap>("ObjectMap");
    qDBusRegisterMetaType<ObjectMap>();

    qRegisterMetaType<ObjectInterfaceMap>("ObjectInterfaceMap");
    qDBusRegisterMetaType<ObjectInterfaceMap>();
}
Q_CONSTRUCTOR_FUNCTION(registerType)

AMDBusInter::AMDBusInter(QObject *parent)
    : QDBusAbstractInterface(AM_SERVICE_NAME, AM_SERVICE_PATH, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

AMDBusInter::~AMDBusInter()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

