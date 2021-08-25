/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "minicategorywidget.h"
#include "appsmanager.h"

#include <QVBoxLayout>
#include <QButtonGroup>
#include <QMouseEvent>
#include <QDebug>

MiniCategoryItem::MiniCategoryItem(const QString &title, QWidget *parent) :
    QPushButton(title, parent)
{
    setCheckable(true);
}

MiniCategoryWidget::MiniCategoryWidget(QWidget *parent)
    : QWidget(parent),
      m_active(false),
      m_currentCategory(AppsListModel::All),

      m_appsManager(AppsManager::instance()),
      m_buttonGroup(new QButtonGroup(this))
{
    m_allApps = new MiniCategoryItem(tr("All Apps"));
    m_internet = new MiniCategoryItem(tr("Internet"));
    m_chat = new MiniCategoryItem(tr("Chat"));
    m_music = new MiniCategoryItem(tr("Music"));
    m_video = new MiniCategoryItem(tr("Video"));
    m_graphics = new MiniCategoryItem(tr("Graphics"));
    m_game = new MiniCategoryItem(tr("Games"));
    m_office = new MiniCategoryItem(tr("Office"));
    m_reading = new MiniCategoryItem(tr("Reading"));
    m_development = new MiniCategoryItem(tr("Development"));
    m_system = new MiniCategoryItem(tr("System"));
    m_others = new MiniCategoryItem(tr("Other"));

    m_items << m_allApps << m_internet << m_chat << m_music << m_video;
    m_items << m_graphics << m_game << m_office << m_reading << m_development;
    m_items << m_system << m_others;

    m_buttonGroup->addButton(m_allApps);
    m_buttonGroup->addButton(m_internet);
    m_buttonGroup->addButton(m_chat);
    m_buttonGroup->addButton(m_music);
    m_buttonGroup->addButton(m_video);
    m_buttonGroup->addButton(m_graphics);
    m_buttonGroup->addButton(m_game);
    m_buttonGroup->addButton(m_office);
    m_buttonGroup->addButton(m_reading);
    m_buttonGroup->addButton(m_development);
    m_buttonGroup->addButton(m_system);
    m_buttonGroup->addButton(m_others);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_allApps);
    buttonLayout->addWidget(m_internet);
    buttonLayout->addWidget(m_chat);
    buttonLayout->addWidget(m_music);
    buttonLayout->addWidget(m_video);
    buttonLayout->addWidget(m_graphics);
    buttonLayout->addWidget(m_game);
    buttonLayout->addWidget(m_office);
    buttonLayout->addWidget(m_reading);
    buttonLayout->addWidget(m_development);
    buttonLayout->addWidget(m_system);
    buttonLayout->addWidget(m_others);
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addStretch();
    centralLayout->addLayout(buttonLayout);
    centralLayout->addStretch();
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);
    setObjectName("MiniCategoryWidget");
    setFocusPolicy(Qt::StrongFocus);

    m_allApps->setChecked(true);

    connect(m_appsManager, &AppsManager::categoryListChanged, this, &MiniCategoryWidget::onCategoryListChanged, Qt::QueuedConnection);
    connect(m_allApps, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::All; emit requestCategory(AppsListModel::All); });
    connect(m_internet, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Internet; emit requestCategory(AppsListModel::Internet); });
    connect(m_chat, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Chat; emit requestCategory(AppsListModel::Chat); });
    connect(m_music, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Music; emit requestCategory(AppsListModel::Music); });
    connect(m_video, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Video; emit requestCategory(AppsListModel::Video); });
    connect(m_graphics, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Graphics; emit requestCategory(AppsListModel::Graphics); });
    connect(m_game, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Game; emit requestCategory(AppsListModel::Game); });
    connect(m_office, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Office; emit requestCategory(AppsListModel::Office); });
    connect(m_reading, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Reading; emit requestCategory(AppsListModel::Reading); });
    connect(m_development, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Development; emit requestCategory(AppsListModel::Development); });
    connect(m_system, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::System; emit requestCategory(AppsListModel::System); });
    connect(m_others, &QPushButton::clicked, this, [this] { m_currentCategory = AppsListModel::Others; emit requestCategory(AppsListModel::Others); });

    QTimer::singleShot(1, this, &MiniCategoryWidget::onCategoryListChanged);
}

void MiniCategoryWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Down:      selectNext();           break;
    case Qt::Key_Up:        selectPrev();           break;
    case Qt::Key_Right:     emit requestRight();    break;
    default:;
    }
}

void MiniCategoryWidget::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);

    setFocus();
}

bool MiniCategoryWidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::FocusIn:
        m_active = true;
        emit activeChanged(m_active);
        break;
    case QEvent::FocusOut:
        m_active = false;
        emit activeChanged(m_active);
        break;
    default:;
    }

    return QWidget::event(event);
}

void MiniCategoryWidget::selectNext()
{
    int select = 0;
    for (int i(0); i != m_items.size(); ++i)
    {
        if (m_items[i]->isChecked())
        {
            select = i + 1;
            break;
        }
    }

    while (!m_items[select % m_items.size()]->isVisible())
        ++select;

    m_items[select % m_items.size()]->click();
}

void MiniCategoryWidget::selectPrev()
{
    int select = 0;
    for (int i(0); i != m_items.size(); ++i)
    {
        if (m_items[i]->isChecked())
        {
            select = i - 1;
            break;
        }
    }

    while (!m_items[(select + m_items.size()) % m_items.size()]->isVisible())
        --select;

    m_items[(select + m_items.size()) % m_items.size()]->click();
}

void MiniCategoryWidget::onCategoryListChanged()
{
    m_internet->setVisible(m_appsManager->appNums(AppsListModel::Internet));
    m_chat->setVisible(m_appsManager->appNums(AppsListModel::Chat));
    m_music->setVisible(m_appsManager->appNums(AppsListModel::Music));
    m_video->setVisible(m_appsManager->appNums(AppsListModel::Video));
    m_graphics->setVisible(m_appsManager->appNums(AppsListModel::Graphics));
    m_game->setVisible(m_appsManager->appNums(AppsListModel::Game));
    m_office->setVisible(m_appsManager->appNums(AppsListModel::Office));
    m_reading->setVisible(m_appsManager->appNums(AppsListModel::Reading));
    m_development->setVisible(m_appsManager->appNums(AppsListModel::Development));
    m_others->setVisible(m_appsManager->appNums(AppsListModel::System));
    m_others->setVisible(m_appsManager->appNums(AppsListModel::Others));
}
