#include "searchwidget.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent)
{
    m_searchEdit = new SearchLineEdit(this);
    m_searchEdit->setAccessibleName("search-edit");

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_searchEdit);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    connect(m_searchEdit, &SearchLineEdit::textChanged, [this] {
        emit searchTextChanged(m_searchEdit->text().trimmed());
    });
}

QLineEdit *SearchWidget::edit()
{
    return m_searchEdit;
}

void SearchWidget::clearSearchContent()
{
    m_searchEdit->normalMode();
}
