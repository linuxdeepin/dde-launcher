/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

      m_actived(false)
{
    const auto ratio = qApp->devicePixelRatio();
    m_blueDotPixmap = DSvgRenderer::render(":/skin/images/new_install_indicator.svg", QSize(10, 10) * ratio);
    m_blueDotPixmap.setDevicePixelRatio(ratio);
    m_autoStartPixmap = DSvgRenderer::render(":/skin/images/emblem-autostart.svg", QSize(16, 16) * ratio);
    m_autoStartPixmap.setDevicePixelRatio(ratio);
}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const qreal ratio = qApp->devicePixelRatio();
    const QRect rect = option.rect;
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).value<QSize>();
    const bool drawTipsDot = index.data(AppsListModel::AppNewInstallRole).toBool();
    QPixmap iconPixmap = index.data(AppsListModel::AppIconRole).value<QPixmap>();
    iconPixmap = iconPixmap.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconPixmap.setDevicePixelRatio(ratio);

    painter->setRenderHint(QPainter::Antialiasing);

    if (option.state.testFlag(QStyle::State_Selected)) {
        if (m_actived)
            painter->setBrush(QColor(0, 0, 0, 255 * .4));
        else
            painter->setBrush(QColor(0, 0, 0, 255 * .2));

        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rect.marginsRemoved(QMargins(1, 1, 1, 1)), 4, 4);
    }

    const int iconX = rect.x() + 10;
    const int iconY = rect.y() + (rect.height() - iconPixmap.height() / ratio) / 2;
    painter->drawPixmap(iconX, iconY, iconPixmap);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool()) {
        painter->drawPixmap(iconX, iconY + 16, m_autoStartPixmap);
    }

    QRect textRect = rect.marginsRemoved(QMargins(60, 1, 1, 1));
    QString appName = index.data(AppsListModel::AppNameRole).toString();
    const QFontMetrics fm = painter->fontMetrics();

    if (drawTipsDot) {
        textRect.setWidth(textRect.width() - 90);
    }

    // draw app name.
    painter->setPen(Qt::white);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(appName, Qt::ElideRight, textRect.width()));

    if (drawTipsDot) {
        QRect tipsRect = rect;
        const int rightPadding = 5;
        tipsRect.setLeft(rect.right() - 90 - rightPadding);
        tipsRect.setWidth(90);
        tipsRect.setHeight(30);
        tipsRect.moveTop(rect.y() + (rect.height() - tipsRect.height() / ratio) / 2);

        // draw tips background.
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 255 * .5));
        painter->drawRoundedRect(tipsRect, 15, 15);

        // set the font size of tips text.
        QFont font = painter->font();
        const int fontSize = font.pointSize();
        font.setPointSize(10);

        const QRect tipsTextRect = tipsRect.marginsRemoved(QMargins(5, 0, 2, 0));
        const QString tipsText = painter->fontMetrics().elidedText(tr("New"), Qt::ElideRight, tipsTextRect.width());

        // draw tips text.
        painter->setFont(font);
        painter->setPen(Qt::white);
        painter->drawText(tipsTextRect, Qt::AlignCenter, tipsText);

        // restore original font size.
        font.setPointSize(fontSize);
        painter->setFont(font);
    }
}

QSize AppListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(0, 51);
}

void AppListDelegate::setActived(bool active)
{
    m_actived = active;
}
