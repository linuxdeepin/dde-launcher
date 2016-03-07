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
    painter->setPen(Qt::black);
    if (index == CurrentIndex)
        painter->setBrush(QBrush(QColor(23, 238, 238)));
    else
        painter->setBrush(QBrush(QColor(238, 23, 238)));
    painter->drawRect(option.rect);
    painter->drawText(option.rect, index.data(AppsListModel::AppNameRole).toString());
}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}
