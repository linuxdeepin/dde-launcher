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
}
