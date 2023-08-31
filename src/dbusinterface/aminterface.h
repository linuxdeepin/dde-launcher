// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AMINTERFACE_H
#define AMINTERFACE_H

#include "iteminfo.h"

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#include <DDBusInterface>
#include <DFileWatcher>

DCORE_USE_NAMESPACE

using IconMap = QMap<QString, QMap<uint, QMap<QString, QDBusUnixFileDescriptor>>>;
using ObjectInterfaceMap = QMap<QString, QVariantMap>;
using ObjectMap = QMap<QDBusObjectPath, ObjectInterfaceMap>;
using PropMap = QMap<QString, QMap<QString, QString>>;

Q_DECLARE_METATYPE(ObjectInterfaceMap)
Q_DECLARE_METATYPE(ObjectMap)
Q_DECLARE_METATYPE(PropMap)

/* AM */
class ItemInfo_v3
{
    // 应用类型
    enum Categorytype {
        CategoryInternet,
        CategoryChat,
        CategoryMusic,
        CategoryVideo,
        CategoryGraphics,
        CategoryGame,
        CategoryOffice,
        CategoryReading,
        CategoryDevelopment,
        CategorySystem,
        CategoryOthers,
        CategoryErr,
    };
public:
    ItemInfo_v3();
    PropMap m_actionName;
    QStringList m_actions;
    QStringList m_categories;
    bool m_autoStart;
    PropMap m_displayName;
    QString m_id;
    PropMap m_icons;
    QList<QDBusObjectPath> m_instances;
    qulonglong m_lastLaunchedTime;
    bool m_X_Flatpak;
    bool m_X_linglong;
    qulonglong m_installedTime;
    QString m_objectPath;

    Categorytype category() const;
    friend QDebug operator<<(QDebug argument, const ItemInfo_v3 &info);
    bool operator==(const ItemInfo_v3 &other) const;
};

typedef QList<ItemInfo_v3> ItemInfoList_v3;

class DBusProxy : public QObject
{
    Q_OBJECT
public:
    explicit DBusProxy(const QString &service,
                       const QString &path,
                       const QString &interface,
                       QObject *parent = nullptr);
    Q_PROPERTY(bool AutoStart2 READ autoStart NOTIFY AutoStartChanged)
    bool autoStart() const;
signals:
    void AutoStartChanged(bool ) const;
    void InterfacesAdded(const QDBusObjectPath &, const ObjectInterfaceMap &);
private:
    DDBusInterface *m_dBusDisplayInter;
};

class AMDBusInter;

class AMInter: public QObject
{
    Q_OBJECT
public:
    static AMInter *instance();

    bool isAMReborn();
    ItemInfoList_v2 GetAllItemInfos();
    QStringList GetAllNewInstalledApps();
    void RequestRemoveFromDesktop(const QString &in0);
    void RequestSendToDesktop(const QString &in0);
    bool IsItemOnDesktop(QString appId);
    void Launch(const QString &in0);
    bool isLingLong(const QString &appId) const;
    void uninstallApp(const QString &name, bool isLinglong = false);
    void setDisableScaling(QString appId, bool value);
    bool getDisableScaling(QString appId);
    QStringList autostartList() const;
    bool addAutostart(const QString &in0);
    bool removeAutostart(const QString &in0);

Q_SIGNALS: // SIGNALS
    void NewAppLaunched(const QString &in0);
    void autostartChanged(const QString &in0, const QString &in1);

public Q_SLOTS:
    void onInterfaceAdded(const QDBusObjectPath &object_path, const ObjectInterfaceMap &interfaces);
    void onRequestRemoveFromDesktop(QDBusPendingCallWatcher* watcher);
    void onAutoStartChanged(const bool isAutoStart, const QString &id);

private:
    AMInter(QObject *parent = Q_NULLPTR);

    void GetAllInfos();
    void buildConnect();
    bool ll_uninstall(const QString &appid);
    bool lastore_uninstall(const QString &pkgName);
    bool shouldDisableScaling(QString appId);
    QStringList getDisableScalingApps();
    void setDisableScalingApps(const QStringList &value);
    void monitorAutoStartFiles();
    AMDBusInter *m_amDbusInter;
    ItemInfoList_v2 m_itemInfoList_v2;
    ItemInfoList_v3 m_itemInfoList_v3;
    QStringList m_autoStartApps;
    DFileWatcher *m_autoStartFileWather;
};

#endif
