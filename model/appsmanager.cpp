#include "appsmanager.h"

#include <QDebug>

AppsManager::AppsManager(QObject *parent) :
    QObject(parent),
    m_launterInter(new DBusLauncher(this))
{
    m_appInfoList = m_launterInter->GetAllItemInfos().value();
}

void AppsManager::removeRow(const int row)
{
    m_appInfoList.removeAt(row);
}
