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
class MiniCategoryWidget;
class MiniFrame : public DBlurEffectWidget, public LauncherInterface
{
    Q_OBJECT

public:
    explicit MiniFrame(QWidget *parent = 0);
    ~MiniFrame();

private:
    void showLauncher() Q_DECL_OVERRIDE;
    void hideLauncher() Q_DECL_OVERRIDE;
    bool visible() Q_DECL_OVERRIDE;

    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;

    bool event(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void adjustPosition();
    void toggleAppsView();
    void toggleFullScreen();
    void onToggleViewClicked();
    void searchText(const QString &text);

private:
    DBusDock *m_dockInter;
    AppsManager *m_appsManager;

    DImageButton *m_viewToggle;
    DImageButton *m_modeToggle;
    SearchLineEdit *m_searchEdit;

    MiniFrameNavigation *m_navigation;
    MiniCategoryWidget *m_categoryWidget;
    QWidget *m_viewWrapper;
    QScrollArea *m_appsArea;
    DVBoxWidget *m_appsBox;

    QListView *m_appsView;
    AppsListModel *m_appsModel;
    AppsListModel *m_searchModel;
};

#endif // MINIFRAME_H
