#include "appdirwidget.h"

/**TODO: 后面提交更新
 * @brief AppDirWidget::AppDirWidget
 * @param parent
 */
AppDirWidget::AppDirWidget(QWidget *parent)
    : QWidget (parent)
{
    setWindowFlag(Qt::Popup);
}

void AppDirWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}
