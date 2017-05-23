#ifndef MINIFRAMENAVIGATION_H
#define MINIFRAMENAVIGATION_H

#include <QWidget>
#include <QPushButton>
#include <QStackedLayout>
#include <QStandardPaths>

#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class NavigationButton : public QPushButton
{
    Q_OBJECT

public:
    explicit NavigationButton(const QString &title, QWidget *parent = Q_NULLPTR);
};

class MiniFrameNavigation : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameNavigation(QWidget *parent = 0);

private slots:
    void openDirectory(const QString &dir);
    void openStandardDirectory(const QStandardPaths::StandardLocation &location);
    void handleShutdownAction(const QString &action);
    void handleLockAction();

private:
    DImageButton *m_avatar;

    NavigationButton *m_computer;
    NavigationButton *m_document;
    NavigationButton *m_video;
    NavigationButton *m_music;
    NavigationButton *m_picture;
    NavigationButton *m_download;
    NavigationButton *m_toShutdown;

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

#endif // MINIFRAMENAVIGATION_H
