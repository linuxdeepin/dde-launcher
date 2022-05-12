#include "appdirwidget.h"

#include <QHBoxLayout>

#include "calculate_util.h"
#include "appsmanager.h"
#include "constants.h"

AppDirWidget::AppDirWidget(QWidget *parent)
    : QWidget (parent)
{
    setWindowFlag(Qt::Popup);
}

AppDirWidget::~AppDirWidget()
{

}

void AppDirWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}
