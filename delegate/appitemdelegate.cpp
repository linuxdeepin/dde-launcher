#include "appitemdelegate.h"
#include "model/appslistmodel.h"

#include <QDebug>

QModelIndex AppItemDelegate::CurrentIndex = QModelIndex();

AppItemDelegate::AppItemDelegate(QObject *parent) :
    QAbstractItemDelegate(parent)
{

}

void AppItemDelegate::setCurrentIndex(const QModelIndex &index)
{
    CurrentIndex = index;
}

void AppItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect itemRect = option.rect;
    painter->setPen(Qt::black);

    painter->setBrush(QBrush(QColor(169, 169, 169, 50)));
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
    qDebug() << "option" << option.rect;
    painter->drawRect(itemRect);
    int margins = 15;
    QRect textRect = QRect(itemRect.x() + margins, itemRect.y() + margins, itemRect.width() - margins*2,
                           itemRect.height() -margins*2);

    QPixmap itemMap = QPixmap(index.data(AppsListModel::AppIconRole).toString());
    painter->drawPixmap(textRect, itemMap);

}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}
