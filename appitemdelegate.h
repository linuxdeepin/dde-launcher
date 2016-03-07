#ifndef APPITEMDELEGATE_H
#define APPITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QPainter>

class AppItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AppItemDelegate(QObject *parent = 0);

    static void setCurrentIndex(const QModelIndex &index);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    static QModelIndex CurrentIndex;
};

#endif // APPITEMDELEGATE_H
