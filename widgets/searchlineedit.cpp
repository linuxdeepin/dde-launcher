
#include "searchlineedit.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QEvent>

SearchLineEdit::SearchLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    m_icon = new DImageButton;
    m_icon->setFixedSize(16, 16);
    m_icon->setNormalPic(":/skin/images/search_normal.png");
    m_icon->setHoverPic(":/skin/images/search_hover.png");
    m_icon->setPressPic(":/skin/images/search_press.png");
    m_placeholderText = new QLabel(tr("Search"));
    QFontMetrics fm(m_placeholderText->font());
    m_placeholderText->setFixedWidth(fm.width(m_placeholderText->text()) + 10);
    m_placeholderText->setStyleSheet("color:white;");
    m_floatWidget = new QWidget(this);

    QHBoxLayout *floatLayout = new QHBoxLayout;
    floatLayout->addWidget(m_icon);
    floatLayout->setAlignment(m_icon, Qt::AlignVCenter);
    floatLayout->addStretch();
    floatLayout->addWidget(m_placeholderText);
    floatLayout->setAlignment(m_placeholderText, Qt::AlignVCenter);
    floatLayout->setSpacing(0);
    floatLayout->setMargin(0);

    m_floatWidget->setFixedHeight(30);
    m_floatWidget->setFixedWidth(m_icon->width() + m_placeholderText->width() + 5);
    m_floatWidget->setLayout(floatLayout);
//    m_floatWidget->setStyleSheet("border:1px solid red;");

    m_floatAni = new QPropertyAnimation(m_floatWidget, "pos", this);

    setFixedSize(290, 30);
    setStyleSheet("QLineEdit {"
                  "background-color:rgba(255, 255, 255, .2);"
                  "padding:0px 0 0 25px;"
                  "border:none;"
                  "border-radius:5px;"
                  "color:white;"
                  "}");

    m_floatWidget->move(rect().center() - m_floatWidget->rect().center());
}

bool SearchLineEdit::event(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::FocusIn:       editMode();         break;
//    case QEvent::FocusOut:      normalMode();       break;
    default:;
    }

    return QLineEdit::event(e);
}

void SearchLineEdit::normalMode()
{
    // clear text when back to normal mode
    clear();

    m_placeholderText->show();

    m_floatAni->stop();
    m_floatAni->setStartValue(m_floatWidget->pos());
    m_floatAni->setEndValue(rect().center() - m_floatWidget->rect().center());
    m_floatAni->start();
}

void SearchLineEdit::editMode()
{
    m_placeholderText->hide();

    m_floatAni->stop();
    m_floatAni->setStartValue(m_floatWidget->pos());
    m_floatAni->setEndValue(QPoint(5, 0));
    m_floatAni->start();
}
