#include "appitemdelegate.h"
#include "model/appslistmodel.h"

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
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);

    if (CurrentIndex == index)
        painter->setBrush(QBrush(QColor(23, 238, 238)));
    else
        painter->setBrush(QBrush(QColor(238, 23, 238)));
//    qDebug() << "option" << option.rect
//             << option.icon << index.data(AppsListModel::AppIconRole).toString();
//    painter->drawPixmap(itemRect.x(), itemRect.y(), itemRect.width(), itemRect.height(), QIcon(option.icon));
    painter->drawRect(option.rect);
//    QRect textRect = QRect(itemRect.x(), itemRect.y() + 110, itemRect.width(), itemRect.height());
//    painter->drawText(textRect, index.data(AppsListModel::AppNameRole).toString());
//    qDebug() << "%%%%%%%%%%%" << index.data(AppsListModel::AppIconRole).toString();
//    QPixmap itemMap = QPixmap(index.data(AppsListModel::AppIconRole).toString());
//    QPixmap itemMap = QPixmap(":/skin/images/googleChrome.png");
//    painter->drawPixmap(itemRect, itemMap);

    // draw app icon
    painter->drawPixmap(option.rect.marginsRemoved(QMargins(30, 20, 30, 40)),
                        index.data(AppsListModel::AppIconRole).value<QPixmap>());

    // draw app name
    painter->setPen(Qt::black);
    painter->setBrush(QBrush(Qt::blue));
    painter->boundingRect(option.rect,
                          index.data(AppsListModel::AppNameRole).toString());
}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}
