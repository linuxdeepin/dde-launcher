#include "applistdelegate.h"
#include "model/appslistmodel.h"

#include <QPainter>
#include <QDebug>

AppListDelegate::AppListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{

}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QRect r = option.rect;

    const QPixmap icon = index.data(AppsListModel::AppIconRole).value<QPixmap>();

    painter->setPen(Qt::red);
    painter->fillRect(r, Qt::cyan);
    painter->drawPixmap(r.x() + 10, r.y() + (r.height() - icon.height()) / 2, icon);
    painter->drawText(r.marginsRemoved(QMargins(50, 0, 0, 0)), Qt::AlignVCenter | Qt::AlignLeft, index.data(AppsListModel::AppNameRole).toString());
}

QSize AppListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(0, 50);
}
