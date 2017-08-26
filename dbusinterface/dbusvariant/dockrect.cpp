/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dockrect.h"

DockRect::DockRect()
    : x(0),
      y(0),
      w(0),
      h(0)
{
}

QDebug operator<<(QDebug debug, const DockRect &rect)
{
    debug << QString("ScreenRect(%1, %2, %3, %4)").arg(rect.x)
                                                    .arg(rect.y)
                                                    .arg(rect.w)
                                                    .arg(rect.h);

    return debug;
}

DockRect::operator QRect() const
{
    return QRect(x, y, w, h);
}

QDBusArgument &operator<<(QDBusArgument &arg, const DockRect &rect)
{
    arg.beginStructure();
    arg << rect.x << rect.y << rect.w << rect.h;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, DockRect &rect)
{
    arg.beginStructure();
    arg >> rect.x >> rect.y >> rect.w >> rect.h;
    arg.endStructure();

    return arg;
}
