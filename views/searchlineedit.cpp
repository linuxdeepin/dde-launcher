#include "searchlineedit.h"
#include <QLineEdit>
#include <QLabel>
#include <QResizeEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QHBoxLayout>
#include <QRegExp>
#include <QRegExpValidator>
#include <QDebug>

SearchLineEdit::SearchLineEdit(QWidget *parent) :
    QFrame(parent),
    m_lineEdit(new QLineEdit),
    m_iconLabel(new QLabel)
{
    initUI();
    initConnect();
    handleTextChanged("");
}

void SearchLineEdit::initUI(){
    m_iconLabel->setPixmap(QPixmap(":/images/skin/img/search.png"));
    m_iconLabel->setFixedWidth(30);
    m_lineEdit->setObjectName("SearchLineEdit");
    m_lineEdit->setFixedHeight(30);
    m_lineEdit->setMaxLength(255);


    QHBoxLayout* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_iconLabel);
    searchLayout->addWidget(m_lineEdit);
    searchLayout->setSpacing(0);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(searchLayout);
    setMinimumSize(45, 50);
}

void SearchLineEdit::initConnect(){
    connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(handleTextChanged(QString)));
}

void SearchLineEdit::setText(const QString &text){
    m_lineEdit->setText(text);
    setSearchFocus();
}

void SearchLineEdit::handleTextChanged(const QString &text){
    QFontMetrics f = m_lineEdit->fontMetrics();
    int fw = f.width(text);
    m_lineEdit->setFixedSize(fw + 10, m_lineEdit->height());
    setFixedSize(m_lineEdit->width() + m_iconLabel->width(), height());
    textChanged(text);
}

void SearchLineEdit::setSearchFocus(){
    m_lineEdit->setFocus();
}

void SearchLineEdit::resizeEvent(QResizeEvent *event){
    int screenWidth = qApp->desktop()->screenGeometry().width();
    move((screenWidth - width()) / 2, 5);
    QFrame::resizeEvent(event);
}
