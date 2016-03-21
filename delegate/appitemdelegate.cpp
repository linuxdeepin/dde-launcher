
#include "appitemdelegate.h"
#include "global_util/constants.h"
#include "model/appslistmodel.h"
#include "dbus/dbusvariant/iteminfo.h"

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

    const QRect squaredRect = getSquareRect(option.rect);
    // draw focus background
   if (CurrentIndex == index)
    {
        const QColor borderColor(255, 255, 255, 51);
        const QColor brushColor(0, 0, 0, 76);

        QPen pen;
        pen.setColor(borderColor);
        pen.setWidth(2);
        QPainterPath border;
        border.addRoundedRect(squaredRect.marginsRemoved(QMargins(2, 2, 2, 2)), 4, 4);
        painter->strokePath(border, pen);

        painter->setBrush(brushColor);
        painter->drawRoundedRect(squaredRect.marginsRemoved(QMargins(2, 2, 2, 2)), 4, 4);
    }

    // draw app icon

    painter->drawPixmap(squaredRect.marginsRemoved(QMargins(0.2*squaredRect.width(), squaredRect.width()*1/15, 0.2*squaredRect.width(), squaredRect.width()*5/15)),
                        index.data(AppsListModel::AppIconRole).value<QPixmap>());

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(squaredRect.x() + squaredRect.width()*0.2+5, squaredRect.y() + squaredRect.width()*80/150, 16, 16, QPixmap(":/skin/images/emblem-autostart.png"));

    // draw app name
    painter->setBrush(QBrush(Qt::transparent));

    QFont font = painter->font();
    font.setPixelSize(index.data(AppsListModel::AppFontSizeRole).value<int>());
    painter->setFont(font);

    int leftMargin = 10;
    QRect textRect = QRect(squaredRect.x() + leftMargin, option.rect.y() + squaredRect.width()*110/150, option.rect.width() - leftMargin*2, option.rect.height() - leftMargin);
    painter->setPen(QColor(0, 0, 0, 170));
    painter->drawText(QRectF(textRect.x(), textRect.y() + 0.5, textRect.width(), textRect.height()), Qt::TextWordWrap|Qt::AlignHCenter, itemInfo.m_name);
    painter->setPen(Qt::white);

    painter->drawText(textRect, Qt::TextWordWrap|Qt::AlignHCenter, itemInfo.m_name);
    // draw blue dot if new install
    if (index.data(AppsListModel::AppNewInstallRole).toBool())
    {
        painter->setPen(Qt::transparent);
        painter->setBrush(QColor(0, 127, 244));
        painter->drawEllipse(QPoint(squaredRect.x() + squaredRect.width()*8/150, squaredRect.y() + squaredRect.width()*118/150), 4, 4);
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
