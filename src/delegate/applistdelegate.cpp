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
#include "src/model/appslistmodel.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QApplication>

#include <DSvgRenderer>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

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
    if (index.data(AppsListModel::AppItemIsDraggingRole).value<bool>() &&
        !(option.features & QStyleOptionViewItem::HasDisplay)) {
        return;
    }

    const qreal ratio = qApp->devicePixelRatio();
    const QRect rect = option.rect;
    const bool isDrawTips = index.data(AppsListModel::AppNewInstallRole).toBool();
    const bool isDragItem = option.features & QStyleOptionViewItem::Alternate;

    QSize iconSize = index.data(AppsListModel::AppIconSizeRole).value<QSize>();

    QPixmap iconPixmap = index.data(AppsListModel::AppIconRole).value<QPixmap>();
    iconPixmap = iconPixmap.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconPixmap.setDevicePixelRatio(ratio);

    if (isDragItem) {
        iconSize = iconSize * 1.1;

        QPixmap dragIndicator = DSvgRenderer::render(":/widgets/images/drag_indicator.svg",
                                                     QSize(20, 20) * ratio);
        dragIndicator.setDevicePixelRatio(ratio);
        painter->drawPixmap(rect.right() - 30,
                            rect.y() + (rect.height() - dragIndicator.height() / ratio) / 2,
                            dragIndicator);
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    if (option.state.testFlag(QStyle::State_Selected)) {
        // hover background color.
        painter->setBrush(QColor(0, 0, 0, 255 * .2));
    } else if (isDragItem) {
        // drag item background color.
        painter->setBrush(QColor(255, 255, 255, 255 * 0.4));
        painter->setPen(QColor(0, 0, 0, 255 * 0.05));
    } else {
        // normal item.
        painter->setBrush(Qt::NoBrush);
    }

    // draw the background.
    painter->drawRoundedRect(rect, 4, 4);

    const int iconX = rect.x() + 10;
    const int iconY = rect.y() + (rect.height() - iconPixmap.height() / ratio) / 2;
    painter->drawPixmap(iconX, iconY, iconPixmap);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool()) {
        painter->drawPixmap(iconX, iconY + 16, m_autoStartPixmap);
    }

    QRect textRect = rect.marginsRemoved(QMargins(48, 1, 1, 1));
    QString appName = index.data(AppsListModel::AppNameRole).toString();
    const QFontMetrics fm = painter->fontMetrics();

    if (isDrawTips) {
        textRect.setWidth(textRect.width() - 90);
    }

    if (isDragItem) {
        QFont nameFont = painter->font();
        nameFont.setPointSize(nameFont.pointSize() + 2);
        painter->setFont(nameFont);
    }

    // draw app name.
    painter->setPen(Qt::white);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(appName, Qt::ElideRight, textRect.width()));

    // drag item is not drawing tips.
    if (isDrawTips && !isDragItem) {
        QRect tipsRect = rect.marginsRemoved(QMargins(1, 1, 1, 1));
        tipsRect.setWidth(50);
        tipsRect.setHeight(20);
        tipsRect.moveTopLeft(QPoint(rect.right() - (tipsRect.width() + 5),
                                    rect.y() + (rect.height() - tipsRect.height()) / 2));

        // draw tips background.
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 255 * .5));
        painter->drawRoundedRect(tipsRect, 10, 10);

        // set the font size of tips text.
        QFont font = painter->font();
        const int fontSize = font.pointSize();
        font.setPointSize(8);

        const QRect tipsTextRect = tipsRect;
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

    return QSize(0, 36);
}

void AppListDelegate::setActived(bool active)
{
    m_actived = active;
}

QPixmap AppListDelegate::dropShadow(QPixmap pixmap, int radius, const QColor &color, const QPoint &offset)
{
    if (pixmap.isNull()) {
        return QPixmap();
    }

    pixmap.setDevicePixelRatio(1);

    // refrence here:
    // https://forum.qt.io/topic/77576/painting-shadow-around-a-parentless-qwidget

    QImage temp(pixmap.size() + QSize(radius * 2, radius * 2),
               QImage::Format_ARGB32_Premultiplied);
    temp.fill(0);

    QPainter tempPainter(&temp);
    tempPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tempPainter.drawPixmap(QPoint(radius, radius), pixmap);
    tempPainter.end();

    QImage blurred(temp.size(),
                   QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);

    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, temp, radius, false, true);
    blurPainter.end();

    temp = blurred;

    tempPainter.begin(&temp);
    tempPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tempPainter.fillRect(temp.rect(), color);
    tempPainter.end();

    QPainter pa(&temp);
    pa.setCompositionMode(QPainter::CompositionMode_SourceOver);
    pa.drawPixmap(radius - offset.x(), radius - offset.y(), pixmap);
    pa.end();

    return QPixmap::fromImage(temp);
}
