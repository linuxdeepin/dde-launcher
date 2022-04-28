#include "modeswitch.h"

#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>
#include <QAbstractButton>

ModeSwitch::ModeSwitch(QWidget *parent)
    : QWidget(parent)
    , m_titleModeButton(new IconButton(this))
    , m_letterModeButton(new IconButton(this))
    , m_buttonGrp(new QButtonGroup(this))
{
    setWindowFlag(Qt::FramelessWindowHint);
    m_titleModeButton->setObjectName("categoryButton");
    m_letterModeButton->setObjectName("letterButton");
    setAccessibleName("modeSwitchWidget");

    m_titleModeButton->setIcon(QIcon::fromTheme("computer"));
    m_letterModeButton->setIcon(QIcon::fromTheme("music"));

    m_titleModeButton->resize(30, 30);
    m_letterModeButton->resize(30, 30);

    m_buttonGrp->addButton(m_titleModeButton);
    m_buttonGrp->addButton(m_letterModeButton);
    m_buttonGrp->setExclusive(true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    hLayout->addWidget(m_titleModeButton);
    hLayout->addWidget(m_letterModeButton);

    setLayout(hLayout);

    connect(m_titleModeButton, &IconButton::clicked, this, &ModeSwitch::titleModeClicked);
    connect(m_letterModeButton, &IconButton::clicked, this, &ModeSwitch::letterModeClicked);
}

ModeSwitch::~ModeSwitch()
{
}

void ModeSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QPen pen;
    pen.setColor(Qt::transparent);
    painter.setPen(pen);
    painter.drawRoundedRect(this->rect(), 10, 10);
}
