#include "applistview.h"

AppListView::AppListView(QWidget *parent)
    : QListView(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(NoSelection);
    setFixedSize(480, 430);
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
}
