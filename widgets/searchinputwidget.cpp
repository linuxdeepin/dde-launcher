#include "searchinputwidget.h"
#include <QDebug>

SearchLineEdit::SearchLineEdit(QWidget *parent)
    : QLineEdit(parent) {
    this->setStyleSheet("background-color:rgba(255, 255, 255, 0);"
                                "padding:0px 5px;"
                                "border:none;"
                                "color:white;");

}

SearchLineEdit::~SearchLineEdit()
{}

void SearchLineEdit::focusInEvent(QFocusEvent *e) {
    qDebug() << "focusInEvent";
    this->setFocusPolicy(Qt::StrongFocus);
    emit focusChanged(true);
    Q_UNUSED(e);
}

void SearchLineEdit::focusOutEvent(QFocusEvent *e) {
    emit focusChanged(false);
    this->clear();
    Q_UNUSED(e);
}

//void SearchLineEdit::mousePressEvent(QMouseEvent *e) {
//}

//void SearchLineEdit::mouseReleaseEvent(QMouseEvent *e) {
//}

SearchInputWidget::SearchInputWidget(QFrame *parent)
    : QFrame(parent),
    m_searchIcon(new QLabel(this)),
    m_searchLineEdit(new SearchLineEdit(this))
{
    m_searchIcon->setFixedSize(16, 16);
    m_searchIcon->setPixmap(QPixmap(":/skin/images/search_normal.png"));
    m_searchIcon->setStyleSheet("background-color: transparent;");
    m_searchLineEdit->setFixedSize(150, 30);
    this->setFixedSize(320, 30);
    m_Layout = new QHBoxLayout;
    m_Layout->setMargin(0);
    m_Layout->addStretch();
    m_Layout->addSpacing(16);
    m_Layout->addWidget(m_searchLineEdit);


    m_searchIcon->move(150, 8);
    setLayout(m_Layout);

    setStyleSheet("background-color: rgba(255, 255, 255, .3);"
                  "border:none;"
                  "border-radius: 5px;");

    connect(m_searchLineEdit, &SearchLineEdit::focusChanged, this,
            &SearchInputWidget::handleInputFocusChanged);
    connect(m_searchLineEdit, &SearchLineEdit::textChanged, this,
            &SearchInputWidget::textChanged);
}

QLineEdit* SearchInputWidget::lineEdit() {
    return m_searchLineEdit;
}

void SearchInputWidget::handleInputFocusChanged(bool isFocus) {
    if (isFocus) {
        m_searchIcon->move(12, 8);
        m_searchLineEdit->setFixedSize(290, 30);
    } else {
        m_searchIcon->move(150, 8);
        m_searchLineEdit->setFixedSize(150, 30);
    }
}

const QString SearchInputWidget::text() {
    return m_searchLineEdit->text();
}

void SearchInputWidget::clear() {
    m_searchLineEdit->clear();
    handleInputFocusChanged(false);
}

SearchInputWidget::~SearchInputWidget(){

}
