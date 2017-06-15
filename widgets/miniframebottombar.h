#ifndef MINIFRAMEBOTTOMBAR_H
#define MINIFRAMEBOTTOMBAR_H

#include "miniframenavigation.h"

#include <QWidget>
#include <dimagebutton.h>

class MiniFrameBottomBar : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameBottomBar(QWidget *parent = 0);

private slots:
    void openDirectory(const QString &dir);
    void openStandardDirectory(const QStandardPaths::StandardLocation &location);
    void handleShutdownAction(const QString &action);
    void handleLockAction();

private:
    NavigationButton *m_computer;
    NavigationButton *m_document;
    NavigationButton *m_video;
    NavigationButton *m_music;
    NavigationButton *m_picture;
    NavigationButton *m_download;
    DImageButton *m_shutdownBtn;

    NavigationButton *m_lock;
    NavigationButton *m_logout;
    NavigationButton *m_reboot;
    NavigationButton *m_suspend;
    NavigationButton *m_shutdown;
    NavigationButton *m_toNavigation;

    QWidget *m_navigationPanel;
    QWidget *m_shutdownPanel;

    QStackedLayout *m_panelStack;
};

#endif // MINIFRAMEBOTTOMBAR_H
