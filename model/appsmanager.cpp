#include "appsmanager.h"
#include "global_util/themeappicon.h"
#include <QDebug>

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_fileInfoInterface(new FileInfoInterface(this)),
    m_launterInter(new DBusLauncher(this))
{
    allItemInfo();
    categoryInfo();
}

void AppsManager::allItemInfo() {
    m_allAppInfoList = m_launterInter->GetAllItemInfos().value();

    qDebug() << "m_appInfoList*********:" << m_allAppInfoList.at(0).m_iconKey
             << m_allAppInfoList.at(0).m_id
             << m_allAppInfoList.at(0).m_key
             << m_allAppInfoList.at(0).m_name
             << m_allAppInfoList.at(0).m_url;

        for(int i = 0; i < m_allAppInfoList.length(); i++) {

             m_allAppInfoList[i].iconUrl =  m_fileInfoInterface->GetThemeIcon( m_allAppInfoList[i].m_url, 64);
            qDebug() << "itemInfo.iconUrl:" << m_allAppInfoList[i].iconUrl << m_allAppInfoList[i].m_url;
            qDebug() << "foreach itemInfo" << m_allAppInfoList[i].m_id;
            int itemId = int(m_allAppInfoList[i].m_id);
            switch(itemId) {
            case 0: {m_internetInfos.append(m_allAppInfoList[i]);break;}
            case 1: {m_chatInfos.append(m_allAppInfoList[i]);break;}
            case 2: {m_musicInfos.append(m_allAppInfoList[i]);break;}
            case 3: {m_videoInfos.append(m_allAppInfoList[i]);break;}
            case 4: {m_graphicsInfos.append(m_allAppInfoList[i]);break;}
            case 5: {m_gamesInfos.append(m_allAppInfoList[i]);break;}
            case 6: {m_officeInfos.append(m_allAppInfoList[i]);break;}
            case 7: {m_readingInfos.append(m_allAppInfoList[i]);break;}
            case 8: {m_developmentInfos.append(m_allAppInfoList[i]);break;}
            case 9: {m_systemInfos.append(m_allAppInfoList[i]);break;}
            default: {m_otherInfos.append(m_allAppInfoList[i]);break;}
            }
        }


    qDebug() << "m_interInfos ..., numbers:" << m_internetInfos.length() << m_chatInfos.length()
             << m_musicInfos.length() << m_videoInfos.length() << m_graphicsInfos.length()
             << m_gamesInfos.length() << m_officeInfos.length() << m_readingInfos.length()
             << m_developmentInfos.length() << m_systemInfos.length() << m_otherInfos.length();
}

void AppsManager::categoryInfo() {

    for (int i = 0; i < 11; i++) {
        QDBusPendingReply<CategoryInfo> tmpCategoryInfo = m_launterInter->GetCategoryInfo(qint64(i));
        tmpCategoryInfo.waitForFinished();
        if (tmpCategoryInfo.isValid() && !tmpCategoryInfo.isError()) {
            qDebug() << "get categoryInfo:" << tmpCategoryInfo.value();
            m_categoryInfoList.append(tmpCategoryInfo);
        } else {
            qDebug() << tmpCategoryInfo.reply().errorMessage();
        }
    }

}

ItemInfoList AppsManager::getCategoryItemInfo(int category_index) {
    switch (category_index) {
    case -1: return m_allAppInfoList;
    case 0: return m_internetInfos;
    case 1: return m_chatInfos;
    case 2: return m_musicInfos;
    case 3: return m_videoInfos;
    case 4: return m_graphicsInfos;
    case 5: return m_gamesInfos;
    case 6: return m_officeInfos;
    case 7: return m_readingInfos;
    case 8: return m_developmentInfos;
    case 9: return m_systemInfos;
    default: return m_otherInfos;
    }
    return ItemInfoList();
}

QList<int> AppsManager::getCategoryAppNumsList() {
    m_categoryAppNums.clear();
    m_categoryAppNums.append(m_internetInfos.size());
    m_categoryAppNums.append(m_chatInfos.size());
    m_categoryAppNums.append(m_musicInfos.size());
    m_categoryAppNums.append(m_videoInfos.size());
    m_categoryAppNums.append(m_graphicsInfos.size());
    m_categoryAppNums.append(m_gamesInfos.size());
    m_categoryAppNums.append(m_officeInfos.size());
    m_categoryAppNums.append(m_readingInfos.size());
    m_categoryAppNums.append(m_developmentInfos.size());
    m_categoryAppNums.append(m_systemInfos.size());
    m_categoryAppNums.append(m_otherInfos.size());

    return m_categoryAppNums;
}

void AppsManager::removeRow(const int row)
{
    m_allAppInfoList.removeAt(row);
}
