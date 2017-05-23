#include "miniframenavigation.h"

#include <QVBoxLayout>
#include <QProcess>

#include <DDesktopServices>

DUTIL_USE_NAMESPACE

NavigationButton::NavigationButton(const QString &title, QWidget *parent) :
    QPushButton(title, parent)
{

}


MiniFrameNavigation::MiniFrameNavigation(QWidget *parent)
    : QWidget(parent)
{
    m_avatar = new DImageButton;
    m_avatar->setFixedSize(60, 60);
    m_avatar->setNormalPic("/var/lib/AccountsService/icons/7.png");

    m_computer = new NavigationButton(tr("Computer"));
    m_document = new NavigationButton(tr("Documents"));
    m_video = new NavigationButton(tr("Videos"));
    m_music = new NavigationButton(tr("Music"));
    m_picture = new NavigationButton(tr("Pictures"));
    m_download = new NavigationButton(tr("Downloads"));
    m_toShutdown = new NavigationButton(tr("Shutdown"));

    QVBoxLayout *navigationLayout = new QVBoxLayout;
    navigationLayout->addWidget(m_computer);
    navigationLayout->addWidget(m_document);
    navigationLayout->addWidget(m_video);
    navigationLayout->addWidget(m_music);
    navigationLayout->addWidget(m_picture);
    navigationLayout->addWidget(m_download);
    navigationLayout->addStretch();
    navigationLayout->addWidget(m_toShutdown);

    m_navigationPanel = new QWidget;
    m_navigationPanel->setLayout(navigationLayout);

    m_lock = new NavigationButton(tr("Lock"));
    m_logout = new NavigationButton(tr("Logout"));
    m_reboot = new NavigationButton(tr("Reboot"));
    m_suspend = new NavigationButton(tr("Suspend"));
    m_shutdown = new NavigationButton(tr("Shutdown"));
    m_toNavigation = new NavigationButton(tr("Back"));

    QVBoxLayout *shutdownLayout = new QVBoxLayout;
    shutdownLayout->addWidget(m_lock);
    shutdownLayout->addWidget(m_logout);
    shutdownLayout->addWidget(m_reboot);
    shutdownLayout->addWidget(m_suspend);
    shutdownLayout->addWidget(m_shutdown);
    shutdownLayout->addStretch();
    shutdownLayout->addWidget(m_toNavigation);

    m_shutdownPanel = new QWidget;
    m_shutdownPanel->setLayout(shutdownLayout);

    m_panelStack = new QStackedLayout;
    m_panelStack->addWidget(m_navigationPanel);
    m_panelStack->addWidget(m_shutdownPanel);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_avatar);
    centralLayout->setAlignment(m_avatar, Qt::AlignHCenter);
    centralLayout->addLayout(m_panelStack);
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);

    connect(m_computer, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    connect(m_document, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    connect(m_video, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    connect(m_music, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    connect(m_picture, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    connect(m_download, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });

    connect(m_toShutdown, &QPushButton::clicked, this, [this] { m_panelStack->setCurrentWidget(m_shutdownPanel); });
    connect(m_toNavigation, &QPushButton::clicked, this, [this] { m_panelStack->setCurrentWidget(m_navigationPanel); });
}

void MiniFrameNavigation::openDirectory(const QString &dir)
{
    DDesktopServices::showFolder(dir);
}

void MiniFrameNavigation::openStandardDirectory(const QStandardPaths::StandardLocation &location)
{
    const QString dir = QStandardPaths::writableLocation(location);
    if (!dir.isEmpty())
        openDirectory(dir);
}
