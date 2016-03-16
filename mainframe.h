#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "delegate/appitemdelegate.h"
#include "global_util/util.h"
#include "model/appslistmodel.h"
#include "view/applistview.h"
#include "categorytitlewidget.h"
#include "searchwidget.h"
#include "navigationwidget.h"
#include "worker/menuworker.h"
#include "model/appsmanager.h"
#include "confirmuninstalldialog.h"

#include <QFrame>
#include <QScrollArea>
#include <QPropertyAnimation>

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class DBusLauncherService;
class MainFrame : public QFrame
{
    Q_OBJECT

    enum DisplayMode {
        AllApps,
        GroupByCategory,
        Search,
    };

public:
    explicit MainFrame(QWidget *parent = 0);

    void exit();
    void showByMode(const qlonglong mode);

signals:
    void categoryAppNumsChanged(const AppsListModel::AppCategory category, const int appNums);
    void displayModeChanged(const DisplayMode mode);
    void currentVisibleCategoryChanged(const AppsListModel::AppCategory currentVisibleCategory) const;

public slots:
    void scrollToCategory(const AppsListModel::AppCategory &category);

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private:
    void initUI();
    void initConnection();
    void initTimer();
    void checkCategoryVisible();
    void showPopupMenu(const QPoint &pos, const QModelIndex &context);
    void showPopupUninstallDialog(const QModelIndex &context);
    void updateDisplayMode(const DisplayMode mode);
    void updateCurrentVisibleCategory();

private slots:
    void searchTextChanged(const QString &keywords);
    void ensureScrollToDest(const QVariant &value);
    void refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums);
    void handleUninstallResult(int result);

private:
    DisplayMode m_displayMode = AllApps;
    AppsListModel::AppCategory m_currentCategory = AppsListModel::All;

    AppsManager *m_appsManager;
    QPropertyAnimation *m_scrollAnimation;
    QWidget *m_scrollDest;

    NavigationWidget *m_navigationBar;
    SearchWidget *m_searchWidget;
    QScrollArea *m_appsArea;
    DVBoxWidget *m_appsVbox;
    MenuWorker* m_menuWorker;

    QWidget *m_viewListPlaceholder;

    AppItemDelegate *m_appItemDelegate;
    AppListView *m_allAppsView;
    AppListView *m_internetView;
    AppListView * m_chatView;
    AppListView *m_musicView;
    AppListView *m_videoView;
    AppListView *m_graphicsView;
    AppListView *m_gameView;
    AppListView *m_officeView;
    AppListView *m_readingView;
    AppListView *m_developmentView;
    AppListView *m_systemView;
    AppListView *m_othersView;
    AppsListModel *m_allAppsModel;
    AppsListModel *m_searchResultModel;
    AppsListModel *m_internetModel;
    AppsListModel *m_chatModel;
    AppsListModel *m_musicModel;
    AppsListModel *m_videoModel;
    AppsListModel *m_graphicsModel;
    AppsListModel *m_gameModel;
    AppsListModel *m_officeModel;
    AppsListModel *m_readingModel;
    AppsListModel *m_developmentModel;
    AppsListModel *m_systemModel;
    AppsListModel *m_othersModel;
    CategoryTitleWidget *m_internetTitle;
    CategoryTitleWidget* m_chatTitle;
    CategoryTitleWidget *m_musicTitle;
    CategoryTitleWidget *m_videoTitle;
    CategoryTitleWidget *m_graphicsTitle;
    CategoryTitleWidget *m_gameTitle;
    CategoryTitleWidget *m_officeTitle;
    CategoryTitleWidget *m_readingTitle;
    CategoryTitleWidget *m_developmentTitle;
    CategoryTitleWidget *m_systemTitle;
    CategoryTitleWidget *m_othersTitle;
};

#endif // MAINFRAME_H
