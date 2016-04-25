
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
#include <QGraphicsEffect>

#include <ddialog.h>

const QString DEFAULT_DISPLAY_MODE_KEY = "defaultDisplayMode";

MainFrame::MainFrame(QWidget *parent) :
    QFrame(parent),
    m_settings("deepin", "dde-launcher", this),

    m_displayInter(new DBusDisplay(this)),

    m_calcUtil(CalculateUtil::instance(this)),
    m_appsManager(AppsManager::instance(this)),
    m_delayHideTimer(new QTimer(this)),
    m_backgroundLabel(new SystemBackground(qApp->primaryScreen()->geometry().size(), true, this)),

    m_toggleModeBtn(new DImageButton(this)),
    m_navigationBar(new NavigationWidget),
    m_searchWidget(new SearchWidget(this)),
    m_appsArea(new AppListArea),
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

    m_internetTitle(new CategoryTitleWidget("Internet")),
    m_chatTitle(new CategoryTitleWidget("Chat")),
    m_musicTitle(new CategoryTitleWidget("Music")),
    m_videoTitle(new CategoryTitleWidget("Video")),
    m_graphicsTitle(new CategoryTitleWidget("Graphics")),
    m_gameTitle(new CategoryTitleWidget("Game")),
    m_officeTitle(new CategoryTitleWidget("Office")),
    m_readingTitle(new CategoryTitleWidget("Reading")),
    m_developmentTitle(new CategoryTitleWidget("Development")),
    m_systemTitle(new CategoryTitleWidget("System")),
    m_othersTitle(new CategoryTitleWidget("Others"))
{
    setFocusPolicy(Qt::ClickFocus);
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
    setFixedSize(qApp->primaryScreen()->geometry().size());

    setObjectName("LauncherFrame");

    initUI();
    initConnection();

    updateDisplayMode(DisplayMode(m_settings.value(DEFAULT_DISPLAY_MODE_KEY, AllApps).toInt()));

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

    m_currentCategory = category;

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

    qDebug() << "m_appsArea:" << m_appsArea->size();
    m_calcUtil->calculateAppLayout(m_appsArea->size());
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
    case Qt::Key_Plus:          m_calcUtil->increaseIconSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                break;
    case Qt::Key_Minus:         m_calcUtil->decreaseIconSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                break;
    case Qt::Key_Slash:         m_calcUtil->increaseItemSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                break;
    case Qt::Key_Asterisk:      m_calcUtil->decreaseItemSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                break;
#endif
    case Qt::Key_Enter:
    case Qt::Key_Return:        launchCurrentApp();                             break;
    case Qt::Key_Escape:        hide();                                         break;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:         moveCurrentSelectApp(e->key());                 break;
    default:;
    }
}

void MainFrame::showEvent(QShowEvent *e)
{
    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();
    updateCurrentVisibleCategory();
    // TODO: Do we need this in showEvent ???
    XcbMisc::instance()->set_deepin_override(winId());
    // To make sure the window is placed at the right position.
    updateGeometry();

    QFrame::showEvent(e);

    showGradient();
    raise();
    activateWindow();
    setFocus();
}

void MainFrame::mouseReleaseEvent(QMouseEvent *e)
{
    QFrame::mouseReleaseEvent(e);
    if (e->button() == Qt::RightButton) {
        return;
    }
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
    if (o == m_searchWidget->edit() && e->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyPress = static_cast<QKeyEvent *>(e);
        if (keyPress->key() == Qt::Key_Left || keyPress->key() == Qt::Key_Right)
        {
            QKeyEvent *event = new QKeyEvent(keyPress->type(), keyPress->key(), keyPress->modifiers());

            qApp->postEvent(this, event);
            return true;
        }
    } else if (o == m_navigationBar && e->type() == QEvent::Wheel) {
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
    m_searchWidget->edit()->installEventFilter(this);
//    qApp->installEventFilter(this);

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

    m_internetTitle->setTextVisible(false);
    m_chatTitle->setTextVisible(false);
    m_musicTitle->setTextVisible(false);
    m_videoTitle->setTextVisible(false);
    m_graphicsTitle->setTextVisible(false);
    m_gameTitle->setTextVisible(false);
    m_officeTitle->setTextVisible(false);
    m_readingTitle->setTextVisible(false);
    m_developmentTitle->setTextVisible(false);
    m_systemTitle->setTextVisible(false);
    m_othersTitle->setTextVisible(false);

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
    m_appsVbox->layout()->setContentsMargins(0, DLauncher::APPS_AREA_TOP_BOTTOM_MARGIN,
                                             0, DLauncher::APPS_AREA_TOP_BOTTOM_MARGIN);
    m_appsArea->setWidget(m_appsVbox);

    m_scrollAreaLayout = new QVBoxLayout;
    m_scrollAreaLayout->setMargin(0);
    m_scrollAreaLayout->setSpacing(0);
    m_scrollAreaLayout->addWidget(m_appsArea);
    m_scrollAreaLayout->addSpacing(DLauncher::VIEWLIST_BOTTOM_MARGIN);

    m_contentLayout = new QHBoxLayout;
    m_contentLayout->setMargin(0);
    m_contentLayout->addSpacing(0);
    m_contentLayout->addWidget(m_navigationBar);
    m_contentLayout->addLayout(m_scrollAreaLayout);
    m_contentLayout->addSpacing(int(DLauncher::VIEWLIST_RIGHT_MARGIN*m_calcUtil->viewMarginRation()));

    m_bottomGradient->setDirection(GradientLabel::BottomToTop);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addSpacing(60);
    m_mainLayout->addLayout(m_contentLayout);

    setLayout(m_mainLayout);

    m_searchWidget->move(qApp->desktop()->screenGeometry().width()/2 - m_searchWidget->width()/2, 10);

    m_toggleModeBtn->setFixedSize(22, 22);
    m_toggleModeBtn->setNormalPic(":/icons/skin/icons/category_normal_22px.svg");
    m_toggleModeBtn->setHoverPic(":/icons/skin/icons/category_hover_22px.svg");
    m_toggleModeBtn->setPressPic(":/icons/skin/icons/category_active_22px.svg");
    m_toggleModeBtn->move(QPoint(30, 30));
    // animation
    m_scrollAnimation = new QPropertyAnimation(m_appsArea->verticalScrollBar(), "value");
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

// FIXME:
void MainFrame::showGradient() {
        QPoint topLeft = m_appsArea->mapTo(this,
                                           QPoint(0, 0));
        QSize topSize(m_appsArea->width(), DLauncher::TOP_BOTTOM_GRADIENT_HEIGHT);
        QRect topRect(topLeft, topSize);
        m_topGradient->setPixmap(m_backgroundLabel->getBackground().copy(topRect));
        m_topGradient->resize(topRect.size());

//        qDebug() << "topleft point:" << topRect.topLeft() << topRect.size();
        m_topGradient->move(topRect.topLeft());
        m_topGradient->show();
        m_topGradient->raise();

        QPoint bottomPoint = m_appsArea->mapTo(this,
                                             m_appsArea->rect().bottomLeft());
        QSize bottomSize(m_appsArea->width(), DLauncher::TOP_BOTTOM_GRADIENT_HEIGHT);

        QPoint bottomLeft(bottomPoint.x(), bottomPoint.y() + 1 - bottomSize.height());

        QRect bottomRect(bottomLeft, bottomSize);
        m_bottomGradient->setPixmap(m_backgroundLabel->getBackground().copy(bottomRect));


        m_bottomGradient->resize(bottomRect.size());
        m_bottomGradient->move(bottomRect.topLeft());
        m_bottomGradient->show();
        m_bottomGradient->raise();
}

void MainFrame::showCategoryMoveAnimation()
{
    QWidget *widget = qobject_cast<QWidget *>(sender());
    if (!widget)
        return;

    const bool shownListArea = widget == m_appsArea;

    m_refershCategoryTextVisible = true;

    // NOTE: fake item move animation
    if (shownListArea)
    {
        // from left side to right side
        if (m_internetTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Internet), m_internetTitle->textLabel());
        if (m_chatTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Chat), m_chatTitle->textLabel());
        if (m_musicTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Music), m_musicTitle->textLabel());
        if (m_videoTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Video), m_videoTitle->textLabel());
        if (m_graphicsTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Graphics), m_graphicsTitle->textLabel());
        if (m_gameTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Game), m_gameTitle->textLabel());
        if (m_officeTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Office), m_officeTitle->textLabel());
        if (m_readingTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Reading), m_readingTitle->textLabel());
        if (m_developmentTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Development), m_developmentTitle->textLabel());
        if (m_systemTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::System), m_systemTitle->textLabel());
        if (m_othersTitle->isVisible())
            fakeLabelMoveAni(m_navigationBar->categoryTextLabel(AppsListModel::Others), m_othersTitle->textLabel());
    } else {
        // from right side to left side
        if (m_internetTitle->isVisible())
            fakeLabelMoveAni(m_internetTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Internet));
        if (m_chatTitle->isVisible())
            fakeLabelMoveAni(m_chatTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Chat));
        if (m_musicTitle->isVisible())
            fakeLabelMoveAni(m_musicTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Music));
        if (m_videoTitle->isVisible())
            fakeLabelMoveAni(m_videoTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Video));
        if (m_graphicsTitle->isVisible())
            fakeLabelMoveAni(m_graphicsTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Graphics));
        if (m_gameTitle->isVisible())
            fakeLabelMoveAni(m_gameTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Game));
        if (m_officeTitle->isVisible())
            fakeLabelMoveAni(m_officeTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Office));
        if (m_readingTitle->isVisible())
            fakeLabelMoveAni(m_readingTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Reading));
        if (m_developmentTitle->isVisible())
            fakeLabelMoveAni(m_developmentTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Development));
        if (m_systemTitle->isVisible())
            fakeLabelMoveAni(m_systemTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::System));
        if (m_othersTitle->isVisible())
            fakeLabelMoveAni(m_othersTitle->textLabel(), m_navigationBar->categoryTextLabel(AppsListModel::Others));
    }

    m_refershCategoryTextVisible = false;
}

void MainFrame::fakeLabelMoveAni(QLabel *source, QLabel *dest)
{
    if (!source || !dest)
        return;
    // ignore when animation is running
    if (!source->isVisible() && !dest->isVisible())
        return;
    if (dest->isVisible())
        return;

    // free this label when ani finished
    QLabel *floatLabel = new QLabel(this);
    QPropertyAnimation *ani = new QPropertyAnimation(floatLabel, "geometry", this);

    QPoint sourcePos;
    QWidget *widget = source;
    while (widget)
    {
        sourcePos += widget->pos();
        widget = qobject_cast<QWidget *>(widget->parent());
    }

    QPoint destPos;
    widget = dest;
    while (widget)
    {
        destPos += widget->pos();
        widget = qobject_cast<QWidget *>(widget->parent());
    }

    // disable graphics before grab to pixmap.
    if (dest->graphicsEffect())
        dest->graphicsEffect()->setEnabled(false);
    floatLabel->setPixmap(dest->grab());
    if (dest->graphicsEffect())
        dest->graphicsEffect()->setEnabled(true);
    floatLabel->show();

    ani->setStartValue(QRect(sourcePos, source->size()));
    ani->setEndValue(QRect(destPos, dest->size()));
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(300);

    connect(ani, &QPropertyAnimation::finished, floatLabel, &QLabel::deleteLater);
    // ignore repeat connections
    if (m_refershCategoryTextVisible)
    {
        m_refershCategoryTextVisible = false;
        connect(ani, &QPropertyAnimation::finished, this, &MainFrame::refershCategoryTextVisible);
    }

    source->hide();
    dest->hide();
    ani->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainFrame::refershCategoryTextVisible()
{
    const QPoint pos = QCursor::pos();
    const bool shownAppList = !m_navigationBar->rect().contains(pos);

    m_navigationBar->setCategoryTextVisible(!shownAppList);
    m_internetTitle->setTextVisible(shownAppList);
    m_chatTitle->setTextVisible(shownAppList);
    m_musicTitle->setTextVisible(shownAppList);
    m_videoTitle->setTextVisible(shownAppList);
    m_graphicsTitle->setTextVisible(shownAppList);
    m_gameTitle->setTextVisible(shownAppList);
    m_officeTitle->setTextVisible(shownAppList);
    m_readingTitle->setTextVisible(shownAppList);
    m_developmentTitle->setTextVisible(shownAppList);
    m_systemTitle->setTextVisible(shownAppList);
    m_othersTitle->setTextVisible(shownAppList);
}

void MainFrame::initConnection()
{
    connect(m_displayInter, &DBusDisplay::PrimaryChanged, this, &MainFrame::updateGeometry);
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &MainFrame::updateGeometry);

    connect(m_scrollAnimation, &QPropertyAnimation::valueChanged, this, &MainFrame::ensureScrollToDest);
//    connect(m_scrollAnimation, &QPropertyAnimation::finished, [this] {m_navigationBar->setCurrentCategory(m_currentCategory);});
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

//    connect(m_allAppsView, &AppListView::appBeDraged, m_appsManager, &AppsManager::handleDragedApp);
//    connect(m_allAppsView, &AppListView::appDropedIn, m_appsManager, &AppsManager::handleDropedApp);
//    connect(m_allAppsView, &AppListView::handleDragItems, m_appsManager, &AppsManager::handleDragedApp);

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

    connect(m_appsArea, &AppListArea::mouseEntered, this, &MainFrame::showCategoryMoveAnimation);
    connect(m_navigationBar, &NavigationWidget::mouseEntered, this, &MainFrame::showCategoryMoveAnimation);

    connect(m_menuWorker, &MenuWorker::quitLauncher, this, &MainFrame::hide);
    connect(m_menuWorker, &MenuWorker::unInstallApp, this, &MainFrame::showPopupUninstallDialog);
    connect(m_toggleModeBtn, &DImageButton::clicked, [this]{
        m_searchWidget->clearFocus();
        m_searchWidget->clearSearchContent();
        updateDisplayMode(m_displayMode == GroupByCategory ? AllApps : GroupByCategory);
    });

    connect(m_appsManager, &AppsManager::updateCategoryView, this, &MainFrame::checkCategoryVisible);
}

void MainFrame::updateGeometry()
{
    const QRect rect = m_displayInter->primaryRect();
    setFixedSize(rect.size());
    move(rect.topLeft());

    QFrame::updateGeometry();
}

void MainFrame::moveCurrentSelectApp(const int key)
{
    const QModelIndex currentIndex = m_appItemDelegate->currentIndex();
//    const AppsListModel::AppCategory indexMode = currentIndex.data(AppsListModel::AppCategoryRole).value<AppsListModel::AppCategory>();

    if (!currentIndex.isValid())
    {
        m_appItemDelegate->setCurrentIndex(m_displayMode == GroupByCategory ? m_internetView->indexAt(0) : m_allAppsView->indexAt(0));
        update();
        return;
    }

    const int column = m_calcUtil->appColumnCount();
    QModelIndex index;

    switch (key)
    {
    case Qt::Key_Backtab:
    case Qt::Key_Left:      index = currentIndex.sibling(currentIndex.row() - 1, 0);        break;
    case Qt::Key_Tab:
    case Qt::Key_Right:     index = currentIndex.sibling(currentIndex.row() + 1, 0);        break;
    case Qt::Key_Up:        index = currentIndex.sibling(currentIndex.row() - column, 0);   break;
    case Qt::Key_Down:      index = currentIndex.sibling(currentIndex.row() + column, 0);   break;
    default:;
    }

    if (!index.isValid() && (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right))
    {
        const int realColumn = currentIndex.row() % column;
        const AppsListModel *model = static_cast<const AppsListModel *>(currentIndex.model());
        if (key == Qt::Key_Down || key == Qt::Key_Right)
            model = nextCategoryModel(model);
        else
            model = prevCategoryModel(model);

        if (key == Qt::Key_Up || key == Qt::Key_Down) {
            while (model && model->rowCount(QModelIndex()) <= realColumn)
                if (key == Qt::Key_Down)
                    model = nextCategoryModel(model);
                else
                    model = prevCategoryModel(model);
        } else {
            while (model && model->rowCount(QModelIndex()) <= realColumn && realColumn!=1)
                if (key == Qt::Key_Right)
                    model = nextCategoryModel(model);
                else
                    model = prevCategoryModel(model);
        }
        if (model)
        {
            if (key == Qt::Key_Down)
                index = model->index(realColumn);
            else if (key == Qt::Key_Up){
                const int count = model->rowCount(QModelIndex()) - 1;
                int realIndex = count;
                while (realIndex && realIndex % column != realColumn)
                    --realIndex;
                index = model->index(realIndex);
            } else if (key == Qt::Key_Left) {
                const int count = model->rowCount(QModelIndex()) - 1;
                index = model->index(count);
            } else {
                index = model->index(0);
            }
        }
    }

    const QModelIndex selectedIndex = index.isValid() ? index : currentIndex;
    ensureItemVisible(selectedIndex);
    m_appItemDelegate->setCurrentIndex(selectedIndex);

    update();
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

    DTK_WIDGET_NAMESPACE::DDialog unInstallDialog;
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    const QString appKey = context.data(AppsListModel::AppKeyRole).toString();
    QString appName = context.data(AppsListModel::AppNameRole).toString();
    unInstallDialog.setTitle(QString(tr("Are you sure to uninstall %1 ?")).arg(appName));
    QPixmap appIcon = context.data(AppsListModel::AppIconRole).value<QPixmap>();
    appIcon = appIcon.scaled(64, 64, Qt::IgnoreAspectRatio);
    unInstallDialog.setIconPixmap(appIcon);

    QString message = tr("All dependencies will be removed together");
    unInstallDialog.setMessage(message);
    QStringList buttons;
    buttons << tr("Cancel") << tr("Confirm");
    unInstallDialog.addButtons(buttons);

//    connect(&unInstallDialog, SIGNAL(buttonClicked(int, QString)), this, SLOT(handleUninstallResult(int, QString)));
    connect(&unInstallDialog, &DTK_WIDGET_NAMESPACE::DDialog::buttonClicked, [&] (int clickedResult) {
        // 0 means "cancel" button clicked
        if (clickedResult == 0)
            return;

        m_appsManager->uninstallApp(appKey);
    });

    unInstallDialog.exec();
    unInstallDialog.deleteLater();
    m_isConfirmDialogShown = false;
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

void MainFrame::ensureItemVisible(const QModelIndex &index)
{
    AppListView *view = nullptr;
    const AppsListModel::AppCategory category = index.data(AppsListModel::AppCategoryRole).value<AppsListModel::AppCategory>();

    if (m_displayMode == Search || m_displayMode == AllApps)
        view = m_allAppsView;
    else
        switch (category)
        {
        case AppsListModel::Internet:       view = m_internetView;      break;
        case AppsListModel::Chat:           view = m_chatView;          break;
        case AppsListModel::Music:          view = m_musicView;         break;
        case AppsListModel::Video:          view = m_videoView;         break;
        case AppsListModel::Graphics:       view = m_graphicsView;      break;
        case AppsListModel::Game:           view = m_gameView;          break;
        case AppsListModel::Office:         view = m_officeView;        break;
        case AppsListModel::Reading:        view = m_readingView;       break;
        case AppsListModel::Development:    view = m_developmentView;   break;
        case AppsListModel::System:         view = m_systemView;        break;
        case AppsListModel::Others:         view = m_othersView;        break;
        default:;
        }

    if (!view)
        return;

    m_appsArea->ensureVisible(0, view->indexYOffset(index) + view->pos().y(), 0, DLauncher::APPS_AREA_ENSURE_VISIBLE_MARGIN_Y);
    updateCurrentVisibleCategory();
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
    m_navigationBar->setButtonsVisible(isCategoryMode);

    m_allAppsView->setModel(m_displayMode == Search ? m_searchResultModel : m_allAppsModel);
    // choose nothing
    m_appItemDelegate->setCurrentIndex(QModelIndex());

    if (m_displayMode != Search)
        m_settings.setValue(DEFAULT_DISPLAY_MODE_KEY, m_displayMode);

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

AppsListModel *MainFrame::nextCategoryModel(const AppsListModel *currentModel)
{
    if (currentModel == m_internetModel)
        return m_chatModel;
    if (currentModel == m_chatModel)
        return m_musicModel;
    if (currentModel == m_musicModel)
        return m_videoModel;
    if (currentModel == m_videoModel)
        return m_graphicsModel;
    if (currentModel == m_graphicsModel)
        return m_gameModel;
    if (currentModel == m_gameModel)
        return m_officeModel;
    if (currentModel == m_officeModel)
        return m_readingModel;
    if (currentModel == m_readingModel)
        return m_developmentModel;
    if (currentModel == m_developmentModel)
        return m_systemModel;
    if (currentModel == m_systemModel)
        return m_othersModel;
    if (currentModel == m_othersModel)
        return nullptr;

    return nullptr;
}

AppsListModel *MainFrame::prevCategoryModel(const AppsListModel *currentModel)
{
    if (currentModel == m_internetModel)
        return nullptr;
    if (currentModel == m_chatModel)
        return m_internetModel;
    if (currentModel == m_musicModel)
        return m_chatModel;
    if (currentModel == m_videoModel)
        return m_musicModel;
    if (currentModel == m_graphicsModel)
        return m_videoModel;
    if (currentModel == m_gameModel)
        return m_graphicsModel;
    if (currentModel == m_officeModel)
        return m_gameModel;
    if (currentModel == m_readingModel)
        return m_officeModel;
    if (currentModel == m_developmentModel)
        return m_readingModel;
    if (currentModel == m_systemModel)
        return m_developmentModel;
    if (currentModel == m_othersModel)
        return m_systemModel;

    return nullptr;
}

void MainFrame::searchTextChanged(const QString &keywords)
{
    m_appsManager->searchApp(keywords);

    if (keywords.isEmpty())
        updateDisplayMode(DisplayMode(m_settings.value(DEFAULT_DISPLAY_MODE_KEY, AllApps).toInt()));
    else
        updateDisplayMode(Search);
}
