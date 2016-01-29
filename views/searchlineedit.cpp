#include "searchlineedit.h"
#include "dlineedit.h"
#include "app/global.h"
#include "launcherframe.h"
#include <QLabel>
#include <QResizeEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QHBoxLayout>
#include <QRegExp>
#include <QRegExpValidator>
#include <QDebug>


int LineEditStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
if (metric == QStyle::PM_TextCursorWidth)
    return 0;
return QCommonStyle::pixelMetric(metric,option,widget);
}


SearchLineEdit::SearchLineEdit(QWidget *parent) :
    QFrame(parent),
    m_iconLabel(new QLabel(this)),
    m_lineEdit(new DLineEdit(this))

{
    initUI();
    initConnect();
    handleTextChanged("");
    setStyle(new LineEditStyle);
    installEventFilter(this);
}

void SearchLineEdit::initUI(){
    m_iconLabel->setPixmap(QPixmap(":/images/skin/img/search.png"));
    m_iconLabel->setFixedWidth(30);
    m_lineEdit->setObjectName("SearchLineEdit");
    m_lineEdit->setFixedHeight(30);
    m_lineEdit->setMaxLength(255);
//    m_lineEdit->setReadOnly(true);
//    m_lineEdit->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_iconLabel);
    searchLayout->addWidget(m_lineEdit);
    searchLayout->setSpacing(0);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(searchLayout);
    setMinimumSize(45, 50);

    m_iconLabel->hide();
}

void SearchLineEdit::initConnect(){
    connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(handleTextChanged(QString)));
}

QString SearchLineEdit::getText(){
    return m_lineEdit->text();
}

void SearchLineEdit::setText(const QString &text){
    m_lineEdit->setText(text);
//    setSearchFocus();
}

void SearchLineEdit::handleTextChanged(const QString &text){
    QFontMetrics f = m_lineEdit->fontMetrics();
    int fw = f.width(text);
    m_lineEdit->setFixedSize(fw + 10, m_lineEdit->height());
    setFixedSize(m_lineEdit->width() + m_iconLabel->width(), height());
    textChanged(text);
    if (text.length() == 0){
        m_iconLabel->hide();
    }else{
        m_iconLabel->show();
    }
}

void SearchLineEdit::setSearchFocus(){
    m_lineEdit->setFocus();
//    m_lineEdit->setCursorPosition(-100);
}

void SearchLineEdit::resizeEvent(QResizeEvent *event){
    int screenWidth = static_cast<QWidget*>(parent())->width();
    move((screenWidth - width()) / 2, 5);
    QFrame::resizeEvent(event);
}


void SearchLineEdit::keyPressEvent(QKeyEvent *event) {
    qDebug() << event;
    if (event->key() == Qt::Key_Escape) {
        if (m_lineEdit->text().length() > 0) {
            m_lineEdit->setText("");
        } else {
            emit signalManager->Hide();
        }
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Up) {
        emit signalManager->keyDirectionPressed(Qt::Key_Up);
    } else if (event->modifiers() == Qt::CTRL && event->key() == Qt::Key_P){
        emit signalManager->keyDirectionPressed(Qt::Key_Up);
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Down) {
        emit signalManager->keyDirectionPressed(Qt::Key_Down);
    } else if (event->modifiers() == Qt::CTRL && event->key() == Qt::Key_N) {
        emit signalManager->keyDirectionPressed(Qt::Key_Down);
    } else if (event->modifiers() == Qt::CTRL && event->key() == Qt::Key_B) {
        emit signalManager->keyDirectionPressed(Qt::Key_Left);
    } else if (event->modifiers() == Qt::CTRL && event->key() == Qt::Key_F) {
        emit signalManager->keyDirectionPressed(Qt::Key_Right);
    } else if ((event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Return) ||
              (event->modifiers() == Qt::KeypadModifier && event->key() == Qt::Key_Enter)) {
        int index = static_cast<LauncherFrame*>(parent())->currentMode();
        if ( index == 0) {
            emit signalManager->appOpenedInCategoryMode();
        } else {
            emit signalManager->appOpenedInAppMode();
        }
    } else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Tab) {
        emit signalManager->keyDirectionPressed(Qt::Key_Right);
    } else if ((event->modifiers() & Qt::ShiftModifier) && event->key() == Qt::Key_Backtab) {
        emit signalManager->keyDirectionPressed(Qt::Key_Left);
    }

    QFrame::keyPressEvent(event);
}
