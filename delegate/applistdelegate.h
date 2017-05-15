#ifndef APPLISTDELEGATE_H
#define APPLISTDELEGATE_H

#include <QAbstractItemDelegate>

class AppListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AppListDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // APPLISTDELEGATE_H
