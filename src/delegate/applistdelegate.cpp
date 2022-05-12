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
#include "appslistmodel.h"
#include "calculate_util.h"
#include "util.h"
#include "iteminfo.h"

#include <DGuiApplicationHelper>

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QApplication>

DGUI_USE_NAMESPACE

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

AppListDelegate::AppListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent),
      m_actived(false),
      m_calcUtil(CalculateUtil::instance())
{
    m_blueDotPixmap = renderSVG(":/skin/images/new_install_indicator.svg", QSize(10, 10));
    m_autoStartPixmap = renderSVG(":/skin/images/emblem-autostart.svg", QSize(16, 16));

    // 修改背景颜色
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        m_color.setRgb(255, 255, 255, 25); // 深色背景为:#FFFFFF 透明度: 25
    } else {
        m_color.setRgb(0, 0, 0, 25); // 深色背景为:#000000 透明度: 25
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ](DGuiApplicationHelper::ColorType themeType) {
        // 修改背景颜色
        if (DGuiApplicationHelper::DarkType == themeType) {
            m_color.setRgb(255, 255, 255, 25); // 深色背景为:#FFFFFF 透明度: 25
        } else {
            m_color.setRgb(0, 0, 0, 25); // 深色背景为:#000000 透明度: 25
        }
    });
}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(AppsListModel::AppItemIsDraggingRole).value<bool>() &&
            !(option.features & QStyleOptionViewItem::HasDisplay)) {
        return;
    }

    if (!m_actived)
        return;

    const qreal ratio = qApp->devicePixelRatio();
    const QRect rect = option.rect;
    const bool isDragItem = option.features & QStyleOptionViewItem::Alternate;

    QPixmap iconPixmap = index.data(AppsListModel::AppListIconRole).value<QPixmap>();
    ItemInfo itemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo>();
    bool isTitle = index.data(AppsListModel::AppItemTitleRole).toBool();

    painter->setPen(Qt::NoPen);

    if (option.state.testFlag(QStyle::State_Selected) && !isTitle) {
        painter->setBrush(m_color);
    } else if (isDragItem && !isTitle) {
        painter->setBrush(QColor(255, 255, 255, 255 * 0.4));
        painter->setPen(QColor(0, 0, 0, 255 * 0.05));
    } else {
        painter->setBrush(Qt::NoBrush);
    }

    if (index.data(AppsListModel::DrawBackgroundRole).toBool())
        painter->drawRoundedRect(option.rect.marginsRemoved(QMargins(15, 0, 0, 0)), 8, 8);

    const int iconX = rect.x() + 20;
    const int iconY = qRound(rect.y() + (rect.height() - iconPixmap.height() / ratio) / 2);

    if (!isTitle)
        painter->drawPixmap(iconX, iconY, iconPixmap);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool()) {
        painter->drawPixmap(iconX, iconY + 16, m_autoStartPixmap);
    }

    QRect textRect = rect.marginsRemoved(QMargins(20, 1, 1, 1));
    if (!isTitle)
        textRect = rect.marginsRemoved(QMargins(60, 1, 1, 1)); // 图标 + 间隔,再显示文字

    textRect.setWidth(qRound(textRect.width() - m_blueDotPixmap.width() / ratio - 60));

    if (isDragItem) {
        QFont nameFont = painter->font();
        nameFont.setPointSize(nameFont.pointSize() + 2);
        painter->setFont(nameFont);
    }

    // 使用省略号显示超长的应用名称
    const QString appName = painter->fontMetrics().elidedText(index.data(AppsListModel::AppNameRole).toString(), Qt::ElideRight, textRect.width());

    static QPen pen = painter->pen();
    static QFont font = painter->font();
    painter->setPen(Qt::black);
    if (isTitle) {
        painter->setFont(font);
    } else {
        painter->setFont(QFont(font.family(), 8));
    }

    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, appName);

    // draw blue dot if needed
    if (index.data(AppsListModel::AppNewInstallRole).toBool() && !isDragItem && !isTitle) {
        const QPointF blueDotPos(rect.width() - m_blueDotPixmap.width() / ratio - 6,
                                 rect.y() + (+ rect.height() - m_blueDotPixmap.height() / ratio) / 2);

        painter->drawPixmap(blueDotPos, m_blueDotPixmap);
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
