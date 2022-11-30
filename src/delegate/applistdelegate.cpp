// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    if (!m_actived) return;

    const qreal ratio = qApp->devicePixelRatio();
    const QRect rect = option.rect;
    const bool isDragItem = option.features & QStyleOptionViewItem::Alternate;
    const bool isCategoryList(static_cast<AppsListModel::AppCategory>(index.data(AppsListModel::AppGroupRole).toInt()) == AppsListModel::Category);

    static QSize size(30, 30);
    QPixmap iconPixmap = index.data(AppsListModel::AppListIconRole).value<QPixmap>();
    if (!iconPixmap.isNull()) {
        size = iconPixmap.size();
    }
    ItemInfo itemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo>();
    if (iconPixmap.isNull() && itemInfo.m_iconKey != "") {
        iconPixmap = QPixmap(itemInfo.m_iconKey).scaled(size);
        iconPixmap.setDevicePixelRatio(ratio);
        qWarning() << " [paint] iconPixmap is null. use other itemInfo : " << itemInfo << iconPixmap << size;
    }

    if (isDragItem) {
        QPixmap dragIndicator = renderSVG(":/widgets/images/drag_indicator.svg",
                                          QSize(20, 20));
        painter->drawPixmap(rect.right() - 30,
                            rect.y() + (rect.height() - dragIndicator.height() / ratio) / 2,
                            dragIndicator);
    }

    painter->setPen(Qt::NoPen);

    if (option.state.testFlag(QStyle::State_Selected)) {
        // hover background color.
        painter->setBrush(m_color);
    } else if (isDragItem) {
        // drag item background color.
        painter->setBrush(QColor(255, 255, 255, 255 * 0.4));
        painter->setPen(QColor(0, 0, 0, 255 * 0.05));
    } else {
        // normal item.
        painter->setBrush(Qt::NoBrush);
    }

    if (index.data(AppsListModel::DrawBackgroundRole).toBool()) {
        // draw the background.
        painter->drawRoundedRect(option.rect, 8, 8);
    }

    const int iconX = rect.x() + 10;
    const int iconY = rect.y() + (rect.height() - iconPixmap.height() / ratio) / 2;

    painter->drawPixmap(iconX, iconY, iconPixmap);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool()) {
        painter->drawPixmap(iconX, iconY + 16, m_autoStartPixmap);
    }

    QRect textRect = rect.marginsRemoved(QMargins(60, 1, 1, 1));
    textRect.setWidth(textRect.width() - m_blueDotPixmap.width() / ratio - 10);

    if (isDragItem) {
        QFont nameFont = painter->font();
        nameFont.setPointSize(nameFont.pointSize() + 2);
        painter->setFont(nameFont);
    }

    // draw app name.
    painter->setPen(QPen(QPalette().brightText(), 1));

    if (isCategoryList) {
        textRect.setX(textRect.x() - 13);
        textRect.setY(textRect.y() - 2);
    }

    // 使用省略号显示超长的应用名称
    const QString appName = painter->fontMetrics().elidedText(index.data(AppsListModel::AppNameRole).toString(), Qt::ElideRight, textRect.width());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, appName);

    // draw category right icon
    if (isCategoryList) {
        const QPixmap &pixmap = index.data(AppsListModel::CategoryEnterIconRole).value<QPixmap>();
        painter->setRenderHints(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(rect.right() - pixmap.width() / ratio,
                            rect.y() + (rect.height() - pixmap.height() / ratio) / 2,
                            pixmap.width() /  ratio,
                            pixmap.height() / ratio,
                            pixmap);
    }

    // draw blue dot if needed
    if (index.data(AppsListModel::AppNewInstallRole).toBool() && !isDragItem) {
        const QPointF blueDotPos(rect.width() - m_blueDotPixmap.width() / ratio - (isCategoryList ? 20 : 6),
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

    // reference here:
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
