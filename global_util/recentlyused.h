/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef RECENTLYUSED_H
#define RECENTLYUSED_H

#include <QObject>
#include <QXmlStreamReader>

#include <memory>

class RecentlyUsed : public QObject
{
    Q_OBJECT

public:
    explicit RecentlyUsed(QObject *parent = 0);

    const QList<QString> history() const { return m_history; }

public slots:
    void clear();

private slots:
    void reload();

private:
    std::unique_ptr<QXmlStreamReader> m_xmlRdr;

    QList<QString> m_history;
};

#endif // RECENTLYUSED_H
