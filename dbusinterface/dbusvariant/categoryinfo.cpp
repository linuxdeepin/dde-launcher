/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "categoryinfo.h"

CategoryInfo::CategoryInfo()
{

}

CategoryInfo::~CategoryInfo()
{

}

void CategoryInfo::registerMetaType()
{
    qRegisterMetaType<CategoryInfo>("CategoryInfo");
    qDBusRegisterMetaType<CategoryInfo>();
    qRegisterMetaType<CategoryInfoList>("CategoryInfoList");
    qDBusRegisterMetaType<CategoryInfoList>();
}

QDebug operator<<(QDebug argument, const CategoryInfo &info)
{
    argument << info.m_id << info.m_name;
    argument << info.m_items;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CategoryInfo &info)
{
    argument.beginStructure();
    argument << info.m_name << info.m_id << info.m_items;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const CategoryInfo &info)
{
    argument << info.m_name << info.m_id << info.m_items;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CategoryInfo &info)
{
    argument.beginStructure();
    argument >> info.m_name >> info.m_id >> info.m_items;
    argument.endStructure();

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, CategoryInfo &info)
{
    argument >> info.m_name >> info.m_id >> info.m_items;

    return argument;
}
