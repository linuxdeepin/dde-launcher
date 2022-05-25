#include "editlabel.h"

#include <QDebug>
#include <QKeyEvent>
#include <QHBoxLayout>

EditLabel::EditLabel(QWidget *parent)
    : QWidget(parent)
    , m_lineEdit(new QLineEdit(this))
    , m_label(new QLabel(this))
{
    initUi();
    initConnection();
    initAccessibleName();
}

EditLabel::~EditLabel()
{
}

void EditLabel::initUi()
{
    this->setFixedSize(150, 60);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    hBoxLayout->setSpacing(0);
    hBoxLayout->addWidget(m_label);
    hBoxLayout->addWidget(m_lineEdit);
    setLayout(hBoxLayout);

    QColor labelColor(Qt::white);
    QPalette palette(m_label->palette());
    palette.setColor(QPalette::WindowText, labelColor);
    m_label->setPalette(palette);

    QColor editColor(Qt::transparent);
    QPalette editPalette(m_lineEdit->palette());
    editPalette.setColor(QPalette::Base, editColor);
    m_lineEdit->setPalette(editPalette);

    m_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_lineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_lineEdit->setFocusPolicy(Qt::ClickFocus);
    m_lineEdit->hide();
    m_label->setFixedSize(150, 35);
    m_lineEdit->setFixedSize(150, 35);

    m_label->setAlignment(Qt::AlignCenter);
}

void EditLabel::initConnection()
{
    connect(m_lineEdit, &QLineEdit::returnPressed, this, &EditLabel::onReturnPressed);
}

void EditLabel::initAccessibleName()
{
    m_label->setAccessibleName("drawerTitle");
    m_lineEdit->setAccessibleName("drawerLineEdit");
}

void EditLabel::cancelEditState()
{
    m_lineEdit->clearFocus();
    m_lineEdit->hide();
    m_label->show();
}

void EditLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    m_oldTitle = m_label->text();
    m_label->hide();
    m_lineEdit->show();
    m_lineEdit->setGeometry(m_label->geometry());
    m_lineEdit->setText(m_label->text());
    m_lineEdit->setFocus();
}

void EditLabel::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        cancelEditState();
}

void EditLabel::onReturnPressed()
{
    if (m_lineEdit->text().isEmpty()) {
        m_label->setText(m_oldTitle);
        cancelEditState();
        return;
    }

    m_label->setText(m_lineEdit->text());
    cancelEditState();

    emit titleChanged();
}

void EditLabel::setText(const QString &str)
{
    m_label->setText(str);
}

QString EditLabel::text() const
{
    return m_label->text();
}
