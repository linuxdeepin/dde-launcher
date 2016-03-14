
#include "constants.h"
#include "navigationwidget.h"

#include <QVBoxLayout>
#include <QDebug>

NavigationWidget::NavigationWidget(QWidget *parent) :
    QWidget(parent),
    m_internetBtn(new CategoryButton(AppsListModel::Internet, this)),
    m_musicBtn(new CategoryButton(AppsListModel::Music, this)),
    m_videoBtn(new CategoryButton(AppsListModel::Video, this)),
    m_graphicsBtn(new CategoryButton(AppsListModel::Graphics, this)),
    m_officeBtn(new CategoryButton(AppsListModel::Office, this)),
    m_readingBtn(new CategoryButton(AppsListModel::Reading, this)),
    m_developmentBtn(new CategoryButton(AppsListModel::Development, this)),
    m_systemBtn(new CategoryButton(AppsListModel::System, this)),
    m_othersBtn(new CategoryButton(AppsListModel::Others, this))

{
    setFixedWidth(DLauncher::NAVIGATION_WIDGET_WIDTH);

    initUI();
    initConnection();
}

void NavigationWidget::setCurrentCategory(const AppsListModel::AppCategory category)
{
    qDebug() << "set to " << category;
}

void NavigationWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_internetBtn);
    mainLayout->addWidget(m_musicBtn);
    mainLayout->addWidget(m_videoBtn);
    mainLayout->addWidget(m_graphicsBtn);
    mainLayout->addWidget(m_officeBtn);
    mainLayout->addWidget(m_readingBtn);
    mainLayout->addWidget(m_developmentBtn);
    mainLayout->addWidget(m_systemBtn);
    mainLayout->addWidget(m_othersBtn);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    setLayout(mainLayout);
}

void NavigationWidget::initConnection()
{
    connect(m_internetBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_musicBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_videoBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_graphicsBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_officeBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_readingBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_developmentBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_systemBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_othersBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
}

void NavigationWidget::buttonClicked()
{
    CategoryButton *btn = qobject_cast<CategoryButton *>(sender());
    if (!btn)
        return;

    emit scrollToCategory(btn->category());
}
