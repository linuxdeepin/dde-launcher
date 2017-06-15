#include "applistdelegate.h"
#include "model/appslistmodel.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>

AppListDelegate::AppListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_autoStartPixmap(":/skin/images/emblem-autostart.png")
{

}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QRect r = option.rect;
    const QPixmap icon = index.data(AppsListModel::AppIconRole).value<QPixmap>();

    painter->setRenderHint(QPainter::Antialiasing);

    if (option.state.testFlag(QStyle::State_Selected))
    {
        painter->setBrush(QColor(0, 0, 0, 255 * .2));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 4, 4);
    }

    painter->drawPixmap(r.x() + 10, r.y() + (r.height() - icon.height()) / 2, icon);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(r.x() + 10, r.y() + (r.height() - icon.height()) / 2 + m_autoStartPixmap.height(), m_autoStartPixmap);

    painter->setPen(Qt::white);
    painter->drawText(r.marginsRemoved(QMargins(50, 0, 0, 0)), Qt::AlignVCenter | Qt::AlignLeft, index.data(AppsListModel::AppNameRole).toString());
}

QSize AppListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(0, 50);
}
