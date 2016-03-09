#include "appsmanager.h"
#include "global_util/themeappicon.h"
#include <QDebug>

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
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

    qDebug() << "iconUrl------------:" << ThemeAppIcon::getThemeIconPath("deepin-screenshot");
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
    ItemInfoList tmpCategoryItemInfoList;
    for (int i(0); i < m_categoryInfoList.length(); i++) {
        if (category_index == i) {
            for (int j = 0; j < m_categoryInfoList.at(i).m_items.length(); j++) {
                QString tmpItem = m_categoryInfoList.at(i).m_items.at(j);
                QDBusPendingReply<ItemInfo> itemReply =  m_launterInter->GetItemInfo(tmpItem);
                itemReply.waitForFinished();
                if (itemReply.isValid() && !itemReply.isError()) {
                    tmpCategoryItemInfoList.append(itemReply.value());
                }
            }
            return tmpCategoryItemInfoList;
        } else {
            continue;
        }
    }
    return ItemInfoList();
}

void AppsManager::removeRow(const int row)
{
    m_allAppInfoList.removeAt(row);
}
