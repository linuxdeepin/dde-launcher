
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

    // draw focus background
   if (CurrentIndex == index)
    {
        const QColor borderColor(255, 255, 255, 51);
        const QColor brushColor(0, 0, 0, 76);

        QPen pen;
        pen.setColor(borderColor);
        pen.setWidth(2);
        QPainterPath border;
        border.addRoundedRect(option.rect.marginsRemoved(QMargins(2, 2, 2, 2)), 4, 4);
        painter->strokePath(border, pen);

        painter->setBrush(brushColor);
        painter->drawRoundedRect(option.rect.marginsRemoved(QMargins(2, 2, 2, 2)), 4, 4);
    }

    // draw app icon
    painter->drawPixmap(option.rect.marginsRemoved(QMargins(30, 20, 30, 40)),
                        index.data(AppsListModel::AppIconRole).value<QPixmap>());

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(option.rect.x(), option.rect.y(), 16, 16, QPixmap(":/skin/images/emblem-autostart.png"));

    // draw app name
    painter->setPen(Qt::white);
    painter->setBrush(QBrush(Qt::transparent));
    QRect textRect = QRect(option.rect.x() + 20, option.rect.y() + 115, option.rect.width() - 20, option.rect.height() - 20);
    painter->drawText(textRect, itemInfo.m_name);

    // draw blue dot if new install
    if (index.data(AppsListModel::AppNewInstallRole).toBool())
    {
        painter->setPen(Qt::black);
        painter->setBrush(Qt::blue);
        painter->drawEllipse(option.rect.center(), 10, 10);
    }
}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}
