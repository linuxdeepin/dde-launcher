#ifndef MINIFRAMENAVIGATION_H
#define MINIFRAMENAVIGATION_H

#include <QWidget>
#include <QPushButton>
#include <QStackedLayout>

#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class MiniFrameNavigation : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameNavigation(QWidget *parent = 0);

private slots:
    void openDirectory(const QString &dir);

private:
    DImageButton *m_avatar;

    QPushButton *m_computer;
    QPushButton *m_document;
    QPushButton *m_video;
    QPushButton *m_music;
    QPushButton *m_picture;
    QPushButton *m_download;
    QPushButton *m_toShutdown;

    QPushButton *m_lock;
    QPushButton *m_logout;
    QPushButton *m_reboot;
    QPushButton *m_suspend;
    QPushButton *m_shutdown;
    QPushButton *m_toNavigation;

    QWidget *m_navigationPanel;
    QWidget *m_shutdownPanel;

    QStackedLayout *m_panelStack;
};

#endif // MINIFRAMENAVIGATION_H
