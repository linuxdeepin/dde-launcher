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
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // draw focus background
    if (CurrentIndex == index)
    {
        const QColor borderColor(255, 255, 255, 51);
        const QColor brushColor(0, 0, 0, 76);

        QPen pen;
        pen.setColor(borderColor);
        pen.setWidth(2);
        QPainterPath border;
        border.addRoundedRect(1, 1, 148, 148, 4, 4);
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
