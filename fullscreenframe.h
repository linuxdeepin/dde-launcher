#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "gradientlabel.h"
#include "searchwidget.h"
#include "navigationwidget.h"
#include "categorytitlewidget.h"
#include "launcherinterface.h"
#include "delegate/appitemdelegate.h"
#include "global_util/util.h"
#include "global_util/calculate_util.h"
#include "global_util/constants.h"
#include "model/appsmanager.h"
#include "model/appslistmodel.h"
#include "view/applistview.h"
#include "worker/menuworker.h"
#include "dbusinterface/dbusdisplay.h"
#include "widgets/applistarea.h"
#include "boxframe/boxframe.h"

#include <QFrame>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTimer>
#include <QGSettings>

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class BackgroundManager;
class DBusLauncherService;
class FullScreenFrame : public BoxFrame, public LauncherInterface
{
    Q_OBJECT
    Q_PROPERTY(int dockPosition READ dockPosition DESIGNABLE true)

public:
    enum DisplayMode {
        AllApps,
        GroupByCategory,
        Search,
    };

public:
    explicit FullScreenFrame(QWidget *parent = 0);

    void exit();
    void uninstallApp(const QString &appKey);
    void showByMode(const qlonglong mode);
    int dockPosition();

signals:
    void categoryAppNumsChanged(const AppsListModel::AppCategory category, const int appNums);
    void displayModeChanged(const DisplayMode mode);
    void currentVisibleCategoryChanged(const AppsListModel::AppCategory currentVisibleCategory) const;

public slots:
    void scrollToCategory(const AppsListModel::AppCategory &category);
    void showTips(const QString &tips);
    void hideTips();

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void paintEvent(QPaintEvent *e);
    bool event(QEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private:
    void initUI();
    void initConnection();
    void initTimer();

    void _destructor();
    void showLauncher();
    void hideLauncher();
    bool visible();

    void updateGeometry();
    void moveCurrentSelectApp(const int key);
    void launchCurrentApp();
    void checkCategoryVisible();
    void showPopupMenu(const QPoint &pos, const QModelIndex &context);
    void showPopupUninstallDialog(const QModelIndex &context);
    void updateDisplayMode(const DisplayMode mode);
    void updateCurrentVisibleCategory();
    void updatePlaceholderSize();
    void updateDockPosition();
    DisplayMode getDisplayMode();

    AppsListModel *nextCategoryModel(const AppsListModel *currentModel);
    AppsListModel *prevCategoryModel(const AppsListModel *currentModel);

private slots:
    void layoutChanged();
    void searchTextChanged(const QString &keywords);
    void ensureScrollToDest(const QVariant &value);
    void ensureItemVisible(const QModelIndex &index);
    void refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums);
    void showGradient();
    void toMiniMode();
    void refreshTitleVisible();
    void refershCategoryTextVisible();
    void refershCurrentFloatTitle();

private:
    CategoryTitleWidget *categoryTitle(const AppsListModel::AppCategory category) const;
    AppListView *categoryView(const AppsListModel::AppCategory category) const;
    AppListView *lastVisibleView() const;

private:
    bool m_isConfirmDialogShown = false;
    bool m_refershCategoryTextVisible = false;
    int m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
    double rightMarginRation = 1;
    DisplayMode m_displayMode = Search;
    AppsListModel::AppCategory m_currentCategory = AppsListModel::All;
    QGSettings *m_launcherGsettings;
    BackgroundManager *m_backgroundManager;

    DBusDisplay *m_displayInter;

    CalculateUtil *m_calcUtil;
    AppsManager *m_appsManager;
    QPropertyAnimation *m_scrollAnimation;
    QWidget *m_scrollDest;
    QTimer *m_delayHideTimer;
    QTimer *m_autoScrollTimer;

    NavigationWidget *m_navigationWidget;
    QWidget *m_rightSpacing;
    DImageButton *m_miniMode;
    SearchWidget *m_searchWidget;
    AppListArea *m_appsArea;
    DVBoxWidget *m_appsVbox;
    MenuWorker* m_menuWorker;

    QWidget *m_viewListPlaceholder;
    QLabel *m_tipsLabel;

    AppItemDelegate *m_appItemDelegate;
    GradientLabel* m_topGradient;
    GradientLabel* m_bottomGradient;

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

    CategoryTitleWidget* m_floatTitle;
    CategoryTitleWidget *m_internetTitle;
    CategoryTitleWidget *m_chatTitle;
    CategoryTitleWidget *m_musicTitle;
    CategoryTitleWidget *m_videoTitle;
    CategoryTitleWidget *m_graphicsTitle;
    CategoryTitleWidget *m_gameTitle;
    CategoryTitleWidget *m_officeTitle;
    CategoryTitleWidget *m_readingTitle;
    CategoryTitleWidget *m_developmentTitle;
    CategoryTitleWidget *m_systemTitle;
    CategoryTitleWidget *m_othersTitle;

    QVBoxLayout *m_scrollAreaLayout;
    QHBoxLayout *m_mainLayout;
    QVBoxLayout *m_contentLayout;
};

Q_DECLARE_METATYPE(FullScreenFrame::DisplayMode)

#endif // MAINFRAME_H
