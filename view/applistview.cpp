#include "applistview.h"

#include <QMouseEvent>
#include <QDebug>

AppListView::AppListView(QWidget *parent)
    : QListView(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(SingleSelection);
    setFixedSize(380, 410);
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
}

void AppListView::mousePressEvent(QMouseEvent *e)
{
    QListView::mousePressEvent(e);

    if (e->buttons() == Qt::RightButton) {
        const QPoint rightClickPoint = mapToGlobal(e->pos());

        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());
        if (clickedIndex.isValid())
            emit popupMenuRequested(rightClickPoint, clickedIndex);
    }
}
