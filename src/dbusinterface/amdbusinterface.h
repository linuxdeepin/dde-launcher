// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AMDBusInterFACE_H_1456917835
#define AMDBusInterFACE_H_1456917835

#include "aminterface.h"

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#define AM_SERVICE_NAME "org.desktopspec.ApplicationManager1"
#define AM_SERVICE_PATH "/org/desktopspec/ApplicationManager1"
#define AM_INTERFACE_NAME "org.desktopspec.DBus.ObjectManager"
#define APP_INTERFACE_NAME "org.desktopspec.ApplicationManager1.Application"
/*
 * Proxy class for interface org.deepin.dde.ApplicationManager1
 */
class AMDBusInter: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;

        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != staticInterfaceName())
            return;

        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        foreach(const QString &prop, changedProps.keys()) {
        const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                const QMetaProperty property = self->property(i);
                if (property.name() == prop) {
                    Q_EMIT property.notifySignal().invoke(this);
                }
            }
        }
   }
public:
    static inline const char *staticInterfaceName()
    { return AM_INTERFACE_NAME; }

public:
    explicit AMDBusInter(QObject *parent = Q_NULLPTR);
    ~AMDBusInter();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<ObjectMap> GetManagedObjects()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetManagedObjects"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void InterfacesAdded(const QDBusObjectPath &object_path, ObjectInterfaceMap interfaces_and_properties);
    void InterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
};

#endif
