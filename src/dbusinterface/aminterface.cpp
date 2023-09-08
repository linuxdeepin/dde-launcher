// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aminterface.h"
#include "appsmanager.h"
#include "util.h"

#include <mutex>

static const QString KeyAppsDisableScaling = "Apps_Disable_Scaling";
static const QString AMServiceName = "org.desktopspec.ApplicationManager1";
static const QString AMServicePath  = "/org/desktopspec/ApplicationManager1";
static const QString AMInterfaceName  = "org.desktopspec.DBus.ObjectManager";
static const QString APPInterfaceName  = "org.desktopspec.ApplicationManager1.Application";

static void registerType()
{
    qDBusRegisterMetaType<QMap<QString, QString>>();
    qDBusRegisterMetaType<ObjectInterfaceMap>();

    qRegisterMetaType<PropMap>("PropMap");
    qDBusRegisterMetaType<PropMap>();

    qRegisterMetaType<ObjectMap>("ObjectMap");
    qDBusRegisterMetaType<ObjectMap>();

    qRegisterMetaType<ObjectInterfaceMap>("ObjectInterfaceMap");
    qDBusRegisterMetaType<ObjectInterfaceMap>();
}
Q_CONSTRUCTOR_FUNCTION(registerType)

ItemInfo_v3::ItemInfo_v3()
{
}

ItemInfo_v3::Categorytype ItemInfo_v3::category() const
{
    static QMap<QString, Categorytype> categoryTypeMap = {{"internet", CategoryInternet}, {"chat", CategoryChat}, {"music", CategoryMusic},
                         {"video", CategoryMusic}, {"graphic", CategoryGraphics}, {"office", CategoryOffice},
                         {"game", CategoryGame}, {"reading", CategoryReading}, {"development", CategoryDevelopment},
                         {"system", CategorySystem}};
    for (auto value : m_categories) {
        QString category = value.toLower();
        if (categoryTypeMap.contains(category)) {
            return categoryTypeMap.value(category);
        }
    }
    return CategoryOthers;
}

QDebug operator<<(QDebug argument, const ItemInfo_v3 &info)
{
    argument << "actionName: " << info.m_actionName << ", actions: " << info.m_actions
             << ", autoStart: " << info.m_autoStart << ", displayName: " << info.m_displayName
             << ", ID: " << info.m_id << ", icons: " << info.m_icons
             << ", instances: " << info.m_instances << ", lastLaunchedTime: " << info.m_lastLaunchedTime
             << ", categories: " << info.m_categories
             << ", X_Flatpak: " << info.m_X_Flatpak << ", X_linglong: " << info.m_X_linglong
             << ", installedTime: " << info.m_installedTime;

    return argument;
}

bool ItemInfo_v3::operator==(const ItemInfo_v3 &other) const
{
    return (this->m_actionName == other.m_actionName) &&
           (this->m_actions == other.m_actions) &&
           (this->m_autoStart == other.m_autoStart) &&
           (this->m_displayName == other.m_displayName) &&
           (this->m_id == other.m_id) &&
           (this->m_icons == other.m_icons) &&
           (this->m_instances == other.m_instances) &&
           (this->m_lastLaunchedTime == other.m_lastLaunchedTime) &&
           (this->m_categories == other.m_categories) &&
           (this->m_X_Flatpak == other.m_X_Flatpak) &&
           (this->m_X_linglong == other.m_X_linglong) &&
           (this->m_installedTime == other.m_installedTime);
}

AMInter::AMInter(QObject *parent)
    : QObject(parent)
    , m_objManagerDbusInter(new AppManagerDBusProxy(this))
{
    updateAllInfos();
    buildConnect();
    monitorAutoStartFiles();
    connect(m_objManagerDbusInter, &AppManagerDBusProxy::InterfacesAdded, this, [this] (const QDBusObjectPath &objectPath, ObjectInterfaceMap objs){
        qInfo() << objectPath.path() << objs;
        ItemInfo_v3 info_v3 = itemInfoV3(objs);
        m_infos.insert(objectPath.path(), info_v3);
        ItemInfo_v2 info_v2 = itemInfoV2(info_v3);
        Q_EMIT itemChanged("created", info_v2, info_v3.category());
    });
    connect(m_objManagerDbusInter, &AppManagerDBusProxy::InterfacesRemoved, this, [] (const QDBusObjectPath &object_path, const QStringList &interface){
        qInfo() << object_path.path() << interface;
        // TODO: util AM reborn finish the function!
    });
}

AMInter *AMInter::instance()
{
    static AMInter AMInter;
    return &AMInter;
}

bool AMInter::isAMReborn()
{
    static int result = -1;
    if (result < 0) {
        if (qEnvironmentVariableIsSet("DISABLE_AM_REBORN")) {
            result = !(qEnvironmentVariable("DISABLE_AM_REBORN") == "1");
        } else {
            auto services = QDBusConnection::sessionBus().interface()->registeredServiceNames().value();
            result = std::any_of(services.begin(), services.end(), [=](const QString &name){
                return name == AMServiceName;
            });
        }
    }
    return result;
}

ItemInfoList_v2 AMInter::allInfos()
{
    ItemInfoList_v2 itemInfoList_v2;
    const auto itemInfoList_V3 = m_infos.values();
    for (auto info : itemInfoList_V3) {
        itemInfoList_v2.append(itemInfoV2(info));
    }
    return itemInfoList_v2;
}

QStringList AMInter::allNewInstalledApps() const
{
    QStringList apps;
    const auto itemInfoList_V3 = m_infos.values();
    for (auto info : itemInfoList_V3) {
        if (info.m_lastLaunchedTime == 0) {
            apps.append(info.m_id);
        }
    }
    return apps;
}

void AMInter::requestRemoveFromDesktop(const QString &appId)
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return;

    QDBusInterface iface(AMServiceName, m_infos.key(itemInfo), APPInterfaceName, QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        QDBusReply<bool> reply = iface.call("RemoveFromDesktop");
        if (reply.isValid()) {
            qDebug() << "RemoveFromDesktop:" << appId << reply.value();
        } else {
            qWarning() << "RemoveFromDesktop failed!";
        }
    } else {
        qWarning() << "RemoveFromDesktop" << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
}

void AMInter::requestSendToDesktop(const QString &appId)
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return;

    QDBusInterface iface(AMServiceName, m_infos.key(itemInfo), APPInterfaceName, QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        QDBusReply<bool> reply = iface.call("SendToDesktop");
        if (reply.isValid()) {
            qDebug() << "SendToDesktop:" << appId << reply.value();
        } else {
            qWarning() << "SendToDesktop failed!" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
    } else {
        qWarning() << "SendToDesktop" << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
}

void AMInter::updateAllInfos()
{
    QDBusPendingCall call = m_objManagerDbusInter->GetManagedObjects();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(getManagedObjectsFinished(QDBusPendingCallWatcher*)));

}

void AMInter::buildConnect()
{
    for (auto path : m_infos.keys()) {
        ApplicationDBusProxy *autoStartInter = new ApplicationDBusProxy(path, this);
        connect(autoStartInter, &ApplicationDBusProxy::InterfacesAdded, this, &AMInter::onInterfaceAdded);
    }
}

bool AMInter::isOnDesktop(const QString &appId)
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return false;

    QDBusInterface iface(AMServiceName, m_infos.key(itemInfo), APPInterfaceName, QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        QVariant value = iface.property("isOnDesktop");
        return value.toBool();
    } else {
        qWarning() << "isOnDesktop" << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
    return false;
}

void AMInter::launch(const QString &desktop)
{
    const auto itemInfo = itemInfoV3(desktop);
    if (itemInfo.isInvalid())
        return;

    QDBusInterface iface(AMServiceName, m_infos.key(itemInfo), APPInterfaceName, QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        QVariantList arguments;
        arguments << QString();
        arguments << QStringList();
        QMap<QString, QVariant> arg3;
        arguments << QVariant::fromValue(arg3);
        QDBusReply<QDBusObjectPath> reply = iface.callWithArgumentList(QDBus::Block, "Launch", arguments);
        if (reply.isValid()) {
            qDebug() << "Launch:" << desktop << reply.value().path();
        } else {
            qWarning() << "Launch failed!" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
    } else {
        qWarning() << "Launch" << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
}

bool AMInter::isLingLong(const QString &appId) const
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return false;
    return itemInfo.m_X_linglong;
}

void AMInter::onInterfaceAdded(const QDBusObjectPath &objPath, const ObjectInterfaceMap &interfaces)
{
    QString path = objPath.path();
    auto index = path.lastIndexOf("/");
    path.truncate(index);
    const QStringList objPaths = m_infos.keys();
    if (!objPaths.contains(path))
        return;

    Q_EMIT newAppLaunched(m_infos.value(path).m_id);
}

void AMInter::onRequestRemoveFromDesktop(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QString> reply = *watcher;
    if (!reply.isError()) {
        QString name = reply.argumentAt<0>();
        qDebug()<< "QDBusPendingReply name = " << name;
    }
    watcher->deleteLater();
}

void AMInter::onAutoStartChanged(const bool isAutoStart, const QString& id)
{
    if (isAutoStart) {
        Q_EMIT autostartChanged("added", id);
    } else {
        Q_EMIT autostartChanged("deleted", id);
    }
}

void AMInter::getManagedObjectsFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<ObjectMap> reply = *watcher;
    if (!reply.isError()) {
        ObjectMap infos = reply.argumentAt<0>();
        for (auto iter = infos.begin(); iter != infos.end(); ++iter) {
            const auto &objPath = iter.key();
            const auto &objs = iter.value();
            auto info_v3 = itemInfoV3(objs);
            m_infos.insert(objPath.path(), info_v3);
            if (info_v3.m_autoStart) {
                m_autoStartApps.append(info_v3.m_id);
            }
        }
    }
    watcher->deleteLater();
}

void AMInter::uninstallApp(const QString &name, const bool isLinglong)
{
    bool ret = false;
    if (isLinglong) {
        ret = llUninstall(name);
    } else {
        ret = lastoreUninstall(name);
    }

    if (ret) {
        AppsManager::instance()->abandonStashedItem(name);
    } else {
        AppsManager::instance()->onUninstallFail(name);
    }

    // 刷新各列表的分页信息
    emit AppsManager::instance()->dataChanged(AppsListModel::FullscreenAll);
}

void AMInter::setDisableScaling(const QString &appId, const bool value)
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return;

    QStringList apps = disableScalingApps();
    if (value) {
        if (apps.contains(appId)) {
            return;
        }
        apps.append(appId);
    } else {
        bool found = false;
        for (auto iter = apps.begin(); iter != apps.end(); iter++) {
            if (*iter == appId) {
                found = true;
                apps.erase(iter);
                break;
            }
        }
        if (!found)
            return;
    }

    setDisableScalingApps(apps);
}

bool AMInter::disableScaling(const QString &appId)
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return false;

    if (disableScalingApps().contains(appId))
        return true;

    return false;
}

QStringList AMInter::autostartList() const
{
    return m_autoStartApps;
}

bool AMInter::addAutostart(const QString &appId)
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return false;

    QDBusInterface iface(AMServiceName, m_infos.key(itemInfo), APPInterfaceName, QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        bool ret = iface.setProperty("AutoStart", true);
        return ret;
    } else {
        qWarning() << "AutoStart" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return false;
    }
}

bool AMInter::removeAutostart(const QString &appId)
{
    const auto itemInfo = itemInfoV3(appId);
    if (itemInfo.isInvalid())
        return false;

    QDBusInterface iface(AMServiceName, m_infos.key(itemInfo), APPInterfaceName, QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        bool ret = iface.setProperty("AutoStart", false);
        return ret;
    } else {
        qWarning() << "AutoStart" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return false;
    }
}

bool AMInter::llUninstall(const QString &appid)
{
    QProcess ll;
    ll.setProgram(QStandardPaths::findExecutable("ll-cli"));
    ll.setArguments(QStringList() << "uninstall" << appid);
    ll.start();
    ll.waitForFinished();

    if (ll.exitCode() != 0) {
        qWarning() << "uninstall " << appid << "failed. " << ll.errorString();
        return false;
    }

    return true;
}

bool AMInter::lastoreUninstall(const QString &pkgName)
{
    QDBusInterface lastoreDbus("org.deepin.dde.Lastore1",
                               "/org/deepin/dde/Lastore1",
                               "org.deepin.dde.Lastore1.Manager",
                               QDBusConnection::systemBus());

    QDBusReply<bool> reply = lastoreDbus.call("PackageExists", pkgName);
    // 包未安装时
    if (!(reply.isValid() && reply.value())) {
        qWarning() << "check packget" << pkgName << " exists failed" << reply.error();
        return false;
    }

    // RemovePackage 接口有白名单，需要加上 /usr/bin/dde-launcher
    QDBusReply<QDBusObjectPath> rmReply = lastoreDbus.call(QDBus::Block,
                                                           "RemovePackage",
                                                           QString("%1-rm-job").arg(pkgName), // jobname ?
                                                           pkgName);

    if (!rmReply.isValid() || rmReply.value().path().isEmpty()) {
        qWarning() << "RemovePackage failed: " << rmReply.error();
        return false;
    }

    return true;
}

bool AMInter::shouldDisableScaling(const QString &appId)
{
    return disableScalingApps().contains(appId);
}

QStringList AMInter::disableScalingApps() const
{
    QStringList ret;

    QVariant value = ConfigWorker::getValue(KeyAppsDisableScaling, ret);
    auto apps = value.toList();
    for (auto app : apps) {
        ret.append(app.toString());
    }

    return ret;
}

void AMInter::setDisableScalingApps(const QStringList &value)
{
    QList<QVariant> apps;
    for (const auto &app : value)
        apps.push_back(app);

    ConfigWorker::setValue(KeyAppsDisableScaling, apps);
}

void AMInter::monitorAutoStartFiles()
{
    m_autoStartFileWather = new DFileWatcher( QDir::homePath() + "/.config/autostart");
    connect(m_autoStartFileWather, &DBaseFileWatcher::subfileCreated, this, [this](const QUrl &url){
        QString desktop = url.toString().split('/').last();
        desktop.truncate(desktop.lastIndexOf('.'));
        Q_EMIT autostartChanged("added", desktop);
        qWarning() << "autostartChanged added" << desktop;
    });
    connect(m_autoStartFileWather, &DBaseFileWatcher::fileDeleted, this, [this](const QUrl &url){
        QString desktop = url.toString().split('/').last();
        desktop.truncate(desktop.lastIndexOf('.'));
        Q_EMIT autostartChanged("deleted", desktop);
        qWarning() << "autostartChanged deleted" << desktop;
    });
    bool ret = m_autoStartFileWather->startWatcher();
    if (!ret) {
        qWarning() << "autoStartFileWather failed!";
    }
}

ItemInfo_v3 AMInter::itemInfoV3(const ObjectInterfaceMap &objs) const
{
    ItemInfo_v3 info_v3;
    for (auto objIter = objs.begin(); objIter != objs.end(); ++objIter) {
        const auto obj = objIter.key();
        if (obj != "org.desktopspec.ApplicationManager1.Application")
            continue;
        const auto interProps = objIter.value();
        for (auto interPropIter = interProps.begin(); interPropIter != interProps.end(); ++interPropIter) {
            const auto key = interPropIter.key();
            if (key == "ActionName") {
                PropMap value = qdbus_cast<PropMap>(interPropIter.value());
                info_v3.m_actionName = value;
            } else if (key == "Actions") {
                QStringList value = qdbus_cast<QStringList>(interPropIter.value());
                info_v3.m_actions = value;
            } else if (key == "AutoStart") {
                bool value = qdbus_cast<bool>(interPropIter.value());
                info_v3.m_autoStart = value;
            } else if (key == "Categories") {
                QStringList value = qdbus_cast<QStringList>(interPropIter.value());
                info_v3.m_categories = value;
            } else if (key == "DisplayName") {
                PropMap value = qdbus_cast<PropMap>(interPropIter.value());
                info_v3.m_displayName = value;
            } else if (key == "ID") {
                QString value = qdbus_cast<QString>(interPropIter.value());
                info_v3.m_id = value;
            } else if (key == "Icons") {
                PropMap value = qdbus_cast<PropMap>(interPropIter.value());
                info_v3.m_icons = value;
            } else if (key == "Instances") {
                QList<QDBusObjectPath> value = qdbus_cast<QList<QDBusObjectPath>>(interPropIter.value());
                info_v3.m_instances = value;
            } else if (key == "LastLaunchedTime") {
                qulonglong value = qdbus_cast<qulonglong>(interPropIter.value());
                info_v3.m_lastLaunchedTime = value;
            } else if (key == "X_Flatpak") {
                bool value = qdbus_cast<bool>(interPropIter.value());
                info_v3.m_X_Flatpak = value;
            } else if (key == "X_linglong") {
                bool value = qdbus_cast<bool>(interPropIter.value());
                info_v3.m_X_linglong = value;
            } else if (key == "installedTime") {
                qulonglong value = qdbus_cast<qulonglong>(interPropIter.value());
                info_v3.m_installedTime = value;
            } else if (key == "AutoStart") {
                qulonglong value = qdbus_cast<qulonglong>(interPropIter.value());
                info_v3.m_installedTime = value;
            }
        }
    }
    return info_v3;
}

ItemInfo_v3 AMInter::itemInfoV3(const QString &appId) const
{
    const auto itemInfoList_v3 = m_infos.values();
    auto it = std::find_if(itemInfoList_v3.begin(), itemInfoList_v3.end(), [appId](ItemInfo_v3 info){
        return info.m_id == appId;
    });
    if (it == itemInfoList_v3.end())
        return ItemInfo_v3();

    return *it;
}

ItemInfo_v2 AMInter::itemInfoV2(const ItemInfo_v3 &itemInfoV3)
{
    ItemInfo_v2 info_v2;

    QString defaultDisplayName;
    QString zh_displayName;
    for (auto displayNameKey : itemInfoV3.m_displayName.keys()) {
        if (displayNameKey == "Name") {
            auto displayNameValues = itemInfoV3.m_displayName.value(displayNameKey);
            for (auto key : displayNameValues.keys()) {
                if (key == "default") {
                    defaultDisplayName = displayNameValues.value(key);
                }
                if (key == "zh_CN") {
                    zh_displayName = displayNameValues.value(key);
                }
            }
        }
    }
    QString iconName;
    for (auto iconKey : itemInfoV3.m_icons.keys()) {
        if (iconKey == "default") {
            auto iconValues = itemInfoV3.m_icons.value(iconKey);
            for (auto key : iconValues.keys()) {
                if (key == "default") {
                    iconName = iconValues.value(key);
                }
            }
        }
    }
    info_v2.m_desktop = itemInfoV3.m_id;
    info_v2.m_name = zh_displayName.isEmpty() ? defaultDisplayName : zh_displayName;
    info_v2.m_key = itemInfoV3.m_id;
    info_v2.m_iconKey = iconName;
    info_v2.m_keywords.append(defaultDisplayName);
    info_v2.m_categoryId = itemInfoV3.category();

    return info_v2;
}

ApplicationDBusProxy::ApplicationDBusProxy(const QString &path, QObject *parent)
    : QObject(parent)
    , m_applicationDBus(new DDBusInterface(AMServiceName, path, AMInterfaceName, QDBusConnection::sessionBus(), this))
{
}

AppManagerDBusProxy::AppManagerDBusProxy(QObject *parent)
    : QObject(parent)
    , m_objectManagerDBus(new DDBusInterface(AMServiceName, AMServicePath, AMInterfaceName, QDBusConnection::sessionBus(), this))
{

}
