/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c DBusDock -p dbusdock com.deepin.dde.daemon.Dock.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef DBUSDOCK_H_1465782143
#define DBUSDOCK_H_1465782143

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include "dockrect.h"
#include <QDebug>

/*
 * Proxy class for interface com.deepin.dde.daemon.Dock
 */
class DBusDock: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="com.deepin.dde.daemon.Dock")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        foreach(const QString &prop, changedProps.keys()) {
        const QMetaObject* self = metaObject();
            for (int i=self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
 	            Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
   }
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.dde.daemon.Dock"; }

public:
    explicit DBusDock(QObject *parent = 0);

    ~DBusDock();

    Q_PROPERTY(uint ActiveWindow READ activeWindow NOTIFY ActiveWindowChanged)
    inline uint activeWindow() const
    { return qvariant_cast< uint >(property("ActiveWindow")); }

    Q_PROPERTY(QList<QDBusObjectPath> Entries READ entries NOTIFY EntriesChanged)
    inline QList<QDBusObjectPath> entries() const
    { return qvariant_cast< QList<QDBusObjectPath> >(property("Entries")); }

//    Q_PROPERTY(QStringList DockedApps READ dockedApps NOTIFY DockedAppsChanged)
//    inline QStringList dockedApps() const
//    { return qvariant_cast< QStringList >(property("DockedApps")); }

    Q_PROPERTY(int DisplayMode READ displayMode WRITE setDisplayMode NOTIFY DisplayModeChanged)
    inline int displayMode() const
    { return int(qvariant_cast< int >(property("DisplayMode"))); }
    inline void setDisplayMode(int value)
    { setProperty("DisplayMode", QVariant::fromValue(int(value))); }

    Q_PROPERTY(int HideMode READ hideMode WRITE setHideMode NOTIFY HideModeChanged)
    inline int hideMode() const
    { return qvariant_cast< int >(property("HideMode")); }
    inline void setHideMode(int value)
    { setProperty("HideMode", QVariant::fromValue(value)); }

    Q_PROPERTY(int HideState READ hideState NOTIFY HideStateChanged)
    inline int hideState() const
    { return int(qvariant_cast< int >(property("HideState"))); }

    Q_PROPERTY(int Position READ position WRITE setPosition NOTIFY PositionChanged)
    inline int position() const
    { return int(qvariant_cast< int >(property("Position"))); }
    inline void setPosition(int value)
    { setProperty("Position", QVariant::fromValue(int(value))); }

    Q_PROPERTY(quint32 IconSize READ iconSize WRITE setIconSize NOTIFY IconSizeChanged)
    inline quint32 iconSize() const
    { return qvariant_cast< quint32 >(property("IconSize")); }
    inline void setIconSize(quint32 value)
    { setProperty("IconSize", QVariant::fromValue(value)); }

    Q_PROPERTY(DockRect FrontendWindowRect READ frontendRect NOTIFY FrontendRectChanged)
    inline DockRect frontendRect() const
    { return qvariant_cast< DockRect >(property("FrontendWindowRect")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> ActivateWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("ActivateWindow"), argumentList);
    }

    inline QDBusPendingReply<> CloseWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("CloseWindow"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetEntryIDs()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetEntryIDs"), argumentList);
    }

    inline QDBusPendingReply<> MoveEntry(const int oldIndex, const int newIndex)
    {
        QList<QVariant> args;
        args << oldIndex << newIndex;

        return asyncCallWithArgumentList(QStringLiteral("MoveEntry"), args);
    }

    inline QDBusPendingReply<bool> RequestDock(const QString &appDesktop, const int index = -1)
    {
        QList<QVariant> args;
        args << appDesktop << index;

        return asyncCallWithArgumentList(QStringLiteral("RequestDock"), args);
    }

    inline QDBusPendingReply<bool> IsDocked(const QString &appDesktop)
    {
        QList<QVariant> args;
        args << appDesktop;

        return asyncCallWithArgumentList(QStringLiteral("IsDocked"), args);
    }

    inline QDBusPendingReply<bool> RequestUndock(const QString &appDesktop)
    {
        QList<QVariant> args;
        args << appDesktop;

        return asyncCallWithArgumentList(QStringLiteral("RequestUndock"), args);
    }

    inline QDBusPendingReply<> SetFrontendWindowRect(const int x, const int y, const quint32 width, const quint32 height)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(x) << QVariant::fromValue(y) << QVariant::fromValue(width) << QVariant::fromValue(height);
        return asyncCallWithArgumentList(QStringLiteral("SetFrontendWindowRect"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void EntryAdded(const QDBusObjectPath &entryPath, const int index);
    void EntryRemoved(const QString &entryId);
    void ServiceRestarted();
// begin property changed signals
void ActiveWindowChanged();
void DisplayModeChanged();
void EntriesChanged();
void HideModeChanged();
void HideStateChanged();
void PositionChanged();
void IconSizeChanged();
void DockedAppsChanged();
void FrontendRectChanged();
};

namespace com {
  namespace deepin {
    namespace dde {
      namespace daemon {
        typedef ::DBusDock Dock;
      }
    }
  }
}
#endif
