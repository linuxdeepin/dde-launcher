
#include "appitemdelegate.h"
#include "global_util/constants.h"
#include "global_util/calculate_util.h"
#include "model/appslistmodel.h"
#include "dbusinterface/dbusvariant/iteminfo.h"

#include <QDebug>
#include <QPixmap>
#include <QVariant>

QModelIndex AppItemDelegate::CurrentIndex = QModelIndex();

AppItemDelegate::AppItemDelegate(QObject *parent) :
    QAbstractItemDelegate(parent),
    m_calcUtil(CalculateUtil::instance(this)),
    m_blueDotPixmap(":/skin/images/new_install_indicator.png"),
    m_autoStartPixmap(":/skin/images/emblem-autostart.png")
{

}

void AppItemDelegate::setCurrentIndex(const QModelIndex &index)
{
    if (CurrentIndex == index)
        return;

    const QModelIndex previousIndex = CurrentIndex;
    CurrentIndex = index;

    emit currentChanged(previousIndex, CurrentIndex);
}

void AppItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::white);
    painter->setBrush(QBrush(Qt::transparent));

    const ItemInfo itemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo>();
    const int leftMargin = 2, radius = 3;
    const QRect itemRect = getSquareRect(option.rect);
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).toSize();

    // draw focus background
   if (CurrentIndex == index)
    {
        const QColor borderColor(255, 255, 255, 52);
        const QColor brushColor(0, 0, 0, 105);

        QPen pen;
        pen.setColor(borderColor);
        pen.setWidth(2);
        QPainterPath border;
        border.addRoundedRect(itemRect.marginsRemoved(QMargins(leftMargin/2, leftMargin/2, leftMargin*2/3, leftMargin*2/3)),
                              radius, radius);
        painter->strokePath(border, pen);
        painter->setPen(Qt::transparent);
        painter->setBrush(brushColor);
        painter->drawRoundedRect(itemRect.marginsRemoved(QMargins(leftMargin, leftMargin, leftMargin*4/3, leftMargin*4/3)),
                                 radius, radius);
    }

    // draw app icon
    const QPixmap iconPix = index.data(AppsListModel::AppIconRole).value<QPixmap>();
    int iconLeftMargins = (itemRect.width() - iconSize.width())/2;
    int iconTopMargin = itemRect.height()*0.1;
    painter->drawPixmap(itemRect.x() + iconLeftMargins, itemRect.y()+iconTopMargin, iconSize.width(), iconSize.height(),
                        iconPix);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(itemRect.x() + iconLeftMargins, itemRect.y() + iconTopMargin + iconSize.height()-16,
                            16, 16, m_autoStartPixmap);

    // draw app name
    painter->setBrush(QBrush(Qt::transparent));

    QFont font = painter->font();
    font.setPixelSize(index.data(AppsListModel::AppFontSizeRole).value<int>());
    painter->setFont(font);

    int textTopMargin = itemRect.width()*0.73;
    QRect textRect = QRect(itemRect.x() + leftMargin * 3, itemRect.y() + textTopMargin, itemRect.width() - leftMargin * 6, 36);

    painter->setPen(QColor(0, 0, 0, 80));
    painter->drawText(QRectF(textRect.x() + 0.8, textRect.y() + 1, textRect.width(), textRect.height()), Qt::TextWordWrap | Qt::AlignHCenter, itemInfo.m_name);
    painter->drawText(QRectF(textRect.x() - 0.8, textRect.y() + 1, textRect.width(), textRect.height()), Qt::TextWordWrap | Qt::AlignHCenter, itemInfo.m_name);

    painter->setPen(Qt::white);
    painter->drawText(textRect, Qt::TextWordWrap | Qt::AlignHCenter, itemInfo.m_name);

    // draw blue dot if new install
    if (index.data(AppsListModel::AppNewInstallRole).toBool())
    {
        QFontMetrics fm(painter->font());
        const QRect boundingRect = fm.boundingRect(textRect, Qt::TextWordWrap | Qt::AlignHCenter, itemInfo.m_name);

        QRect bluePointRect = QRect(itemRect.x() + (textRect.width() - boundingRect.width()) / 2 - 8, itemRect.y() + itemRect.width() * 111 / 150, 10, 10);
        painter->drawPixmap(bluePointRect,  m_blueDotPixmap);
    }
}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}

const QRect AppItemDelegate::getSquareRect(const QRect &itemRect) const
{
    const int w = itemRect.width();
    const int h = itemRect.height();
    const int sub = qAbs((w - h) / 2);

    if (w == h)
        return itemRect;
    else if (w > h)
        return itemRect - QMargins(sub, 0, sub, 0);
    else
        return itemRect - QMargins(0, sub, 0, sub);
}
