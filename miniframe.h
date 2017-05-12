#ifndef MINIFRAME_H
#define MINIFRAME_H

#include "launcherinterface.h"

#include <QWidget>
#include <DBlurEffectWidget>
#include <dimagebutton.h>
#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class QScrollArea;
class QListView;
class DBusDock;
class MiniFrameNavigation;
class SearchLineEdit;
class AppsListModel;
class AppsManager;
class MiniFrame : public DBlurEffectWidget, public LauncherInterface
{
    Q_OBJECT

public:
    explicit MiniFrame(QWidget *parent = 0);

private:
    void _destructor();
    void showLauncher();
    void hideLauncher();
    bool visible();

    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);

private slots:
    void adjustPosition();
    void toggleAppsView();
    void toggleFullScreen();
    void searchText(const QString &text);

private:
    DBusDock *m_dockInter;
    AppsManager *m_appsManager;

    DImageButton *m_viewToggle;
    DImageButton *m_modeToggle;
    SearchLineEdit *m_searchEdit;

    MiniFrameNavigation *m_navigation;
    QWidget *m_viewWrapper;
    QScrollArea *m_appsArea;
    DVBoxWidget *m_appsBox;

    QListView *m_appsView;
    AppsListModel *m_appsModel;
    AppsListModel *m_searchModel;
};

#endif // MINIFRAME_H
