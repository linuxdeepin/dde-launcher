/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "historymodel.h"

#include <QTimer>
#include <QDebug>
#include <QUrl>

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QTimer::singleShot(1, this, &HistoryModel::updateHistory);
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_data.size();
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    switch (role)
    {
    case ItemNameRole:
        return m_data[index.row()].fileName();
    case ItemFullPathRole:
        return m_data[index.row()].absoluteFilePath();
    default:;
    }

    return QVariant();
}

void HistoryModel::clear()
{
    m_history.clear();

    QTimer::singleShot(1, this, &HistoryModel::updateHistory);
}

void HistoryModel::updateHistory()
{
    const QStringList history = m_history.history();

    for (const auto &h : history)
    {
        const QFileInfo info(QUrl(h).toLocalFile());
        if (!info.exists())
            continue;

        m_data << info;
    }

    emit layoutChanged();
}
