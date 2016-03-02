#include "applistview.h"

#include <QDebug>

AppListView::AppListView(QWidget *parent) :
    QListView(parent)
{
    setFlow(QListView::LeftToRight);
    setLayoutMode(QListView::Batched);
    setResizeMode(QListView::Adjust);
    setWrapping(true);
}
