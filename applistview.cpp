#include "applistview.h"

#include <QDebug>
#include <QWheelEvent>

AppListView::AppListView(QWidget *parent) :
    QListView(parent)
{
    setFlow(QListView::LeftToRight);
    setLayoutMode(QListView::Batched);
    setResizeMode(QListView::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setWrapping(true);
}

QSize AppListView::sizeHint() const
{
    QSize hint = QListView::sizeHint();

//    if (!model())
//        return hint;

//    const int extraHeight = height() - viewport()->height();
//    const QModelIndex lastIndex = model()->index(model()->rowCount() - 1, 0);
//    const QRect rect = visualRect(lastIndex);

//    qDebug() << rect;
////    hint.setHeight(rect.y() + rect.height()/* + extraHeight*/);
//    hint = QSize(rect.x() + rect.width(), rect.y() + rect.height());

//    qDebug() << "hint = " << hint;

    return hint;
}

void AppListView::updatea()
{
    qDebug() << viewport()->size();
    qDebug() << contentsSize() << contentsRect();
//    setFixedSize(contentsSize();
    setFixedWidth(contentsRect().width());
    setFixedHeight(contentsSize().height());
}

void AppListView::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);

    qDebug() << viewport()->size();
    qDebug() << contentsSize() << contentsRect();
//    setFixedSize(contentsSize();
    setFixedWidth(contentsRect().width());
    setFixedHeight(contentsSize().height());
}

void AppListView::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}
