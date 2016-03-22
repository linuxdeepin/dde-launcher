
#include "mainframe.h"
#include "global_util/constants.h"
#include "global_util/xcb_misc.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>
#include <QScrollBar>
#include <QKeyEvent>

#include <ddialog.h>

const QString DEFAULT_DISPLAY_MODE = "defaultDisplayMode";

MainFrame::MainFrame(QWidget *parent) :
    QFrame(parent),
    m_settings("deepin", "dde-launcher", this),
    m_appsManager(AppsManager::instance(this)),
    m_delayHideTimer(new QTimer(this)),
    m_backgroundLabel(new SystemBackground(qApp->primaryScreen()->geometry().size(), true, this)),

    m_navigationBar(new NavigationWidget),
    m_searchWidget(new SearchWidget),
    m_appsArea(new QScrollArea),
    m_appsVbox(new DVBoxWidget),
    m_menuWorker(new MenuWorker),
    m_viewListPlaceholder(new QWidget),
    m_appItemDelegate(new AppItemDelegate),
    m_topGradient(new GradientLabel(this)),
    m_bottomGradient(new GradientLabel(this)),

    m_allAppsView(new AppListView),
    m_internetView(new AppListView),
    m_chatView(new AppListView),
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
    m_searchResultModel(new AppsListModel(AppsListModel::Search)),
    m_internetModel(new AppsListModel(AppsListModel::Internet)),
    m_chatModel(new AppsListModel(AppsListModel::Chat)),
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
    m_chatTitle(new CategoryTitleWidget(tr("Chat"))),
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
    setFocusPolicy(Qt::StrongFocus);
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
    setFixedSize(qApp->primaryScreen()->geometry().size());
    QRect adjustRect = m_appsManager->getPrimayRect();
    this->move(adjustRect.x(), adjustRect.y());

    setObjectName("LauncherFrame");

    initUI();
    initConnection();

    updateDisplayMode(DisplayMode(m_settings.value(DEFAULT_DISPLAY_MODE, AllApps).toInt()));


    setStyleSheet(getQssFromFile(":/skin/qss/main.qss"));
}

void MainFrame::exit()
{
    qApp->quit();
}

void MainFrame::showByMode(const qlonglong mode)
{
    qDebug() << mode;
}

void MainFrame::scrollToCategory(const AppsListModel::AppCategory &category)
{
    QWidget *dest = nullptr;

    switch (category)
    {
    case AppsListModel::Internet:       dest = m_internetTitle;         break;
    case AppsListModel::Chat:           dest = m_chatTitle;             break;
    case AppsListModel::Music:          dest = m_musicTitle;            break;
    case AppsListModel::Video:          dest = m_videoTitle;            break;
    case AppsListModel::Graphics:       dest = m_graphicsTitle;         break;
    case AppsListModel::Game:           dest = m_gameTitle;             break;
    case AppsListModel::Office:         dest = m_officeTitle;           break;
    case AppsListModel::Reading:        dest = m_readingTitle;          break;
    case AppsListModel::Development:    dest = m_developmentTitle;      break;
    case AppsListModel::System:         dest = m_systemTitle;           break;
    case AppsListModel::Others:         dest = m_othersTitle;           break;
    default:;
    }

    if (!dest)
        return;

    // scroll to destination
//    m_appsArea->verticalScrollBar()->setValue(dest->pos().y());
    m_scrollDest = dest;
    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(m_appsArea->verticalScrollBar()->value());
    m_scrollAnimation->setEndValue(dest->y());
    m_scrollAnimation->start();
}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    m_backgroundLabel->setBackgroundSize(size());

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
    if ((e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A) ||
        (e->key() <= Qt::Key_9 && e->key() >= Qt::Key_0))
    {
        e->accept();

        m_searchWidget->edit()->setFocus(Qt::MouseFocusReason);
        m_searchWidget->edit()->setText(m_searchWidget->edit()->text() + char(e->key() | 0x20));

        return;
    }

    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Control:       scrollToCategory(AppsListModel::Internet);      break;
    case Qt::Key_F1:            updateDisplayMode(AllApps);                     break;
    case Qt::Key_F2:            updateDisplayMode(GroupByCategory);             break;
#endif
    case Qt::Key_Enter:
    case Qt::Key_Return:        launchCurrentApp();                             break;
    case Qt::Key_Escape:        hide();                                         break;
    default:;
    }
}

void MainFrame::showEvent(QShowEvent *e)
{
    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();
    updateCurrentVisibleCategory();
    XcbMisc::instance()->set_deepin_override(winId());

    QFrame::showEvent(e);
    showGradient();
    raise();
    activateWindow();
    setFocus();
}

void MainFrame::mouseReleaseEvent(QMouseEvent *e)
{
    QFrame::mouseReleaseEvent(e);

    hide();
}

void MainFrame::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter painter(this);
    painter.drawPixmap(e->rect(), m_backgroundLabel->getBackground(), e->rect());
}

bool MainFrame::event(QEvent *e)
{
    if (e->type() == QEvent::WindowDeactivate && isVisible() && !m_menuWorker->isMenuShown() && !m_isConfirmDialogShown)
        m_delayHideTimer->start();

    return QFrame::event(e);
}

bool MainFrame::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_navigationBar && e->type() == QEvent::Wheel)
    {
        QWheelEvent *wheel = static_cast<QWheelEvent *>(e);
        QWheelEvent *event = new QWheelEvent(wheel->pos(), wheel->delta(), wheel->buttons(), wheel->modifiers());

        qApp->postEvent(m_appsArea->viewport(), event);
        return true;
    } else if (o == m_appsArea->viewport() && e->type() == QEvent::Wheel)
        updateCurrentVisibleCategory();
    else if (o == m_othersView && e->type() == QEvent::Resize)
        m_viewListPlaceholder->setFixedHeight(m_appsArea->height() - m_othersView->height());

    return false;
}

void MainFrame::initUI()
{
    m_delayHideTimer->setInterval(500);
    m_delayHideTimer->setSingleShot(true);

    m_appsArea->setObjectName("AppBox");
    m_appsArea->setWidgetResizable(true);
    m_appsArea->setFocusPolicy(Qt::NoFocus);
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_appsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->viewport()->installEventFilter(this);

    m_othersView->installEventFilter(this);
    m_navigationBar->installEventFilter(this);

    m_allAppsView->setModel(m_allAppsModel);
    m_allAppsView->setItemDelegate(m_appItemDelegate);
    m_internetView->setModel(m_internetModel);
    m_internetView->setItemDelegate(m_appItemDelegate);
    m_chatView->setModel(m_chatModel);
    m_chatView->setItemDelegate(m_appItemDelegate);
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
    m_appsVbox->layout()->addWidget(m_chatTitle);
    m_appsVbox->layout()->addWidget(m_chatView);
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
    m_appsVbox->layout()->addWidget(m_viewListPlaceholder);
    m_appsVbox->layout()->setSpacing(0);
    m_appsVbox->layout()->setMargin(0);
    m_appsArea->setWidget(m_appsVbox);

    m_contentLayout = new QHBoxLayout;
    m_contentLayout->setMargin(0);
    m_contentLayout->addSpacing(0);
    m_contentLayout->addWidget(m_navigationBar);
    m_contentLayout->addWidget(m_appsArea);
    m_contentLayout->addSpacing(DLauncher::VIEWLIST_RIGHT_MARGIN);

    m_bottomGradient->setDirection(GradientLabel::BottomToTop);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_searchWidget);
    m_mainLayout->addLayout(m_contentLayout);


    setLayout(m_mainLayout);

    // animation
    m_scrollAnimation = new QPropertyAnimation(m_appsArea->verticalScrollBar(), "value");
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

void MainFrame::updateUI() {
    QRect updateRect = m_appsManager->getPrimayRect();
    this->move(updateRect.x(), updateRect.y());
}

void MainFrame::showGradient() {
        QPoint topLeft = m_appsArea->mapTo(this,
                                           QPoint(0, 0));
        QSize topSize(m_appsArea->width(), DLauncher::TOP_BOTTOM_GRADIENT_HEIGHT);
        QRect topRect(topLeft, topSize);
        m_topGradient->setPixmap(m_backgroundLabel->getBackground().copy(topRect));
        m_topGradient->resize(topRect.size());
        qDebug() << "topleft point:" << topRect.topLeft() << topRect.size();
        m_topGradient->move(topRect.topLeft());
        m_topGradient->show();
        m_topGradient->raise();

        QPoint bottomPoint = m_appsArea->mapTo(this,
                                             m_appsArea->rect().bottomLeft());
        QSize bottomSize(m_appsArea->width(), DLauncher::TOP_BOTTOM_GRADIENT_HEIGHT);

        QPoint bottomLeft(bottomPoint.x(), bottomPoint.y() + 1 - bottomSize.height());

        QRect bottomRect(bottomLeft, bottomSize);
        m_bottomGradient->setPixmap(m_backgroundLabel->getBackground().copy(bottomRect));
        qDebug() << "m_backgroundLabel->getBackground().copy(bottomRect):" <<
                    m_backgroundLabel->getBackground().copy(bottomRect).size();
        qDebug() << "bottomleft point:" << bottomRect.topLeft() << bottomRect.size();
        m_bottomGradient->resize(bottomRect.size());
        m_bottomGradient->move(bottomRect.topLeft());
        m_bottomGradient->show();
        m_bottomGradient->raise();
}

void MainFrame::initConnection()
{
    connect(m_scrollAnimation, &QPropertyAnimation::valueChanged, this, &MainFrame::ensureScrollToDest);
    connect(m_navigationBar, &NavigationWidget::scrollToCategory, this, &MainFrame::scrollToCategory);
    connect(this, &MainFrame::currentVisibleCategoryChanged, m_navigationBar, &NavigationWidget::setCurrentCategory);
    connect(this, &MainFrame::categoryAppNumsChanged, m_navigationBar, &NavigationWidget::refershCategoryVisible);
    connect(this, &MainFrame::categoryAppNumsChanged, this, &MainFrame::refershCategoryVisible);
    connect(this, &MainFrame::displayModeChanged, this, &MainFrame::checkCategoryVisible);
    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &MainFrame::searchTextChanged);
    connect(m_delayHideTimer, &QTimer::timeout, this, &MainFrame::hide);
    connect(m_backgroundLabel, &SystemBackground::backgroundChanged, this, static_cast<void (MainFrame::*)()>(&MainFrame::update));

    connect(m_allAppsView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_internetView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_chatView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_musicView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_videoView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_graphicsView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_gameView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_officeView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_readingView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_developmentView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_systemView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);
    connect(m_othersView, &AppListView::popupMenuRequested, this, &MainFrame::showPopupMenu);

    connect(m_allAppsView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_internetView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_chatView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_musicView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_videoView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_graphicsView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_gameView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_officeView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_readingView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_developmentView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_systemView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_othersView, &AppListView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);

    connect(m_allAppsView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_internetView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_chatView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_musicView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_videoView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_graphicsView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_gameView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_officeView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_readingView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_developmentView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_systemView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_othersView, &AppListView::clicked, m_appsManager, &AppsManager::launchApp);

    connect(m_allAppsView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_internetView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_chatView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_musicView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_videoView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_graphicsView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_gameView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_officeView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_readingView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_developmentView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_systemView, &AppListView::clicked, this, &MainFrame::hide);
    connect(m_othersView, &AppListView::clicked, this, &MainFrame::hide);

    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_allAppsView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_internetView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_chatView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_musicView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_videoView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_graphicsView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_gameView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_officeView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_readingView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_developmentView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_systemView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_othersView, static_cast<void (AppListView::*)(const QModelIndex&)>(&AppListView::update));

    connect(m_menuWorker, &MenuWorker::quitLauncher, this, &MainFrame::hide);
    connect(m_menuWorker, &MenuWorker::unInstallApp, this, &MainFrame::showPopupUninstallDialog);
    connect(m_navigationBar, &NavigationWidget::toggleModeClicked, [this] {
        if (m_displayMode == Search)
            return;
        updateDisplayMode(m_displayMode == GroupByCategory ? AllApps : GroupByCategory);
    });
    connect(m_appsManager, &AppsManager::primaryChanged, this, &MainFrame::updateUI);
}

void MainFrame::launchCurrentApp()
{
    const QModelIndex &index = m_appItemDelegate->currentIndex();

    if (index.isValid() && !index.data(AppsListModel::AppDesktopRole).toString().isEmpty())
    {
        const AppsListModel::AppCategory category = index.data(AppsListModel::AppGroupRole).value<AppsListModel::AppCategory>();

        if ((category == AppsListModel::All && m_displayMode == AllApps) ||
            (category == AppsListModel::Search && m_displayMode == Search) ||
            (m_displayMode == GroupByCategory && category != AppsListModel::All && category != AppsListModel::Search))
        {
            m_appsManager->launchApp(index);

            hide();
            return;
        }
    }

    switch (m_displayMode)
    {
    case Search:
    case AllApps:           m_appsManager->launchApp(m_allAppsView->indexAt(0));     break;
    case GroupByCategory:   m_appsManager->launchApp(m_internetView->indexAt(0));    break;
    }

    hide();
}

void MainFrame::checkCategoryVisible()
{
    if (m_displayMode != GroupByCategory)
        return;

    emit categoryAppNumsChanged(AppsListModel::Internet, m_appsManager->appNums(AppsListModel::Internet));
    emit categoryAppNumsChanged(AppsListModel::Chat, m_appsManager->appNums(AppsListModel::Chat));
    emit categoryAppNumsChanged(AppsListModel::Music, m_appsManager->appNums(AppsListModel::Music));
    emit categoryAppNumsChanged(AppsListModel::Video, m_appsManager->appNums(AppsListModel::Video));
    emit categoryAppNumsChanged(AppsListModel::Graphics, m_appsManager->appNums(AppsListModel::Graphics));
    emit categoryAppNumsChanged(AppsListModel::Game, m_appsManager->appNums(AppsListModel::Game));
    emit categoryAppNumsChanged(AppsListModel::Office, m_appsManager->appNums(AppsListModel::Office));
    emit categoryAppNumsChanged(AppsListModel::Reading, m_appsManager->appNums(AppsListModel::Reading));
    emit categoryAppNumsChanged(AppsListModel::Development, m_appsManager->appNums(AppsListModel::Development));
    emit categoryAppNumsChanged(AppsListModel::System, m_appsManager->appNums(AppsListModel::System));
    emit categoryAppNumsChanged(AppsListModel::Others, m_appsManager->appNums(AppsListModel::Others));
}

void MainFrame::showPopupMenu(const QPoint &pos, const QModelIndex &context)
{
    qDebug() << "show menu" << pos << context << context.data(AppsListModel::AppNameRole).toString()
             << "app key:" << context.data(AppsListModel::AppKeyRole).toString();

    m_menuWorker->showMenuByAppItem(context, pos);
}

void MainFrame::showPopupUninstallDialog(const QModelIndex &context)
{
    m_isConfirmDialogShown = true;

    DTK_WIDGET_NAMESPACE::DDialog unInstallDialog(this);
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    QString appName = context.data(AppsListModel::AppNameRole).toString();
    unInstallDialog.setTitle(QString(tr("Are you sure to unInstall %1")).arg(appName));
    QPixmap appIcon = context.data(AppsListModel::AppIconRole).value<QPixmap>();
    appIcon = appIcon.scaled(64, 64, Qt::IgnoreAspectRatio);
    unInstallDialog.setIconPixmap(appIcon);

    QString message = tr("All dependencies will be removed together");
    unInstallDialog.setMessage(message);
    QStringList buttons;
    buttons << tr("cancel") << tr("confirm");
    unInstallDialog.addButtons(buttons);

    connect(&unInstallDialog, SIGNAL(buttonClicked(int, QString)), this, SLOT(handleUninstallResult(int, QString)));

    unInstallDialog.exec();
    unInstallDialog.deleteLater();
    m_isConfirmDialogShown = false;
}

void MainFrame::handleUninstallResult(int result, QString content) {
    Q_UNUSED(content);
    const QModelIndex unInstallIndex = m_menuWorker->getCurrentModelIndex();
    qDebug() << "unInstallAppName:" << unInstallIndex.data(AppsListModel::AppNameRole).toString()
             << result;
    emit m_appsManager->handleUninstallApp(unInstallIndex, result);
}

void MainFrame::ensureScrollToDest(const QVariant &value)
{
    Q_UNUSED(value);

    if (sender() != m_scrollAnimation)
        return;

    QPropertyAnimation *ani = qobject_cast<QPropertyAnimation *>(sender());

    if (m_scrollDest->y() != ani->endValue())
        ani->setEndValue(m_scrollDest->y());
}

void MainFrame::refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums)
{
    if (m_displayMode != GroupByCategory)
        return;

    QWidget *categoryTitle = nullptr;
    QWidget *categoryView = nullptr;

    switch (category) {
    case AppsListModel::Internet:       categoryTitle = m_internetTitle;
                                        categoryView = m_internetView;          break;
    case AppsListModel::Chat:           categoryTitle = m_chatTitle;
                                        categoryView = m_chatView;              break;
    case AppsListModel::Music:          categoryTitle = m_musicTitle;
                                        categoryView = m_musicView;             break;
    case AppsListModel::Video:          categoryTitle = m_videoTitle;
                                        categoryView = m_videoView;             break;
    case AppsListModel::Graphics:       categoryTitle = m_graphicsTitle;
                                        categoryView = m_graphicsView;          break;
    case AppsListModel::Game:           categoryTitle = m_gameTitle;
                                        categoryView = m_gameView;              break;
    case AppsListModel::Office:         categoryTitle = m_officeTitle;
                                        categoryView = m_officeView;            break;
    case AppsListModel::Reading:        categoryTitle = m_readingTitle;
                                        categoryView = m_readingView;           break;
    case AppsListModel::Development:    categoryTitle = m_developmentTitle;
                                        categoryView = m_developmentView;       break;
    case AppsListModel::System:         categoryTitle = m_systemTitle;
                                        categoryView = m_systemView;            break;
    case AppsListModel::Others:         categoryTitle = m_othersTitle;
                                        categoryView = m_othersView;            break;
    default:;
    }

    if (categoryTitle)
        categoryTitle->setVisible(appNums);
    if (categoryView)
        categoryView->setVisible(appNums);
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
    m_chatTitle->setVisible(isCategoryMode);
    m_chatView->setVisible(isCategoryMode);
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

    m_viewListPlaceholder->setVisible(isCategoryMode);

    m_navigationBar->setButtonsVisible(m_displayMode == GroupByCategory);

    m_allAppsView->setModel(m_displayMode == Search ? m_searchResultModel : m_allAppsModel);

    if (m_displayMode != Search)
        m_settings.setValue(DEFAULT_DISPLAY_MODE, m_displayMode);

    if (m_displayMode == GroupByCategory)
        scrollToCategory(m_currentCategory);
    else
        // scroll to top on group mode
        m_appsArea->verticalScrollBar()->setValue(0);

    emit displayModeChanged(m_displayMode);
}

void MainFrame::updateCurrentVisibleCategory()
{
    if (m_displayMode != GroupByCategory)
        return;

    AppsListModel::AppCategory currentVisibleCategory;

    if (!m_internetView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Internet;
    else if (!m_chatView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Chat;
    else if (!m_musicView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Music;
    else if (!m_videoView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Video;
    else if (!m_graphicsView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Graphics;
    else if (!m_gameView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Game;
    else if (!m_officeView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Office;
    else if (!m_readingView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Reading;
    else if (!m_developmentView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Development;
    else if (!m_systemView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::System;
    else
        currentVisibleCategory = AppsListModel::Others;

    if (m_currentCategory == currentVisibleCategory)
        return;

    m_currentCategory = currentVisibleCategory;
    emit currentVisibleCategoryChanged(m_currentCategory);
}

void MainFrame::searchTextChanged(const QString &keywords)
{
    m_appsManager->searchApp(keywords);

    if (keywords.isEmpty())
        updateDisplayMode(DisplayMode(m_settings.value(DEFAULT_DISPLAY_MODE, AllApps).toInt()));
    else
        updateDisplayMode(Search);
}
