#include "searchwidget.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent),
    m_searchEdit(new QLineEdit)
{
    setObjectName("SearchFrame");
    m_searchEdit->setFixedSize(290, 30);

    m_searchEdit->setFocus();
    m_searchEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_searchEdit->setStyleSheet("background-color:rgba(255, 255, 255, .3);"
                                "padding:0px 5px;"
                                "border:none;"
                                "border-radius:5px;"
                                "color:white;");

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 33, 0, 0);

    setFocusPolicy(Qt::NoFocus);
    setFocusProxy(m_searchEdit);
    setLayout(mainLayout);
//    m_searchEdit->setStyleSheet(getQssFromFile(":/skin/qss/searchwidget.qss"));

    connect(m_searchEdit, &QLineEdit::textChanged, [this] {
        emit searchTextChanged(m_searchEdit->text());
    });
}

QLineEdit *SearchWidget::edit()
{
    return m_searchEdit;
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
