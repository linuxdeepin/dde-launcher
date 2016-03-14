#include "categorybutton.h"
#include "constants.h"

#include <QHBoxLayout>
#include <QDebug>

CategoryButton::CategoryButton(const AppsListModel::AppCategory category, QWidget *parent) :
    QAbstractButton(parent),
    m_category(category),
    m_iconLabel(new QLabel),
    m_textLabel(new QLabel)
{
    QWidget *textWidget = new QWidget;
    m_textLabel->setParent(textWidget);
    m_textLabel->setFixedHeight(DLauncher::NAVIGATION_ICON_HEIGHT);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addWidget(textWidget);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    setLayout(mainLayout);
    setFixedHeight(DLauncher::NAVIGATION_ICON_HEIGHT);
    setTextByCategory();
    setStyleSheet("background-color:cyan;");
}

AppsListModel::AppCategory CategoryButton::category() const
{
    return m_category;
}

void CategoryButton::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
}

void CategoryButton::setTextByCategory()
{
    switch (m_category)
    {
    case AppsListModel::Internet:       m_textLabel->setText(tr("Internet"));       break;
    case AppsListModel::Music:          m_textLabel->setText(tr("Music"));          break;
    case AppsListModel::Video:          m_textLabel->setText(tr("Video"));          break;
    case AppsListModel::Graphics:       m_textLabel->setText(tr("Graphics"));       break;
    case AppsListModel::Office:         m_textLabel->setText(tr("Office"));         break;
    case AppsListModel::Reading:        m_textLabel->setText(tr("Reading"));        break;
    case AppsListModel::Development:    m_textLabel->setText(tr("Development"));    break;
    case AppsListModel::System:         m_textLabel->setText(tr("System"));         break;
//    case AppsListModel::Others:         m_textLabel->setText(tr("Others"));       break;
    default:                            m_textLabel->setText(tr("Others"));         break;
    }
}
