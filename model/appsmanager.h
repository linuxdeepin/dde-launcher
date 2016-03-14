#ifndef APPSMANAGER_H
#define APPSMANAGER_H

#include "appslistmodel.h"
#include "../dbus/dbuslauncher.h"
#include "../dbus/dbusfileinfo.h"

#include <QMap>
#include <QSettings>

class AppsManager : public QObject
{
    Q_OBJECT

public:
    explicit AppsManager(QObject *parent = 0);

    void removeRow(const int row);
    const ItemInfoList appsInfoList(const AppsListModel::AppCategory &category) const;

    const QPixmap appIcon(const QString &desktop, const int size);

private:
    void refreshCategoryInfoList();

private:
    DBusLauncher *m_launcherInter;
    DBusFileInfo *m_fileInfoInter;

    ItemInfoList m_appInfoList;
    ItemInfoList m_appSearchResultList;
    QMap<AppsListModel::AppCategory, ItemInfoList> m_appInfos;

    // cache
    static QSettings m_appIconCache;
};

#endif // APPSMANAGER_H
