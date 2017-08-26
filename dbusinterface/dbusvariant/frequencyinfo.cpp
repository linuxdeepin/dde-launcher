/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "frequencyinfo.h"

FrequencyInfo::FrequencyInfo()
{

}

FrequencyInfo::~FrequencyInfo()
{

}

void FrequencyInfo::registerMetaType()
{
    qRegisterMetaType<FrequencyInfo>("FrequencyInfo");
    qDBusRegisterMetaType<FrequencyInfo>();
    qRegisterMetaType<FrequencyInfoList>("FrequencyInfoList");
    qDBusRegisterMetaType<FrequencyInfoList>();
}

QDebug operator<<(QDebug argument, const FrequencyInfo &info)
{
    argument << info.m_key << info.m_count;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const FrequencyInfo &info)
{
    argument.beginStructure();
    argument << info.m_key << info.m_count;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const FrequencyInfo &info)
{
    argument << info.m_key << info.m_count;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FrequencyInfo &info)
{
    argument.beginStructure();
    argument >> info.m_key >> info.m_count;
    argument.endStructure();

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, FrequencyInfo &info)
{
    argument >> info.m_key >> info.m_count;

    return argument;
}
