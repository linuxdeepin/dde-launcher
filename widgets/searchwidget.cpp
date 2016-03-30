#include "searchwidget.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent)
{
    setObjectName("SearchFrame");

    m_searchInputWidget = new SearchInputWidget(this);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchInputWidget);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 33, 0, 0);
    setLayout(mainLayout);

    connect(m_searchInputWidget, &SearchInputWidget::textChanged, [this] {
        emit searchTextChanged(m_searchInputWidget->text());
    });
}

QLineEdit *SearchWidget::edit()
{
    return m_searchInputWidget->lineEdit();
}

void SearchWidget::clearSearchContent()
{
    m_searchInputWidget->clear();
}

bool SearchWidget::event(QEvent *e)
{
//    if (e->type() == QEvent::FocusIn)
//        m_searchEdit->setFocus();

    return QWidget::event(e);
}
