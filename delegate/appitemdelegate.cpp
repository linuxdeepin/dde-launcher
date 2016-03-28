
#include "appitemdelegate.h"
#include "global_util/constants.h"
#include "model/appslistmodel.h"
#include "dbusinterface/dbusvariant/iteminfo.h"

#include <QDebug>
#include <QPixmap>
#include <QVariant>

QModelIndex AppItemDelegate::CurrentIndex = QModelIndex();

AppItemDelegate::AppItemDelegate(QObject *parent) :
    QAbstractItemDelegate(parent)
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
                            16, 16, QPixmap(":/skin/images/emblem-autostart.png"));

    // draw app name
    painter->setBrush(QBrush(Qt::transparent));

    QFont font = painter->font();
    font.setPixelSize(qMax(index.data(AppsListModel::AppFontSizeRole).value<int>(), 4));
    painter->setFont(font);

    int textTopMargin = itemRect.width()*0.73;
    QRect textRect = QRect(itemRect.x() + leftMargin, itemRect.y() + textTopMargin, itemRect.width() - leftMargin*2, itemRect.height() - leftMargin*2);

    painter->setPen(QColor(0, 0, 0, 170));
    painter->drawText(QRectF(textRect.x(), textRect.y() + 0.5, textRect.width(), textRect.height()), Qt::TextWordWrap|Qt::AlignHCenter, itemInfo.m_name);
    painter->setPen(Qt::white);

    painter->drawText(textRect, Qt::TextWordWrap|Qt::AlignHCenter, itemInfo.m_name);
    // draw blue dot if new install

    if (index.data(AppsListModel::AppNewInstallRole).toBool())
    {
        QRect bluePointRect = QRect(itemRect.x() + 1, itemRect.y() + itemRect.width()*112/150, 10, 10);
        painter->drawPixmap(bluePointRect,  QPixmap(":/skin/images/new_install_indicator.png"));
    }


}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}

const QRect AppItemDelegate::getSquareRect(const QRect &itemRect) const {
    if (itemRect.width() == itemRect.height()) {
        return itemRect;
    } else if (itemRect.width() > itemRect.height()) {
        int tmpX = itemRect.x() + (itemRect.width() - itemRect.height())/2;
        return QRect(tmpX, itemRect.y(), itemRect.height(), itemRect.height());
    } else {
        int tmpY = itemRect.y() + (itemRect.height() - itemRect.width())/2;
        return QRect(itemRect.x(), tmpY, itemRect.width(), itemRect.width());
    }
}
