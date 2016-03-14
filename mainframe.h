#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "delegate/appitemdelegate.h"
#include "global_util/util.h"
#include "model/appslistmodel.h"
#include "view/applistview.h"
#include "widget/categorytitlewidget.h"
#include "widget/searchwidget.h"
#include "widget/navigationbuttonframe.h"

#include <QFrame>
#include <QScrollArea>

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

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

signals:
    void displayModeChanged(const DisplayMode mode);

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
    void updateDisplayMode(const DisplayMode mode);

private:
    DisplayMode m_displayMode = GroupByCategory;

    NavigationButtonFrame *m_navigationBar;
    SearchWidget *m_searchWidget;
    QScrollArea *m_appsArea;
    DVBoxWidget *m_appsVbox;

    AppItemDelegate *m_appItemDelegate;
    AppListView *m_allAppsView;
    AppListView *m_internetView;
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
    AppsListModel *m_internetModel;
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
