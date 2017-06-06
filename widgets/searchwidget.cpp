#include "searchwidget.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent)
{
//    setObjectName("SearchFrame");
    this->setFixedWidth(300);
    m_searchEdit = new SearchLineEdit(this);
    m_searchEdit->setAccessibleName("search-edit");

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();
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

//bool SearchWidget::event(QEvent *e)
//{
//    if (e->type() == QEvent::FocusIn)
//        m_searchEdit->setFocus();

//    return QWidget::event(e);
//}
