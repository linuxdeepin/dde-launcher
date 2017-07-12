#include "miniframebottombar.h"

#include <DDesktopServices>

DUTIL_USE_NAMESPACE

MiniFrameBottomBar::MiniFrameBottomBar(QWidget *parent)
    : QWidget(parent)
{
    m_computer = new NavigationButton(tr("Computer"));
    m_document = new NavigationButton(tr("Documents"));
    m_video = new NavigationButton(tr("Videos"));
    m_music = new NavigationButton(tr("Music"));
    m_picture = new NavigationButton(tr("Pictures"));
    m_download = new NavigationButton(tr("Downloads"));
    m_shutdownBtn = new DImageButton;
    m_shutdownBtn->setNormalPic(":/icons/skin/icons/poweroff_normal.png");
    m_shutdownBtn->setHoverPic(":/icons/skin/icons/poweroff_hover.png");
    m_shutdownBtn->setPressPic(":/icons/skin/icons/poweroff_press.png");

    QHBoxLayout *navigationLayout = new QHBoxLayout;
    navigationLayout->addStretch();
    navigationLayout->addWidget(m_computer);
    navigationLayout->addWidget(m_document);
    navigationLayout->addWidget(m_video);
    navigationLayout->addWidget(m_music);
    navigationLayout->addWidget(m_picture);
    navigationLayout->addWidget(m_download);
    navigationLayout->addStretch();
    navigationLayout->addWidget(m_shutdownBtn);
    navigationLayout->setMargin(0);

    m_navigationPanel = new QWidget;
    m_navigationPanel->setLayout(navigationLayout);

    m_lock = new NavigationButton(tr("Lock"));
    m_logout = new NavigationButton(tr("Logout"));
    m_reboot = new NavigationButton(tr("Reboot"));
    m_suspend = new NavigationButton(tr("Suspend"));
    m_shutdown = new NavigationButton(tr("Shutdown"));
    m_toNavigation = new NavigationButton(tr("Back"));

    QHBoxLayout *shutdownLayout = new QHBoxLayout;
    shutdownLayout->addStretch();
    shutdownLayout->addWidget(m_shutdown);
    shutdownLayout->addWidget(m_reboot);
    shutdownLayout->addWidget(m_suspend);
    shutdownLayout->addWidget(m_lock);
    shutdownLayout->addWidget(m_logout);
    shutdownLayout->addStretch();
    shutdownLayout->addWidget(m_toNavigation);
    shutdownLayout->setMargin(0);

    m_shutdownPanel = new QWidget;
    m_shutdownPanel->setLayout(shutdownLayout);

    m_panelStack = new QStackedLayout;
    m_panelStack->addWidget(m_navigationPanel);
    m_panelStack->addWidget(m_shutdownPanel);

    setLayout(m_panelStack);

    connect(m_computer, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    connect(m_document, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    connect(m_video, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    connect(m_music, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    connect(m_picture, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    connect(m_download, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });

    connect(m_shutdownBtn, &DImageButton::clicked, this, [this] { m_panelStack->setCurrentWidget(m_shutdownPanel); });
    connect(m_toNavigation, &QPushButton::clicked, this, [this] { m_panelStack->setCurrentWidget(m_navigationPanel); });

    QSignalMapper *signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(m_logout, "Logout");
    signalMapper->setMapping(m_suspend, "Suspend");
    signalMapper->setMapping(m_shutdown, "Shutdown");
    signalMapper->setMapping(m_reboot, "Restart");
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(handleShutdownAction(QString)));

    connect(m_logout, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_suspend, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_shutdown, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_reboot, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(m_lock, SIGNAL(clicked()), this, SLOT(handleLockAction()));
}

void MiniFrameBottomBar::openDirectory(const QString &dir)
{
    DDesktopServices::showFolder(dir);
}

void MiniFrameBottomBar::openStandardDirectory(const QStandardPaths::StandardLocation &location)
{
    const QString dir = QStandardPaths::writableLocation(location);
    if (!dir.isEmpty())
        openDirectory(dir);
}

void MiniFrameBottomBar::handleShutdownAction(const QString &action)
{
    const QString command = QString("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront " \
                                    "/com/deepin/dde/shutdownFront " \
                                    "com.deepin.dde.shutdownFront.%1").arg(action);

    QProcess::startDetached(command);
}

void MiniFrameBottomBar::handleLockAction()
{
    const QString command = QString("dbus-send --print-reply --dest=com.deepin.dde.lockFront " \
                                    "/com/deepin/dde/lockFront " \
                                    "com.deepin.dde.lockFront");

    QProcess::startDetached(command);
}
