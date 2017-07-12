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

public slots:
    void setActived(bool active);

private:
    bool m_actived;
    QPixmap m_blueDotPixmap;
    QPixmap m_autoStartPixmap;
};

#endif // APPLISTDELEGATE_H
