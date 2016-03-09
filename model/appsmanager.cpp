#include "appsmanager.h"

#include <QDebug>

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launcherInter(new DBusLauncher(this))
{
    m_appInfoList = m_launcherInter->GetAllItemInfos().value();


    qDebug() << m_launcherInter->GetAllCategoryInfos().value();
    qDebug() << m_appInfoList;
}

void AppsManager::removeRow(const int row)
{
    m_appInfoList.removeAt(row);
}

const ItemInfoList AppsManager::appsInfoList(const AppsListModel::AppCategory &category) const
{
    int categoryId = -1;
    switch (category)
    {
    case AppsListModel::Custom:
    case AppsListModel::Search:
    case AppsListModel::All:            return appsInfoList();
    case AppsListModel::Office:         categoryId = 6;     break;
    case AppsListModel::Game:           categoryId = 5;     break;
    case AppsListModel::System:         categoryId = 9;     break;
    case AppsListModel::Chat:           categoryId = 1;     break;
    case AppsListModel::Others:         categoryId = 10;    break;
    case AppsListModel::Internet:       categoryId = 0;     break;
    case AppsListModel::Development:    categoryId = 8;     break;
    case AppsListModel::Reading:        categoryId = 7;     break;
    case AppsListModel::Graphics:       categoryId = 4;     break;
    case AppsListModel::Music:          categoryId = 2;     break;
    case AppsListModel::Video:          categoryId = 3;     break;
    default:
        qWarning() << "AppsManager::appsInfoList" << " wrong switch data";
    }

    ItemInfoList retList;
    for (const ItemInfo &info : m_appInfoList)
        if (info.m_id == categoryId)
            retList.append(info);

    return std::move(retList);
}

//void AppsManager::refreshCategoryInfoList()
//{
//    const CategoryInfoList categoryInfoList = m_launcherInter->GetAllCategoryInfos().value();
//    for (const CategoryInfo &info : categoryInfoList)
//    {
//        if (m_categoryMap.contains(info.m_name))
//            continue;

//        m_categoryMap.insert(info.m_name, info.m_id);
//    }
//}
