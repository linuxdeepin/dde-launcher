#include "searchwidget.h"

#include <QHBoxLayout>

SearchWidget::SearchWidget(QWidget *parent) :
    QWidget(parent),
    m_searchEdit(new DSearchEdit)
{
    m_searchEdit->setFixedWidth(200);
    m_searchEdit->setFocus();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_searchEdit);

    setFocusPolicy(Qt::StrongFocus);
    setLayout(mainLayout);
}
