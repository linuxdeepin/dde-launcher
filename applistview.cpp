#include "applistview.h"

#include <QDebug>
#include <QWheelEvent>
#include <QTimer>

AppListView::AppListView(QWidget *parent) :
    QListView(parent)
{
    viewport()->installEventFilter(this);

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
    return QListView::sizeHint();
}

void AppListView::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);
}

void AppListView::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}

bool AppListView::eventFilter(QObject *o, QEvent *e)
{
    if (o == viewport() && e->type() == QEvent::Paint)
        fitToContent();

    return false;
}

void AppListView::fitToContent()
{
    if (width() == contentsRect().width() && height() == contentsSize().height())
        return;

    setFixedWidth(contentsRect().width());
    setFixedHeight(contentsSize().height());
}
