
#include "mainframe.h"
#include "global_util/xcb_misc.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>
#include <QScrollBar>
#include <QKeyEvent>

MainFrame::MainFrame(QWidget *parent) :
    QFrame(parent),
    m_navigationBar(new NavigationButtonFrame),
    m_searchWidget(new SearchWidget),
    m_appsArea(new QScrollArea),
    m_appsVbox(new DVBoxWidget),
    m_appItemDelegate(new AppItemDelegate),

    m_allAppsView(new AppListView),
    m_internetView(new AppListView),
    m_musicView(new AppListView),
    m_videoView(new AppListView),
    m_graphicsView(new AppListView),
    m_gameView(new AppListView),
    m_officeView(new AppListView),
    m_readingView(new AppListView),
    m_developmentView(new AppListView),
    m_systemView(new AppListView),
    m_othersView(new AppListView),

    m_allAppsModel(new AppsListModel(AppsListModel::All)),
    m_internetModel(new AppsListModel(AppsListModel::Internet)),
    m_musicModel(new AppsListModel(AppsListModel::Music)),
    m_videoModel(new AppsListModel(AppsListModel::Video)),
    m_graphicsModel(new AppsListModel(AppsListModel::Graphics)),
    m_gameModel(new AppsListModel(AppsListModel::Game)),
    m_officeModel(new AppsListModel(AppsListModel::Office)),
    m_readingModel(new AppsListModel(AppsListModel::Reading)),
    m_developmentModel(new AppsListModel(AppsListModel::Development)),
    m_systemModel(new AppsListModel(AppsListModel::System)),
    m_othersModel(new AppsListModel(AppsListModel::Others)),

    m_internetTitle(new CategoryTitleWidget(tr("Internet"))),
    m_musicTitle(new CategoryTitleWidget(tr("Music"))),
    m_videoTitle(new CategoryTitleWidget(tr("Video"))),
    m_graphicsTitle(new CategoryTitleWidget(tr("Graphics"))),
    m_gameTitle(new CategoryTitleWidget(tr("Game"))),
    m_officeTitle(new CategoryTitleWidget(tr("Office"))),
    m_readingTitle(new CategoryTitleWidget(tr("Reading"))),
    m_developmentTitle(new CategoryTitleWidget(tr("Development"))),
    m_systemTitle(new CategoryTitleWidget(tr("System"))),
    m_othersTitle(new CategoryTitleWidget(tr("Others")))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_InputMethodEnabled);
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);

    setObjectName("LauncherFrame");

    initUI();

    setFixedSize(qApp->primaryScreen()->geometry().size());
    setStyleSheet(getQssFromFile(":/skin/qss/main.qss"));
    initConnection();

    connect(m_allAppsView, &AppListView::doubleClicked, [this] (const QModelIndex &index) {
        m_allAppsModel->removeRow(index.row());
    });

    updateDisplayMode(AllApps);
    updateDisplayMode(GroupByCategory);
}

void MainFrame::scrollToCategory(const AppsListModel::AppCategory &category)
{
    QWidget *dest = nullptr;

    switch (category)
    {
    case AppsListModel::Internet:   dest = m_internetTitle;     break;
    default:;
    }

    // scroll to destination
    if (dest)
        m_appsArea->verticalScrollBar()->setValue(dest->pos().y());
}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    const int appsContentWidth = m_appsArea->width();

    m_appsVbox->setFixedWidth(appsContentWidth);
    m_allAppsView->setFixedWidth(appsContentWidth);
    m_internetView->setFixedWidth(appsContentWidth);
    m_musicView->setFixedWidth(appsContentWidth);
    m_videoView->setFixedWidth(appsContentWidth);
    m_graphicsView->setFixedWidth(appsContentWidth);
    m_gameView->setFixedWidth(appsContentWidth);
    m_officeView->setFixedWidth(appsContentWidth);
    m_readingView->setFixedWidth(appsContentWidth);
    m_developmentView->setFixedWidth(appsContentWidth);
    m_systemView->setFixedWidth(appsContentWidth);
    m_othersView->setFixedWidth(appsContentWidth);
}

void MainFrame::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
    case Qt::Key_Control:       scrollToCategory(AppsListModel::Internet);      break;
    case Qt::Key_F1:            updateDisplayMode(AllApps);                     break;
    case Qt::Key_F2:            updateDisplayMode(GroupByCategory);             break;
#endif
    default:;
    }
}

void MainFrame::showEvent(QShowEvent *e)
{
    XcbMisc::instance()->set_deepin_override(winId());

    QFrame::showEvent(e);
}

bool MainFrame::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    Q_UNUSED(e);

    return false;
}

void MainFrame::initUI()
{
    m_appsArea->setObjectName("AppBox");
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_appsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_allAppsView->setModel(m_allAppsModel);
    m_allAppsView->setItemDelegate(m_appItemDelegate);
    m_internetView->setModel(m_internetModel);
    m_internetView->setItemDelegate(m_appItemDelegate);
    m_musicView->setModel(m_musicModel);
    m_musicView->setItemDelegate(m_appItemDelegate);
    m_videoView->setModel(m_videoModel);
    m_videoView->setItemDelegate(m_appItemDelegate);
    m_graphicsView->setModel(m_graphicsModel);
    m_graphicsView->setItemDelegate(m_appItemDelegate);
    m_gameView->setModel(m_gameModel);
    m_gameView->setItemDelegate(m_appItemDelegate);
    m_officeView->setModel(m_officeModel);
    m_officeView->setItemDelegate(m_appItemDelegate);
    m_readingView->setModel(m_readingModel);
    m_readingView->setItemDelegate(m_appItemDelegate);
    m_developmentView->setModel(m_developmentModel);
    m_developmentView->setItemDelegate(m_appItemDelegate);
    m_systemView->setModel(m_systemModel);
    m_systemView->setItemDelegate(m_appItemDelegate);
    m_othersView->setModel(m_othersModel);
    m_othersView->setItemDelegate(m_appItemDelegate);

    m_appsVbox->layout()->addWidget(m_allAppsView);
    m_appsVbox->layout()->addWidget(m_internetTitle);
    m_appsVbox->layout()->addWidget(m_internetView);
    m_appsVbox->layout()->addWidget(m_musicTitle);
    m_appsVbox->layout()->addWidget(m_musicView);
    m_appsVbox->layout()->addWidget(m_videoTitle);
    m_appsVbox->layout()->addWidget(m_videoView);
    m_appsVbox->layout()->addWidget(m_graphicsTitle);
    m_appsVbox->layout()->addWidget(m_graphicsView);
    m_appsVbox->layout()->addWidget(m_gameTitle);
    m_appsVbox->layout()->addWidget(m_gameView);
    m_appsVbox->layout()->addWidget(m_officeTitle);
    m_appsVbox->layout()->addWidget(m_officeView);
    m_appsVbox->layout()->addWidget(m_readingTitle);
    m_appsVbox->layout()->addWidget(m_readingView);
    m_appsVbox->layout()->addWidget(m_developmentTitle);
    m_appsVbox->layout()->addWidget(m_developmentView);
    m_appsVbox->layout()->addWidget(m_systemTitle);
    m_appsVbox->layout()->addWidget(m_systemView);
    m_appsVbox->layout()->addWidget(m_othersTitle);
    m_appsVbox->layout()->addWidget(m_othersView);
    m_appsVbox->layout()->setSpacing(0);
    m_appsVbox->layout()->setMargin(0);
    m_appsArea->setWidget(m_appsVbox);

    QVBoxLayout *rightSideLayout = new QVBoxLayout;
    rightSideLayout->addWidget(m_searchWidget);
    rightSideLayout->addWidget(m_appsArea);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_navigationBar);
    mainLayout->addLayout(rightSideLayout);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);
}

void MainFrame::initConnection()
{
    connect(m_allAppsView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_internetView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_musicView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_videoView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_graphicsView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_gameView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_officeView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_readingView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_developmentView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_systemView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_othersView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);

    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_allAppsView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_internetView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_musicView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_videoView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_graphicsView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_gameView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_officeView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_readingView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_developmentView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_systemView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_othersView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
}

void MainFrame::updateDisplayMode(const DisplayMode mode)
{
    if (m_displayMode == mode)
        return;

    m_displayMode = mode;

    bool isCategoryMode = m_displayMode == GroupByCategory;

    m_allAppsView->setVisible(!isCategoryMode);
    m_internetTitle->setVisible(isCategoryMode);
    m_internetView->setVisible(isCategoryMode);
    m_musicTitle->setVisible(isCategoryMode);
    m_musicView->setVisible(isCategoryMode);
    m_videoTitle->setVisible(isCategoryMode);
    m_videoView->setVisible(isCategoryMode);
    m_graphicsTitle->setVisible(isCategoryMode);
    m_graphicsView->setVisible(isCategoryMode);
    m_gameTitle->setVisible(isCategoryMode);
    m_gameView->setVisible(isCategoryMode);
    m_officeTitle->setVisible(isCategoryMode);
    m_officeView->setVisible(isCategoryMode);
    m_readingTitle->setVisible(isCategoryMode);
    m_readingView->setVisible(isCategoryMode);
    m_developmentTitle->setVisible(isCategoryMode);
    m_developmentView->setVisible(isCategoryMode);
    m_systemTitle->setVisible(isCategoryMode);
    m_systemView->setVisible(isCategoryMode);
    m_othersTitle->setVisible(isCategoryMode);
    m_othersView->setVisible(isCategoryMode);

    emit displayModeChanged(m_displayMode);
}
