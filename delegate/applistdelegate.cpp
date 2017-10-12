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

#include "applistdelegate.h"
#include "model/appslistmodel.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QApplication>

#include <DSvgRenderer>

DWIDGET_USE_NAMESPACE

AppListDelegate::AppListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent),

      m_actived(false),
      m_blueDotPixmap(":/skin/images/new_install_indicator.png")
{
    const auto ratio = qApp->devicePixelRatio();
    m_autoStartPixmap = DSvgRenderer::render(":/skin/images/emblem-autostart.svg", QSize(16, 16) * ratio);
    m_autoStartPixmap.setDevicePixelRatio(ratio);
}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto ratio = qApp->devicePixelRatio();
    const QRect r = option.rect;
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).value<QSize>();
    QPixmap icon = index.data(AppsListModel::AppIconRole).value<QPixmap>();
    icon = icon.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (ratio > 1.0)
        icon.setDevicePixelRatio(ratio);

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
    const int icon_y = r.y() + (r.height() - icon.height() / ratio) / 2;
    painter->drawPixmap(icon_x, icon_y, icon);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(icon_x, icon_y + 16, m_autoStartPixmap);

    // draw blue dot if new installed
    const bool drawBlueDot = index.data(AppsListModel::AppNewInstallRole).toBool();
    if (drawBlueDot)
        painter->drawPixmap(70, r.y() + (r.height() - m_blueDotPixmap.height()) / 2, m_blueDotPixmap);

    painter->setPen(Qt::white);
    if (drawBlueDot)
        painter->drawText(r.marginsRemoved(QMargins(85, 0, 0, 0)), Qt::AlignVCenter | Qt::AlignLeft, index.data(AppsListModel::AppNameRole).toString());
    else
        painter->drawText(r.marginsRemoved(QMargins(70, 0, 0, 0)), Qt::AlignVCenter | Qt::AlignLeft, index.data(AppsListModel::AppNameRole).toString());
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
