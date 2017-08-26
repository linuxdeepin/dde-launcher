/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DOCKRECT_H
#define DOCKRECT_H

#include <QRect>
#include <QDBusArgument>
#include <QDebug>
#include <QDBusMetaType>

struct DockRect
{
public:
    DockRect();
    operator QRect() const;

    friend QDebug operator<<(QDebug debug, const DockRect &rect);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, DockRect &rect);
    friend QDBusArgument &operator<<(QDBusArgument &arg, const DockRect &rect);

private:
    qint32 x;
    qint32 y;
    quint32 w;
    quint32 h;
};

Q_DECLARE_METATYPE(DockRect)

#endif // DOCKRECT_H
