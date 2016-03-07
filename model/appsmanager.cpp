#include "appsmanager.h"

#include <QDebug>

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launterInter(new DBusLauncher(this))
{
    m_appInfoList = m_launterInter->GetAllItemInfos().value();

    qDebug() << "m_appInfoList*********:" << m_appInfoList.at(0).m_iconKey
             << m_appInfoList.at(0).m_id
             << m_appInfoList.at(0).m_key
             << m_appInfoList.at(0).m_name
             << m_appInfoList.at(0).m_url;
}

void AppsManager::removeRow(const int row)
{
    m_appInfoList.removeAt(row);
}
