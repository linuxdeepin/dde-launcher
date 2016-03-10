#ifndef APPSMANAGER_H
#define APPSMANAGER_H

#include "appslistmodel.h"
#include "dbus/dbuslauncher.h"
#include "dbus/fileInfo_interface.h"

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
    QList<int> getCategoryAppNumsList();
private:
    FileInfoInterface* m_fileInfoInterface;
    DBusLauncher *m_launterInter;

    ItemInfoList m_allAppInfoList;
    //The data of categoryInfo itemInfos: internet, chat, music,
    // and so on...
    ItemInfoList m_internetInfos;
    ItemInfoList m_chatInfos;
    ItemInfoList m_musicInfos;
    ItemInfoList m_videoInfos;
    ItemInfoList m_graphicsInfos;
    ItemInfoList m_gamesInfos;
    ItemInfoList m_officeInfos;
    ItemInfoList m_readingInfos;
    ItemInfoList m_developmentInfos;
    ItemInfoList m_systemInfos;
    ItemInfoList m_otherInfos;

    QList<ItemInfo*> m_catagoryInfos;
    QList<int> m_categoryAppNums;
    CategoryInfoList m_categoryInfoList;
};

#endif // APPSMANAGER_H
