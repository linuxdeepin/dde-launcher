#ifndef APPSMANAGER_H
#define APPSMANAGER_H

#include "appslistmodel.h"
#include "../dbus/dbuslauncher.h"

#include <QMap>

class AppsManager : public QObject
{
    Q_OBJECT

public:
    explicit AppsManager(QObject *parent = 0);

    void removeRow(const int row);
    const ItemInfoList appsInfoList(const AppsListModel::AppCategory &category) const;

private:
    void refreshCategoryInfoList();
    inline const ItemInfoList appsInfoList() const {return m_appInfoList;}

private:
    DBusLauncher *m_launcherInter;

//    QMap<QString, int> m_categoryMap;

    ItemInfoList m_appInfoList;
};

#endif // APPSMANAGER_H
