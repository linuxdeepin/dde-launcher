/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "installedtimeinfo.h"

InstalledTimeInfo::InstalledTimeInfo()
{

}

InstalledTimeInfo::~InstalledTimeInfo()
{

}

void InstalledTimeInfo::registerMetaType()
{
    qRegisterMetaType<InstalledTimeInfo>("InstalledTimeInfo");
    qDBusRegisterMetaType<InstalledTimeInfo>();
    qRegisterMetaType<InstalledTimeInfoList>("InstalledTimeInfoList");
    qDBusRegisterMetaType<InstalledTimeInfoList>();
}

QDebug operator<<(QDebug argument, const InstalledTimeInfo &info)
{
    argument << info.m_key << info.m_installedTime;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const InstalledTimeInfo &info)
{
    argument.beginStructure();
    argument << info.m_key << info.m_installedTime;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const InstalledTimeInfo &info)
{
    argument << info.m_key << info.m_installedTime;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, InstalledTimeInfo &info)
{
    argument.beginStructure();
    argument >> info.m_key >> info.m_installedTime;
    argument.endStructure();

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, InstalledTimeInfo &info)
{
    argument >> info.m_key >> info.m_installedTime;

    return argument;
}
