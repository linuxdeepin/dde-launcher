#ifndef APPITEMDELEGATE_H
#define APPITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QPainter>

class CalculateUtil;
class AppItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AppItemDelegate(QObject *parent = 0);

    void setCurrentIndex(const QModelIndex &index);
    const QModelIndex &currentIndex() const {return CurrentIndex;}

signals:
    void currentChanged(const QModelIndex previousIndex, const QModelIndex &currentIndex) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    const QRect itemBoundingRect(const QRect &itemRect) const;
    const QRect itemTextRect(const QRect &boundingRect, const QRect &iconRect, const bool extraWidthMargin) const;
    const QString holdTextInRect(const QFontMetrics &fm, const QString &text, const QRect &rect) const;

private:
    CalculateUtil *m_calcUtil;
    QPixmap m_blueDotPixmap;
    QPixmap m_autoStartPixmap;

    static QModelIndex CurrentIndex;
};

#endif // APPITEMDELEGATE_H
