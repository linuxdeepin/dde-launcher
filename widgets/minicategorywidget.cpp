#include "minicategorywidget.h"

#include <QVBoxLayout>
#include <QButtonGroup>

MiniCategoryItem::MiniCategoryItem(const QString &title, QWidget *parent) :
    QPushButton(title, parent)
{
    setCheckable(true);
}

MiniCategoryWidget::MiniCategoryWidget(QWidget *parent)
    : QWidget(parent),
      m_buttonGroup(new QButtonGroup(this))
{
    m_allApps = new MiniCategoryItem(tr("All Apps"));
    m_internet = new MiniCategoryItem(tr("Internet"));
    m_chat = new MiniCategoryItem(tr("Chat"));
    m_music = new MiniCategoryItem(tr("Music"));
    m_video = new MiniCategoryItem(tr("Video"));
    m_graphics = new MiniCategoryItem(tr("Graphics"));
    m_game = new MiniCategoryItem(tr("Game"));
    m_office = new MiniCategoryItem(tr("Office"));
    m_reading = new MiniCategoryItem(tr("Reading"));
    m_development = new MiniCategoryItem(tr("Development"));
    m_system = new MiniCategoryItem(tr("System"));

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

    QVBoxLayout *centralLayout = new QVBoxLayout;

    centralLayout->addWidget(m_allApps);
    centralLayout->addWidget(m_internet);
    centralLayout->addWidget(m_chat);
    centralLayout->addWidget(m_music);
    centralLayout->addWidget(m_video);
    centralLayout->addWidget(m_graphics);
    centralLayout->addWidget(m_game);
    centralLayout->addWidget(m_office);
    centralLayout->addWidget(m_reading);
    centralLayout->addWidget(m_development);
    centralLayout->addWidget(m_system);
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);

    connect(m_allApps, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::All); });
    connect(m_internet, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Internet); });
    connect(m_chat, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Chat); });
    connect(m_music, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Music); });
    connect(m_video, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Video); });
    connect(m_graphics, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Graphics); });
    connect(m_game, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Game); });
    connect(m_office, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Office); });
    connect(m_reading, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Reading); });
    connect(m_development, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Development); });
    connect(m_system, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::System); });
}
