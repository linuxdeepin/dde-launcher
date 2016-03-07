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

    painter->setBrush(QBrush(QColor(238, 23, 238)));
    qDebug() << "option" << option.rect
             << option.icon << index.data(AppsListModel::AppIconRole).toString();
//    painter->drawPixmap(itemRect.x(), itemRect.y(), itemRect.width(), itemRect.height(), QIcon(option.icon));
    painter->drawRect(itemRect);
    QRect textRect = QRect(itemRect.x(), itemRect.y() + 110, itemRect.width(), itemRect.height());
    painter->drawText(textRect, index.data(AppsListModel::AppNameRole).toString());
//    qDebug() << "%%%%%%%%%%%" << index.data(AppsListModel::AppIconRole).toString();
//    QPixmap itemMap = QPixmap(index.data(AppsListModel::AppIconRole).toString());
//    QPixmap itemMap = QPixmap(":/skin/images/googleChrome.png");
//    painter->drawPixmap(itemRect, itemMap);

}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}
