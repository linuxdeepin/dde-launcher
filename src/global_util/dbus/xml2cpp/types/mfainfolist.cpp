/*
 * Copyright (C) 2021 ~ 2026 Deepin Technology Co., Ltd.
 *
 * Author:     Zhang Qipeng <zhangqipeng@uniontech.com>
 *
 * Maintainer: Zhang Qipeng <zhangqipeng@uniontech.com>
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

#include "mfainfolist.h"

bool MFAInfo::operator==(const MFAInfo &info) const
{
    return AuthType == info.AuthType && Priority == info.Priority
           && InputType == info.InputType && Required == info.Required;
}

QDebug operator<<(QDebug argument, const MFAInfo &info)
{
    argument << info.AuthType << "," << info.Priority << "," << info.InputType << "," << info.Required;
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const MFAInfo &info)
{
    argument.beginStructure();
    argument << info.AuthType << info.Priority << info.InputType << info.Required;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, MFAInfo &info)
{
    argument.beginStructure();
    argument >> info.AuthType >> info.Priority >> info.InputType >> info.Required;
    argument.endStructure();
    return argument;
}

void registerMFAInfoMetaType()
{
    qRegisterMetaType<MFAInfo>("MFAInfo");
    qDBusRegisterMetaType<MFAInfo>();
}

void registerMFAInfoListMetaType()
{
    registerMFAInfoMetaType();
    qRegisterMetaType<MFAInfoList>("MFAInfoList");
    qDBusRegisterMetaType<MFAInfoList>();
}
