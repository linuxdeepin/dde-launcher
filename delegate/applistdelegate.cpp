/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "applistdelegate.h"
#include "model/appslistmodel.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>

AppListDelegate::AppListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent),

      m_actived(false),
      m_blueDotPixmap(":/skin/images/new_install_indicator.png"),
      m_autoStartPixmap(":/skin/images/emblem-autostart.png")
{

}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QRect r = option.rect;
    const QPixmap icon = index.data(AppsListModel::AppIconRole).value<QPixmap>();

    painter->setRenderHint(QPainter::Antialiasing);

    if (option.state.testFlag(QStyle::State_Selected))
    {
        if (m_actived)
            painter->setBrush(QColor(0, 0, 0, 255 * .4));
        else
            painter->setBrush(QColor(0, 0, 0, 255 * .2));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 4, 4);
    }

    const int icon_x = r.x() + 10;
    const int icon_y = r.y() + (r.height() - icon.height()) / 2;
    painter->drawPixmap(icon_x, icon_y, icon);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(icon_x, icon_y + m_autoStartPixmap.height(), m_autoStartPixmap);

    // draw blue dot if new installed
    const bool drawBlueDot = index.data(AppsListModel::AppNewInstallRole).toBool();
    if (drawBlueDot)
        painter->drawPixmap(50, r.y() + (r.height() - m_blueDotPixmap.height()) / 2, m_blueDotPixmap);

    painter->setPen(Qt::white);
    if (drawBlueDot)
        painter->drawText(r.marginsRemoved(QMargins(65, 0, 0, 0)), Qt::AlignVCenter | Qt::AlignLeft, index.data(AppsListModel::AppNameRole).toString());
    else
        painter->drawText(r.marginsRemoved(QMargins(50, 0, 0, 0)), Qt::AlignVCenter | Qt::AlignLeft, index.data(AppsListModel::AppNameRole).toString());
}

QSize AppListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(0, 50);
}

void AppListDelegate::setActived(bool active)
{
    m_actived = active;
}
