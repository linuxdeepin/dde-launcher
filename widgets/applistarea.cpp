#include "applistarea.h"

AppListArea::AppListArea(QWidget *parent)
    : QScrollArea(parent)
{

}

void AppListArea::enterEvent(QEvent *e)
{
    QScrollArea::enterEvent(e);

    emit mouseEntered();
}
