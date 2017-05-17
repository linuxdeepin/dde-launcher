#include "minicategorywidget.h"

#include <QVBoxLayout>

MiniCategoryWidget::MiniCategoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_allApps = new QPushButton(tr("All Apps"));
    m_internet = new QPushButton(tr("Internet"));
    m_chat = new QPushButton(tr("Chat"));
    m_music = new QPushButton(tr("Music"));
    m_video = new QPushButton(tr("Video"));
    m_graphics = new QPushButton(tr("Graphics"));
    m_game = new QPushButton(tr("Game"));
    m_office = new QPushButton(tr("Office"));
    m_reading = new QPushButton(tr("Reading"));
    m_development = new QPushButton(tr("Development"));
    m_system = new QPushButton(tr("System"));

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
