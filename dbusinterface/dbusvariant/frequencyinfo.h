/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef FREQUENCYINFO_H
#define FREQUENCYINFO_H

#include <QtDBus>
#include <QDebug>

class FrequencyInfo
{
public:
    FrequencyInfo();
    ~FrequencyInfo();

    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const FrequencyInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const FrequencyInfo &info);
    friend QDataStream &operator<<(QDataStream &argument, const FrequencyInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, FrequencyInfo &info);
    friend const QDataStream &operator>>(QDataStream &argument, FrequencyInfo &info);

public:
    QString m_key;
    qulonglong m_count;
};

typedef QList<FrequencyInfo> FrequencyInfoList;

Q_DECLARE_METATYPE(FrequencyInfo)
Q_DECLARE_METATYPE(FrequencyInfoList)

#endif // FREQUENCYINFO_H
