// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "navigationwidget.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QDebug>

/**
 * @brief NavigationWidget::NavigationWidget 全屏模式下应用分类按钮组控件
 * @param parent
 */
NavigationWidget::NavigationWidget(QWidget *parent) :
    QFrame(parent),
    m_calcUtil(CalculateUtil::instance()),
    m_categoryGroup(new QButtonGroup(this)),
    m_internetBtn(new CategoryButton(AppsListModel::Internet, this)),
    m_chatBtn(new CategoryButton(AppsListModel::Chat, this)),
    m_musicBtn(new CategoryButton(AppsListModel::Music, this)),
    m_videoBtn(new CategoryButton(AppsListModel::Video, this)),
    m_graphicsBtn(new CategoryButton(AppsListModel::Graphics, this)),
    m_gameBtn(new CategoryButton(AppsListModel::Game, this)),
    m_officeBtn(new CategoryButton(AppsListModel::Office, this)),
    m_readingBtn(new CategoryButton(AppsListModel::Reading, this)),
    m_developmentBtn(new CategoryButton(AppsListModel::Development, this)),
    m_systemBtn(new CategoryButton(AppsListModel::System, this)),
    m_othersBtn(new CategoryButton(AppsListModel::Others, this))
{
    initUI();
    initConnection();
}

void NavigationWidget::setCurrentCategory(const AppsListModel::AppCategory category)
{
    m_currentCategory = category;
    m_calcUtil->setCurrentCategory(category);
    CategoryButton *btn = button(category);

    if (btn)
        btn->setChecked(true);
}

void NavigationWidget::setCancelCurrentCategory(const AppsListModel::AppCategory category)
{
    CategoryButton *btn = button(category);

    if (btn)
        btn->setChecked(false);
}

void NavigationWidget::refershCategoryVisible(const AppsListModel::AppCategory category, const bool visible)
{
    QAbstractButton *btn = button(category);

    if (btn)
        btn->setVisible(visible);

    update();
}

void NavigationWidget::initUI()
{
    m_categoryGroup->addButton(m_internetBtn);
    m_categoryGroup->addButton(m_chatBtn);
    m_categoryGroup->addButton(m_musicBtn);
    m_categoryGroup->addButton(m_videoBtn);
    m_categoryGroup->addButton(m_graphicsBtn);
    m_categoryGroup->addButton(m_gameBtn);
    m_categoryGroup->addButton(m_officeBtn);
    m_categoryGroup->addButton(m_readingBtn);
    m_categoryGroup->addButton(m_developmentBtn);
    m_categoryGroup->addButton(m_systemBtn);
    m_categoryGroup->addButton(m_othersBtn);

    m_internetBtn->setChecked(true);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_internetBtn);
    mainLayout->addWidget(m_chatBtn);
    mainLayout->addWidget(m_musicBtn);
    mainLayout->addWidget(m_videoBtn);
    mainLayout->addWidget(m_graphicsBtn);
    mainLayout->addWidget(m_gameBtn);
    mainLayout->addWidget(m_officeBtn);
    mainLayout->addWidget(m_readingBtn);
    mainLayout->addWidget(m_developmentBtn);
    mainLayout->addWidget(m_systemBtn);
    mainLayout->addWidget(m_othersBtn);
    mainLayout->addStretch();
    mainLayout->setSpacing(30);
    mainLayout->setMargin(14);

    setLayout(mainLayout);
}

void NavigationWidget::initConnection()
{
    connect(m_internetBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_musicBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_chatBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_videoBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_graphicsBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
    connect(m_gameBtn, &CategoryButton::clicked, this, &NavigationWidget::buttonClicked);
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

    emit scrollToCategory(m_currentCategory, btn->category());
}

CategoryButton *NavigationWidget::button(const AppsListModel::AppCategory category) const
{
    switch (category) {
    case AppsListModel::Internet:       return m_internetBtn;
    case AppsListModel::Chat:           return m_chatBtn;
    case AppsListModel::Music:          return m_musicBtn;
    case AppsListModel::Video:          return m_videoBtn;
    case AppsListModel::Graphics:       return m_graphicsBtn;
    case AppsListModel::Game:           return m_gameBtn;
    case AppsListModel::Office:         return m_officeBtn;
    case AppsListModel::Reading:        return m_readingBtn;
    case AppsListModel::Development:    return m_developmentBtn;
    case AppsListModel::System:         return m_systemBtn;
    case AppsListModel::Others:         return m_othersBtn;
    default:;
    }

    return nullptr;
}

void NavigationWidget::updateSize()
{
    double scaleX = m_calcUtil->getScreenScaleX();
    double scaleY = m_calcUtil->getScreenScaleY();
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    setFixedHeight(m_calcUtil->instance()->navigationHeight() * scale);
    int btnSize = scale * DLauncher::NAVIGATION_BUTTON_SIZE;
    for (auto btn : m_categoryGroup->buttons()) {
        btn->setFixedSize(btnSize, btnSize);
        static_cast<CategoryButton*>(btn)->setScreenScale(scale);
    }
}

void NavigationWidget::enterEvent(QEvent *e)
{
    QFrame::enterEvent(e);

    emit mouseEntered();
}

void NavigationWidget::leaveEvent(QEvent *e)
{
    QFrame::leaveEvent(e);

}

void NavigationWidget::showEvent(QShowEvent *e)
{
    QFrame::showEvent(e);

    QTimer::singleShot(0, this, &NavigationWidget::raise);
}
