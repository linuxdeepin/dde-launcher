#include "categorytitlewidget.h"
#include "global_util/constants.h"

#include <QHBoxLayout>

CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QWidget(parent),
    m_title(new QLabel(title))
{
    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(m_title);

    setLayout(mainLayout);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);
    setStyleSheet("background-color: rgba(118, 238, 198, 255)");
}
