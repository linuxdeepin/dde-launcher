#ifndef APPSMANAGER_H
#define APPSMANAGER_H

#include "appslistmodel.h"
#include "../dbus/dbuslauncher.h"

class AppsManager : public QObject
{
    Q_OBJECT

public:
    explicit AppsManager(QObject *parent = 0);

    void removeRow(const int row);
    inline const ItemInfoList &appsInfoList() const {return m_appInfoList;}

private:
    DBusLauncher *m_launterInter;

    ItemInfoList m_appInfoList;
};

#endif // APPSMANAGER_H
