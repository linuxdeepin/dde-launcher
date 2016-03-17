#include "searchwidget.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>

SearchWidget::SearchWidget(QWidget *parent) :
    QWidget(parent),
    m_searchEdit(new DSearchEdit)
{
    m_searchEdit->setFixedWidth(200);
    m_searchEdit->setFocus();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();

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

bool SearchWidget::event(QEvent *e)
{
    if (e->type() == QEvent::FocusIn)
        m_searchEdit->setFocus();

    return QWidget::event(e);
}
