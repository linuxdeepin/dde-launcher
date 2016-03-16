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

    connect(m_searchEdit, &DSearchEdit::textChanged, [this] {
        emit searchTextChanged(m_searchEdit->text());
    });
}

void SearchWidget::clearSearchContent()
{
    m_searchEdit->clear();
}
