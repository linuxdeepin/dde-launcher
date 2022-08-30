// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONITORINTERFACE_H_1439948634
#define MONITORINTERFACE_H_1439948634

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

struct MonitorMode {
    uint id;
    ushort width;
    ushort height;
    double rate;
};

typedef QList<ushort> UshortList;
typedef QList<MonitorMode> MonitorModeList;

Q_DECLARE_METATYPE(UshortList)
Q_DECLARE_METATYPE(MonitorMode)
Q_DECLARE_METATYPE(MonitorModeList)

QDBusArgument &operator<<(QDBusArgument &argument, const MonitorMode &mode);
const QDBusArgument &operator>>(const QDBusArgument &argument, MonitorMode &mode);

/*
 * Proxy class for interface com.deepin.daemon.Display.Monitor
 */
class MonitorInterface: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage &msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count()) {
            return;
        }
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != "com.deepin.daemon.Display.Monitor") {
            return;
        }
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys = changedProps.keys();
        foreach(const QString & prop, keys) {
            const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
    }
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.daemon.Display.Monitor"; }
    static inline const char *staticServiceName()
    { return "com.deepin.daemon.Display"; }

public:
    MonitorInterface(const QString &path, QObject *parent = 0);

    ~MonitorInterface();

    Q_PROPERTY(MonitorMode BestMode READ bestMode NOTIFY BestModeChanged)
    inline MonitorMode bestMode() const
    { return qvariant_cast< MonitorMode >(property("BestMode")); }

    Q_PROPERTY(MonitorMode CurrentMode READ currentMode NOTIFY CurrentModeChanged)
    inline MonitorMode currentMode() const
    { return qvariant_cast< MonitorMode >(property("CurrentMode")); }

    Q_PROPERTY(QString FullName READ fullName NOTIFY FullNameChanged)
    inline QString fullName() const
    { return qvariant_cast< QString >(property("FullName")); }

    Q_PROPERTY(ushort Height READ height NOTIFY HeightChanged)
    inline ushort height() const
    { return qvariant_cast< ushort >(property("Height")); }

    Q_PROPERTY(bool IsComposited READ isComposited NOTIFY IsCompositedChanged)
    inline bool isComposited() const
    { return qvariant_cast< bool >(property("IsComposited")); }

    Q_PROPERTY(QString Name READ name NOTIFY NameChanged)
    inline QString name() const
    { return qvariant_cast< QString >(property("Name")); }

    Q_PROPERTY(bool Opened READ opened NOTIFY OpenedChanged)
    inline bool opened() const
    { return qvariant_cast< bool >(property("Opened")); }

    Q_PROPERTY(QStringList Outputs READ outputs NOTIFY OutputsChanged)
    inline QStringList outputs() const
    { return qvariant_cast< QStringList >(property("Outputs")); }

    Q_PROPERTY(ushort Reflect READ reflect NOTIFY ReflectChanged)
    inline ushort reflect() const
    { return qvariant_cast< ushort >(property("Reflect")); }

    Q_PROPERTY(ushort Rotation READ rotation NOTIFY RotationChanged)
    inline ushort rotation() const
    { return qvariant_cast< ushort >(property("Rotation")); }

    Q_PROPERTY(ushort Width READ width NOTIFY WidthChanged)
    inline ushort width() const
    { return qvariant_cast< ushort >(property("Width")); }

    Q_PROPERTY(short X READ x NOTIFY XChanged)
    inline short x() const
    { return qvariant_cast< short >(property("X")); }

    Q_PROPERTY(short Y READ y NOTIFY YChanged)
    inline short y() const
    { return qvariant_cast< short >(property("Y")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<MonitorModeList> ListModes()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("ListModes"), argumentList);
    }

    inline QDBusPendingReply<UshortList> ListReflect()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("ListReflect"), argumentList);
    }

    inline QDBusPendingReply<UshortList> ListRotations()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("ListRotations"), argumentList);
    }

    inline QDBusPendingReply<> SetMode(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetMode"), argumentList);
    }

    inline QDBusPendingReply<> SetPos(short in0, short in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SetPos"), argumentList);
    }

    inline QDBusPendingReply<> SetReflect(ushort in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetReflect"), argumentList);
    }

    inline QDBusPendingReply<> SetRotation(ushort in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetRotation"), argumentList);
    }

    inline QDBusPendingReply<> SwitchOn(bool in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SwitchOn"), argumentList);
    }

Q_SIGNALS: // SIGNALS
// begin property changed signals
    void BestModeChanged();
    void CurrentModeChanged();
    void FullNameChanged();
    void HeightChanged();
    void IsCompositedChanged();
    void NameChanged();
    void OpenedChanged();
    void OutputsChanged();
    void ReflectChanged();
    void RotationChanged();
    void WidthChanged();
    void XChanged();
    void YChanged();
};

#endif
