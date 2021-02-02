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

#include "appitemdelegate.h"
#include "src/global_util/constants.h"
#include "src/global_util/calculate_util.h"
#include "src/global_util/util.h"
#include "src/model/appslistmodel.h"
#include "src/dbusinterface/dbusvariant/iteminfo.h"

#include <QDebug>
#include <QPixmap>
#include <QVariant>
#include <QApplication>


#define  ICONTOLETF  12
#define  ICONTOTOP  6
#define TEXTTOICON  2
#define TEXTTOLEFT  10

QModelIndex AppItemDelegate::CurrentIndex = QModelIndex();

AppItemDelegate::AppItemDelegate(QObject *parent) :
    QAbstractItemDelegate(parent),
    m_calcUtil(CalculateUtil::instance())
{
    m_autoStartPixmap = QIcon(":/skin/images/emblem-autostart.svg").pixmap(QSize(24, 24));
    m_blueDotPixmap = QIcon(":/skin/images/new_install_indicator.svg").pixmap(QSize(10, 10));
}

void AppItemDelegate::setCurrentIndex(const QModelIndex &index)
{
    if (CurrentIndex == index)
        return;

    const QModelIndex previousIndex = CurrentIndex;
    CurrentIndex = index;

    emit requestUpdate(previousIndex);
    emit requestUpdate(CurrentIndex);
}

void AppItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(AppsListModel::AppItemIsDraggingRole).value<bool>() && !(option.features & QStyleOptionViewItem::HasDisplay))
        return;

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::white);
    painter->setBrush(QBrush(Qt::transparent));

#ifdef QT_DEBUG
    ItemInfo itemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo>();
#else
    const ItemInfo itemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo>();
#endif
    const int fontPixelSize = index.data(AppsListModel::AppFontSizeRole).value<int>();
    const bool drawBlueDot = index.data(AppsListModel::AppNewInstallRole).toBool();
    const bool is_current = CurrentIndex == index;
    const QRect ibr = itemBoundingRect(option.rect);
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).toSize();

    // process font
    QFont appNamefont(painter->font());
    appNamefont.setPixelSize(fontPixelSize);
    const QFontMetrics fm(appNamefont);
    //分类模式且不是当前的分类就设置透明度
    if( index.data(AppsListModel::AppGroupRole).toInt() >= 4 && index.data(AppsListModel::AppCategoryRole).toInt() != m_calcUtil->currentCategory()) {
        painter->setOpacity(0.3);
    } else {
        painter->setOpacity(1);
    }

    // Curve Fitting Result from MATLAB
//    const int x = iconSize.width();
//    const int margin = -0.000004236988913209739*x*x*x*x+0.0016406743692943455*x*x*x-0.22885856605074573*x*x+13.187308932617098*x-243.2646393941108;
    const static double x1 = 0.26418192;
    const static double x2 = -0.38890932;
    const double result = x1 * ibr.width() + x2 * iconSize.width();
    int margin = result > 0 ? result * 0.71 : 1;

    // adjust
    QRect br;
    QRect iconRect;
    QRectF appNameRect;
    QString appNameResolved;
    bool adjust = false;
    bool  TextSecond = 0;
    do {
        // adjust
        if (adjust)
            --margin;
        br = ibr.marginsRemoved(QMargins(margin, 1, margin, margin * 2));

        // calc icon rect
        const int iconLeftMargins = (br.width() - iconSize.width()) / 2;
        int  iconTopMargin = ICONTOTOP;// ibr.height() * .2 - iconSize.height() * .3;
        //iconTopMargin = 6; //std::max(iconTopMargin, 1.);
        iconRect = QRect(br.topLeft() + QPoint(iconLeftMargins, iconTopMargin - 2), iconSize);

        //31是字体设置20的时候的高度
        br.setHeight(ICONTOTOP + iconRect.height() + TEXTTOICON + 31 + fontPixelSize * TextSecond + TEXTTOLEFT);
        if (br.height() > ibr.height()) {
            br.setHeight(ibr.height() - 1);
        }

        // calc text
        appNameRect = itemTextRect(br, iconRect, drawBlueDot);
        const QPair<QString, bool> appTextResolvedInfo = holdTextInRect(fm, itemInfo.m_name, appNameRect.toRect());
        appNameResolved = appTextResolvedInfo.first;

        if((fm.width(appNameResolved) + m_blueDotPixmap.width() + 10) > appNameRect.width()){
           TextSecond = 1;
        }

        if (margin == 1 || appTextResolvedInfo.second)
            break;

        // we need adjust again!
        adjust = true;
    } while (true);
//    painter->fillRect(option.rect, Qt::gray);
//    painter->fillRect(ibr, Qt::cyan);
//    painter->fillRect(br, Qt::green);
//    painter->fillRect(appNameRect, Qt::blue);
//    painter->fillRect(iconRect, Qt::magenta);


    // draw focus background
   if (is_current && !(option.features & QStyleOptionViewItem::HasDisplay))
    {
        const int radius = 18;
        const QColor brushColor(255, 255, 255, 51);

        painter->setPen(Qt::transparent);
        painter->setBrush(brushColor);
        int drawBlueDotWidth = 0;
        if(drawBlueDot) {
            drawBlueDotWidth = m_blueDotPixmap.width();
        }

        if(iconSize.width() > (fm.width(appNameResolved) + drawBlueDotWidth)) {
            br.setX(iconRect.x()-ICONTOLETF);
            br.setWidth(iconSize.width()+ICONTOLETF*2);
        }

        painter->drawRoundedRect(br, radius, radius);
    }

    // draw app name
    QTextOption appNameOption;
    appNameOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    appNameOption.setWrapMode(QTextOption::WordWrap);

    appNameRect.setY(br.y() + br.height()  - TEXTTOLEFT + (fm.height() >= 28 ? 2 : 0)- fm.height() - fontPixelSize*TextSecond);

    if(drawBlueDot){
        appNameRect.setX(appNameRect.x() + m_blueDotPixmap.width()/2 + 5);
        appNameRect.setWidth(appNameRect.width() - m_blueDotPixmap.width());
    }

    painter->setFont(appNamefont);
    painter->setBrush(QBrush(Qt::transparent));
    painter->setPen(QColor(0, 0, 0, 80));
    painter->drawText(appNameRect.adjusted(0.8, 1, 0.8, 1), appNameResolved, appNameOption);
    painter->drawText(appNameRect.adjusted(-0.8, 1, -0.8, 1), appNameResolved, appNameOption);
    painter->setPen(Qt::white);
    painter->drawText(appNameRect, appNameResolved, appNameOption);

    // draw app icon
    if("dde-calendar"==itemInfo.m_key)
    {
        //根据不同日期显示不同日历图表
        int tw = iconRect.width();
        int th = iconRect.height();
        int tx = iconRect.x();
        int ty = iconRect.y();
        const double  iconZoom =  iconRect.width() /256.0;

        QStringList calIconList = m_calcUtil->calendarSelectIcon();
        //绘制背景
        painter->drawPixmap(iconRect, renderSVG(calIconList.at(0), iconRect.size()));
        //绘制月份
        QRectF rcMonth(tx + (tw / 3.4), ty + (th / 5.4), 80 * iconZoom, 40 * iconZoom);
        painter->drawPixmap(rcMonth.topLeft(), renderSVG(calIconList.at(1), rcMonth.size().toSize()));
        //绘制日
        QRectF rcDay(tx + (tw / 3.5), ty + th / 3.1, 112 * iconZoom, 104 * iconZoom);
        painter->drawPixmap(rcDay.topLeft(), renderSVG(calIconList.at(2), rcDay.size().toSize()));
        //绘制周
        QRectF rcWeek(tx + (tw / 2.3), ty + ((th / 3.9) * 2.8), 56 * iconZoom, 24 * iconZoom);
        painter->drawPixmap(rcWeek.topLeft(), renderSVG(calIconList.at(3), rcWeek.size().toSize()));
    }else {
        const QPixmap iconPix = index.data(AppsListModel::AppIconRole).value<QPixmap>();
        painter->drawPixmap(iconRect, iconPix, iconPix.rect());
    }

    // draw icon if app is auto startup
    const QPoint autoStartIconPos = iconRect.bottomLeft()
            // offset for auto-start mark itself
            - QPoint(m_autoStartPixmap.height(), m_autoStartPixmap.width()) / m_autoStartPixmap.devicePixelRatioF() / 2
            // extra offset
            + QPoint(iconRect.width() / 10, -iconRect.height() / 10);

    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(autoStartIconPos, m_autoStartPixmap);

    // draw blue dot if needed
    if (drawBlueDot)
    {
        const int marginRight = 2;
        const QRectF textRect = fm.boundingRect(appNameRect.toRect(), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, appNameResolved);
        const auto ratio = m_blueDotPixmap.devicePixelRatioF();
        const QPointF blueDotPos = textRect.topLeft() + QPoint(-m_blueDotPixmap.width() / ratio - marginRight,
                                                               (fm.height() - m_blueDotPixmap.height() / ratio) / 2);
        painter->drawPixmap(blueDotPos, m_blueDotPixmap);
    }
}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}

///
/// \brief calculate item background bounding rect, this rect contains all item like icon, text, blue dot, ...
/// \param itemRect the real item rect
/// \return item bounding rect
///
const QRect AppItemDelegate::itemBoundingRect(const QRect &itemRect) const
{
    const int w = itemRect.width();
    const int h = itemRect.height();
    const int sub = qAbs((w - h) / 2);

    if (w == h)
        return itemRect;
    else if (w > h)
        return itemRect - QMargins(sub, 0, sub, 0);
    else
        return itemRect - QMargins(0, 0, 0, sub * 2);
}

///
/// \brief calculate app item text area rect
/// \param boundingRect item bounding rect
/// \param iconRect item icon rect
/// \param extraWidthMargin remove extra margin if need draw blue dot
/// \return app name text bounding rect
///
const QRect AppItemDelegate::itemTextRect(const QRect &boundingRect, const QRect &iconRect, const bool extraWidthMargin) const
{
    QRect result = boundingRect;
    //名字宽度需要考虑到未打开应用的小蓝点标识
    if(extraWidthMargin) result.setWidth(result.width() + m_blueDotPixmap.width());
    result.setTop(iconRect.bottom());
    return result;
}

const QPair<QString, bool> AppItemDelegate::holdTextInRect(const QFontMetrics &fm, const QString &text, const QRect &rect) const
{
    const int textFlag = Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap;

    if (rect.contains(fm.boundingRect(rect, textFlag, text)))
        return QPair<QString, bool>(text, true);

    QString str(text + "...");

    while (true)
    {
        if (str.size() < 4)
            break;

        QRect boundingRect = fm.boundingRect(rect, textFlag, str);
        if (rect.contains(boundingRect))
            break;

        str.remove(str.size() - 4, 1);
    }

    return QPair<QString, bool>(str, false);
}
