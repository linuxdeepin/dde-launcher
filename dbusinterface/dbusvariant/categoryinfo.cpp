/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
