
#include "searchlineedit.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QEvent>
#include <QTimer>
#include <QResizeEvent>

SearchLineEdit::SearchLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    m_icon = new DImageButton;
    m_icon->setFixedSize(16, 16);
    m_icon->setNormalPic(":/skin/images/search.svg");
    m_clear = new DImageButton;
    m_clear->setFixedSize(16, 16);
    m_clear->setNormalPic(":/icons/skin/icons/input_clear_dark_normal.png");
    m_clear->setHoverPic(":/icons/skin/icons/input_clear_dark_hover.png");
    m_clear->setPressPic(":/icons/skin/icons/input_clear_dark_press.png");
    m_clear->setVisible(false);
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

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addStretch();
    centralLayout->addWidget(m_clear);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 5, 0);

    setLayout(centralLayout);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFocusPolicy(Qt::ClickFocus);
    setFixedHeight(30);
    setObjectName("SearchEdit");

    connect(this, &SearchLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);
    connect(m_clear, &DImageButton::clicked, this, &SearchLineEdit::normalMode);

#ifndef ARCH_MIPSEL
    m_floatAni = new QPropertyAnimation(m_floatWidget, "pos", this);
    m_floatAni->setDuration(260);
    m_floatAni->setEasingCurve(QEasingCurve::OutQuad);

    connect(m_floatAni, &QPropertyAnimation::finished, this, static_cast<void (SearchLineEdit::*)()>(&SearchLineEdit::update), Qt::QueuedConnection);
    connect(m_floatAni, &QPropertyAnimation::finished, this, &SearchLineEdit::moveFloatWidget, Qt::QueuedConnection);
#endif
}

bool SearchLineEdit::event(QEvent *e)
{
    switch (e->type())
    {
#ifdef ARCH_MIPSEL
    case QEvent::InputMethodQuery: // for loongson, there's no FocusIn event when the widget gets focus.
#endif
    case QEvent::FocusIn:       editMode();         break;
//    case QEvent::FocusOut:      normalMode();       break;
    default:;
    }

    return QLineEdit::event(e);
}

void SearchLineEdit::resizeEvent(QResizeEvent *e)
{
    QLineEdit::resizeEvent(e);

    QTimer::singleShot(1, this, &SearchLineEdit::moveFloatWidget);
}

void SearchLineEdit::normalMode()
{
    // clear text when back to normal mode
    clear();
    clearFocus();

    m_placeholderText->show();

#ifndef ARCH_MIPSEL
    m_floatAni->setEndValue(rect().center() - m_floatWidget->rect().center());
    if (m_floatAni->state() == QPropertyAnimation::Running)
        return;
    m_floatAni->setStartValue(m_floatWidget->pos());
    m_floatAni->start();
#else
    moveFloatWidget();
#endif
}

void SearchLineEdit::editMode()
{
    m_placeholderText->hide();

#ifndef ARCH_MIPSEL
    m_floatAni->stop();
    m_floatAni->setStartValue(m_floatWidget->pos());
    m_floatAni->setEndValue(QPoint(5, 0));
    m_floatAni->start();
#else
    m_floatWidget->move(QPoint(5, 0));
#endif
}

void SearchLineEdit::onTextChanged()
{
    m_clear->setVisible(!text().isEmpty());
}

void SearchLineEdit::moveFloatWidget()
{
    if (m_floatAni->endValue().toPoint() == QPoint(5, 0))
        return;

    m_floatAni->stop();
    m_floatWidget->move(rect().center() - m_floatWidget->rect().center());
}
