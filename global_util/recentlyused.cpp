/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "recentlyused.h"

#include <QTimer>
#include <QFile>
#include <QDebug>

RecentlyUsed::RecentlyUsed(QObject *parent)
    : QObject(parent),

      m_xmlRdr(std::unique_ptr<QXmlStreamReader>(new QXmlStreamReader))
{
    QTimer::singleShot(1, this, &RecentlyUsed::reload);
}

void RecentlyUsed::clear()
{
    QFile f("/home/.local/share/recently-used.xbel");
    f.remove();

    QTimer::singleShot(1, this, &RecentlyUsed::reload);
}

void RecentlyUsed::reload()
{
    m_history.clear();

    QFile f("/home/.local/share/recently-used.xbel");
    if (!f.open(QIODevice::ReadOnly))
        return;

    m_xmlRdr->setDevice(&f);
    while (!m_xmlRdr->atEnd())
    {
        if (!m_xmlRdr->readNextStartElement())
            continue;
        if (m_xmlRdr->name() != "bookmark")
            continue;

        const auto &location = m_xmlRdr->attributes().value("href");
        if (!location.isEmpty())
            m_history.append(location.toString());
    }

    f.close();
}
