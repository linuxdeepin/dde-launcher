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

    void allItemInfo();
    //get the ItemInfoList of categoryInfo("Internet, chat,...")
    void categoryInfo();

    ItemInfoList getCategoryInfo(qlonglong id);
    ItemInfoList &appsInfoList()  { return m_allAppInfoList;}
    CategoryInfoList &categoryInfoList() { return m_categoryInfoList;}
    ItemInfoList getCategoryItemInfo(int category_index);
private:
    DBusLauncher *m_launterInter;

    ItemInfoList m_allAppInfoList;
    QList<ItemInfo*> m_catagoryInfos;
    CategoryInfoList m_categoryInfoList;
};

#endif // APPSMANAGER_H
