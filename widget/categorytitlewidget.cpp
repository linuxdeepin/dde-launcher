#include "categorytitlewidget.h"
#include "constants.h"

#include <QHBoxLayout>

CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QWidget(parent),
    m_title(new QLabel(title))
{
    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(m_title);

    setLayout(mainLayout);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);
}
