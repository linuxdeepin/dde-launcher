#ifndef APPSMANAGER_H
#define APPSMANAGER_H

#include "appslistmodel.h"
#include "../dbus/dbuslauncher.h"
#include "../dbus/dbusfileinfo.h"
#include "../dbus/dbustartmanager.h"

#include <QMap>
#include <QSettings>
#include <QPixmap>

class AppsManager : public QObject
{
    Q_OBJECT

public:
    static AppsManager *instance(QObject *parent = nullptr);

public slots:
    void removeRow(const int row);
    const ItemInfoList appsInfoList(const AppsListModel::AppCategory &category) const;

    bool appIsAutoStart(const QString &desktop);
    const QPixmap appIcon(const QString &desktop, const int size);
    ItemInfo getItemInfo(QString appKey);
private:
    explicit AppsManager(QObject *parent = 0);

    void initConnection();

    void refreshCategoryInfoList();
    void refreshAppIconCache();
    void refreshAppAutoStartCache();

private:
    DBusLauncher *m_launcherInter;
    DBusFileInfo *m_fileInfoInter;
    DBusStartManager *m_startManagerInter;

    ItemInfoList m_appInfoList;
    ItemInfoList m_appSearchResultList;
    QMap<AppsListModel::AppCategory, ItemInfoList> m_appInfos;

    static AppsManager *INSTANCE;
    static QSettings APP_ICON_CACHE;
    static QSettings APP_AUTOSTART_CACHE;
//    static QSettings AppInfoCache;
};

#endif // APPSMANAGER_H
