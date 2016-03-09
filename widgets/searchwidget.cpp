#include "searchwidget.h"

#include <QHBoxLayout>

SearchWidget::SearchWidget(QWidget *parent) :
    QWidget(parent),
    m_searchEdit(new DSearchEdit)
{
    m_searchEdit->setFixedWidth(200);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_searchEdit);

    setLayout(mainLayout);
}
