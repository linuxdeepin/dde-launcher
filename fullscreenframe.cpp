
#include "fullscreenframe.h"
#include "global_util/constants.h"
#include "global_util/xcb_misc.h"
#include "backgroundmanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>
#include <QScrollBar>
#include <QKeyEvent>
#include <QGraphicsEffect>
#include <QProcess>

#include <ddialog.h>

static const QString WallpaperKey = "pictureUri";
static const QString DisplayModeKey = "display-mode";
static const QString DisplayModeFree = "free";
static const QString DisplayModeCategory = "category";

FullScreenFrame::FullScreenFrame(QWidget *parent) :
    BoxFrame(parent),
    m_launcherGsettings(new QGSettings("com.deepin.dde.launcher",
                                       "/com/deepin/dde/launcher/", this)),
    m_backgroundManager(new BackgroundManager(this)),
    m_displayInter(new DBusDisplay(this)),

    m_calcUtil(CalculateUtil::instance()),
    m_appsManager(AppsManager::instance()),
    m_delayHideTimer(new QTimer(this)),
    m_autoScrollTimer(new QTimer(this)),
    m_navigationWidget(new NavigationWidget),
    m_rightSpacing(new QWidget),
    m_searchWidget(new SearchWidget(this)),
    m_appsArea(new AppListArea),
    m_appsVbox(new DVBoxWidget),
    m_menuWorker(new MenuWorker),
    m_viewListPlaceholder(new QWidget),
    m_tipsLabel(new QLabel(this)),
    m_appItemDelegate(new AppItemDelegate),
    m_topGradient(new GradientLabel(this)),
    m_bottomGradient(new GradientLabel(this)),

    m_allAppsView(new AppGridView),
    m_internetView(new AppGridView),
    m_chatView(new AppGridView),
    m_musicView(new AppGridView),
    m_videoView(new AppGridView),
    m_graphicsView(new AppGridView),
    m_gameView(new AppGridView),
    m_officeView(new AppGridView),
    m_readingView(new AppGridView),
    m_developmentView(new AppGridView),
    m_systemView(new AppGridView),
    m_othersView(new AppGridView),

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


    m_floatTitle(new CategoryTitleWidget("Internet", this)),
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

    setObjectName("LauncherFrame");

    initUI();
    initConnection();

    updateDisplayMode(getDisplayMode());

    setStyleSheet(getQssFromFile(":/skin/qss/main.qss"));
}

FullScreenFrame::~FullScreenFrame()
{

}

void FullScreenFrame::exit()
{
    qApp->quit();
}

void FullScreenFrame::uninstallApp(const QString &appKey)
{
    showPopupUninstallDialog(m_allAppsModel->indexAt(appKey));
}

void FullScreenFrame::showByMode(const qlonglong mode)
{
    qDebug() << mode;
}

int FullScreenFrame::dockPosition()
{
    return m_appsManager->dockPosition();
}

void FullScreenFrame::scrollToCategory(const AppsListModel::AppCategory &category)
{
    QWidget *dest = categoryView(category);

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

void FullScreenFrame::showTips(const QString &tips)
{
    if (m_displayMode != Search)
        return;

    m_tipsLabel->setText(tips);

    const QPoint center = rect().center() - m_tipsLabel->rect().center();
    m_tipsLabel->move(center);
    m_tipsLabel->setVisible(true);
    m_tipsLabel->raise();
}

void FullScreenFrame::hideTips()
{
    m_tipsLabel->setVisible(false);
}

void FullScreenFrame::resizeEvent(QResizeEvent *e)
{
    const int screenWidth = e->size().width();

    // reset widgets size
    const int besidePadding = m_calcUtil->calculateBesidePadding(screenWidth);
    m_navigationWidget->setFixedWidth(besidePadding);
    m_rightSpacing->setFixedWidth(besidePadding);

    QFrame::resizeEvent(e);
}

void FullScreenFrame::keyPressEvent(QKeyEvent *e)
{
    bool ctrlPressed = e->modifiers() & Qt::ControlModifier;
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Control:       scrollToCategory(AppsListModel::Internet);      return;
    case Qt::Key_F2:            updateDisplayMode(GroupByCategory);             return;
    case Qt::Key_Plus:          m_calcUtil->increaseIconSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                return;
    case Qt::Key_Minus:         m_calcUtil->decreaseIconSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                return;
    case Qt::Key_Slash:         m_calcUtil->increaseItemSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                return;
    case Qt::Key_Asterisk:      m_calcUtil->decreaseItemSize();
                                emit m_appsManager->layoutChanged(AppsListModel::All);
                                                                                return;
#endif
    case Qt::Key_F1: QProcess::startDetached("dman dde");                       return;
    case Qt::Key_Enter:
    case Qt::Key_Return:        launchCurrentApp();                             return;
    case Qt::Key_Escape:        hide();                                         return;
    case Qt::Key_Tab:
                                e->accept();
    case Qt::Key_Backtab:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:         moveCurrentSelectApp(e->key());                 return;
    }

    // handle normal keys
    if ((e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A) ||
        (e->key() <= Qt::Key_9 && e->key() >= Qt::Key_0) ||
        (e->key() == Qt::Key_Space))
    {
        e->accept();
        // handle the emacs key bindings
        if(ctrlPressed) {
            switch (e->key()) {
            case Qt::Key_P:
                moveCurrentSelectApp(Qt::Key_Up);
                return;
            case Qt::Key_N:
                moveCurrentSelectApp(Qt::Key_Down);
                return;
            case Qt::Key_F:
                moveCurrentSelectApp(Qt::Key_Right);
                return;
            case Qt::Key_B:
                moveCurrentSelectApp(Qt::Key_Left);
            default:
                return;
            }
        }


        m_searchWidget->edit()->setFocus(Qt::MouseFocusReason);
        m_searchWidget->edit()->setText(m_searchWidget->edit()->text() + char(e->key() | 0x20));
        return;
    }
}

void FullScreenFrame::showEvent(QShowEvent *e)
{
    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();
    updateCurrentVisibleCategory();
    // TODO: Do we need this in showEvent ???
    XcbMisc::instance()->set_deepin_override(winId());
    // To make sure the window is placed at the right position.
    updateGeometry();

    QFrame::showEvent(e);

    QTimer::singleShot(0, this, [this] () {
        showGradient();
        raise();
        activateWindow();
        m_floatTitle->raise();
    });
}

void FullScreenFrame::mouseReleaseEvent(QMouseEvent *e)
{
    QFrame::mouseReleaseEvent(e);
    if (e->button() == Qt::RightButton) {
        return;
    }
    hide();
}

void FullScreenFrame::wheelEvent(QWheelEvent *e)
{
    auto shouldPostWheelEvent = [this, e]() -> bool {
        bool inAppArea = m_appsArea->geometry().contains(e->pos());
        bool topMost = m_appsArea->verticalScrollBar()->value() == m_appsArea->verticalScrollBar()->minimum();
        bool bottomMost = m_appsArea->verticalScrollBar()->value() == m_appsArea->verticalScrollBar()->maximum();
        bool exceedingLimits = (e->delta() > 0 && topMost) || (e->delta() < 0 && bottomMost);

        return !inAppArea && !exceedingLimits;
    };

    if (shouldPostWheelEvent()) {
        QWheelEvent *event = new QWheelEvent(e->pos(), e->delta(), e->buttons(), e->modifiers());
        qApp->postEvent(m_appsArea->viewport(), event);

        e->accept();
    }
}

void FullScreenFrame::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter painter(this);
    painter.drawPixmap(e->rect(), getBackground(), e->rect());
    //    painter.setBrush(QColor(255, 0, 0, 0.2 * 255));
    //    painter.drawRect(rect());
}

bool FullScreenFrame::event(QEvent *e)
{
    if (e->type() == QEvent::WindowDeactivate && isVisible() && !m_menuWorker->isMenuShown() && !m_isConfirmDialogShown)
        m_delayHideTimer->start();

    return QFrame::event(e);
}

bool FullScreenFrame::eventFilter(QObject *o, QEvent *e)
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
    }
    else if (o == m_appsArea->viewport() && e->type() == QEvent::Wheel)
    {
        updateCurrentVisibleCategory();
        QMetaObject::invokeMethod(this, "refershCurrentFloatTitle", Qt::QueuedConnection);
    }
    else if (o == m_appsArea->viewport() && e->type() == QEvent::Resize)
    {
        const int pos = m_appsManager->dockPosition();
        m_calcUtil->calculateAppLayout(static_cast<QResizeEvent *>(e)->size(), pos);
        updatePlaceholderSize();
    }

    return false;
}

void FullScreenFrame::initUI()
{
    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(200, 50);
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setStyleSheet("color:rgba(238, 238, 238, .6);"
//                               "background-color:red;"
                               "font-size:22px;");

    m_delayHideTimer->setInterval(500);
    m_delayHideTimer->setSingleShot(true);

    m_autoScrollTimer->setInterval(DLauncher::APPS_AREA_AUTO_SCROLL_TIMER);
    m_autoScrollTimer->setSingleShot(false);

    m_appsArea->setObjectName("AppBox");
    m_appsArea->setWidgetResizable(true);
    m_appsArea->setFocusPolicy(Qt::NoFocus);
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_appsArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->viewport()->installEventFilter(this);

//    m_othersView->installEventFilter(this);
//    m_navigationWidget->installEventFilter(this);
    m_searchWidget->edit()->installEventFilter(this);
//    qApp->installEventFilter(this);

    m_allAppsView->setAccessibleName("all");
    m_allAppsView->setModel(m_allAppsModel);
    m_allAppsView->setItemDelegate(m_appItemDelegate);
    m_allAppsView->setContainerBox(m_appsArea);
    m_internetView->setAccessibleName("internet");
    m_internetView->setModel(m_internetModel);
    m_internetView->setItemDelegate(m_appItemDelegate);
    m_chatView->setAccessibleName("chat");
    m_chatView->setModel(m_chatModel);
    m_chatView->setItemDelegate(m_appItemDelegate);
    m_musicView->setAccessibleName("music");
    m_musicView->setModel(m_musicModel);
    m_musicView->setItemDelegate(m_appItemDelegate);
    m_videoView->setAccessibleName("video");
    m_videoView->setModel(m_videoModel);
    m_videoView->setItemDelegate(m_appItemDelegate);
    m_graphicsView->setAccessibleName("graphics");
    m_graphicsView->setModel(m_graphicsModel);
    m_graphicsView->setItemDelegate(m_appItemDelegate);
    m_gameView->setAccessibleName("game");
    m_gameView->setModel(m_gameModel);
    m_gameView->setItemDelegate(m_appItemDelegate);
    m_officeView->setAccessibleName("office");
    m_officeView->setModel(m_officeModel);
    m_officeView->setItemDelegate(m_appItemDelegate);
    m_readingView->setAccessibleName("reading");
    m_readingView->setModel(m_readingModel);
    m_readingView->setItemDelegate(m_appItemDelegate);
    m_developmentView->setAccessibleName("development");
    m_developmentView->setModel(m_developmentModel);
    m_developmentView->setItemDelegate(m_appItemDelegate);
    m_systemView->setAccessibleName("system");
    m_systemView->setModel(m_systemModel);
    m_systemView->setItemDelegate(m_appItemDelegate);
    m_othersView->setAccessibleName("others");
    m_othersView->setModel(m_othersModel);
    m_othersView->setItemDelegate(m_appItemDelegate);

    m_floatTitle->setVisible(false);
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
    m_appsVbox->layout()->setContentsMargins(0, DLauncher::APPS_AREA_TOP_MARGIN,
                                             0, DLauncher::APPS_AREA_BOTTOM_MARGIN);
    m_appsArea->setWidget(m_appsVbox);

//    m_viewListPlaceholder->setStyleSheet("background-color:red;");

    m_scrollAreaLayout = new QVBoxLayout;
    m_scrollAreaLayout->setMargin(0);
    m_scrollAreaLayout->setSpacing(0);
    m_scrollAreaLayout->addWidget(m_appsArea);
    m_scrollAreaLayout->addSpacing(DLauncher::VIEWLIST_BOTTOM_MARGIN);

    m_bottomGradient->setDirection(GradientLabel::BottomToTop);

    m_contentLayout = new QVBoxLayout;
    m_contentLayout->setMargin(0);
    m_contentLayout->setSpacing(0);
    m_contentLayout->addSpacing(30);
    m_contentLayout->addWidget(m_searchWidget);
    m_contentLayout->setAlignment(m_searchWidget, Qt::AlignCenter);
    m_contentLayout->addSpacing(30);
    m_contentLayout->addLayout(m_scrollAreaLayout);

    m_miniMode = new DImageButton;
    m_miniMode->setNormalPic(":/icons/skin/icons/unfullscreen_normal.png");

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(m_miniMode);
    rightLayout->setAlignment(m_miniMode, Qt::AlignTop | Qt::AlignRight);

    m_rightSpacing->setLayout(rightLayout);

    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->addSpacing(0);
    m_mainLayout->addWidget(m_navigationWidget);
    m_mainLayout->addLayout(m_contentLayout);
    m_mainLayout->addWidget(m_rightSpacing);

    setLayout(m_mainLayout);

//    m_floatTitle->setStyleSheet(getQssFromFile(":/skin/qss/categorytitlewidget.qss"));;
    // animation
    m_scrollAnimation = new QPropertyAnimation(m_appsArea->verticalScrollBar(), "value");
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);

    // setup background.
    auto updateBackground = [this] (const QString &uri) { setBackground(uri); };

    connect(m_backgroundManager, &BackgroundManager::currentWorkspaceBackgroundChanged, this, updateBackground);
    updateBackground(m_backgroundManager->currentWorkspaceBackground());
}

// FIXME:
void FullScreenFrame::showGradient() {
        QPoint topLeft = m_appsArea->mapTo(this,
                                           QPoint(0, 0));
        QSize topSize(m_appsArea->width(), DLauncher::TOP_BOTTOM_GRADIENT_HEIGHT);
        QRect topRect(topLeft, topSize);
        m_topGradient->setPixmap(getBackground().copy(topRect));
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
        m_bottomGradient->setPixmap(getBackground().copy(bottomRect));

        m_bottomGradient->resize(bottomRect.size());
        m_bottomGradient->move(bottomRect.topLeft());
        m_bottomGradient->show();
        m_bottomGradient->raise();
}

void FullScreenFrame::toMiniMode()
{
    const QStringList args {
        "--print-reply",
        "--dest=com.deepin.dde.daemon.Launcher",
        "/com/deepin/dde/daemon/Launcher",
        "org.freedesktop.DBus.Properties.Set",
        "string:com.deepin.dde.daemon.Launcher",
        "string:DisplayMode",
        "variant:int32:1"
    };

    QProcess::startDetached("dbus-send", args);
}

void FullScreenFrame::refreshTitleVisible()
{
    QWidget *widget = qobject_cast<QWidget *>(sender());
    if (!widget)
        return;

    checkCategoryVisible();

    const bool shownListArea = widget == m_appsArea;
    m_floatTitle->setTextVisible(shownListArea);

    refershCategoryTextVisible();
    refershCurrentFloatTitle();
}

void FullScreenFrame::refershCategoryTextVisible()
{
    const QPoint pos = QCursor::pos() - this->pos();
    const bool shownAppList = m_navigationWidget->geometry().right() < pos.x();

    // NOTE(hualet): don't show/hide category text with animation, it'll conflicts
    // with the zoom animation causing very strange behavior;
    m_navigationWidget->setCategoryTextVisible(!shownAppList/*, true*/);
    m_internetTitle->setTextVisible(shownAppList, true);
    m_chatTitle->setTextVisible(shownAppList, true);
    m_musicTitle->setTextVisible(shownAppList, true);
    m_videoTitle->setTextVisible(shownAppList, true);
    m_graphicsTitle->setTextVisible(shownAppList, true);
    m_gameTitle->setTextVisible(shownAppList, true);
    m_officeTitle->setTextVisible(shownAppList, true);
    m_readingTitle->setTextVisible(shownAppList, true);
    m_developmentTitle->setTextVisible(shownAppList, true);
    m_systemTitle->setTextVisible(shownAppList, true);
    m_othersTitle->setTextVisible(shownAppList, true);
}

void FullScreenFrame::refershCurrentFloatTitle()
{
    if (m_displayMode != GroupByCategory)
        return m_floatTitle->setVisible(false);

    CategoryTitleWidget *sourceTitle = categoryTitle(m_currentCategory);
    if (!sourceTitle)
        return;

    m_floatTitle->setFixedSize(sourceTitle->size());
    m_floatTitle->setText(sourceTitle->textLabel()->text());
    m_floatTitle->setVisible(sourceTitle->visibleRegion().isEmpty() ||
                             sourceTitle->visibleRegion().boundingRect().height() < 20);
}

CategoryTitleWidget *FullScreenFrame::categoryTitle(const AppsListModel::AppCategory category) const
{
    CategoryTitleWidget *dest = nullptr;

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

    return dest;
}

AppGridView *FullScreenFrame::categoryView(const AppsListModel::AppCategory category) const
{
    AppGridView *view = nullptr;

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

    return view;
}

AppGridView *FullScreenFrame::lastVisibleView() const
{
    if (!m_appsManager->appsInfoList(AppsListModel::Others).isEmpty())
        return m_othersView;
    if (!m_appsManager->appsInfoList(AppsListModel::System).isEmpty())
        return m_systemView;
    if (!m_appsManager->appsInfoList(AppsListModel::Development).isEmpty())
        return m_developmentView;
    if (!m_appsManager->appsInfoList(AppsListModel::Reading).isEmpty())
        return m_readingView;
    if (!m_appsManager->appsInfoList(AppsListModel::Office).isEmpty())
        return m_officeView;
    if (!m_appsManager->appsInfoList(AppsListModel::Game).isEmpty())
        return m_gameView;
    if (!m_appsManager->appsInfoList(AppsListModel::Graphics).isEmpty())
        return m_graphicsView;
    if (!m_appsManager->appsInfoList(AppsListModel::Video).isEmpty())
        return m_videoView;
    if (!m_appsManager->appsInfoList(AppsListModel::Music).isEmpty())
        return m_musicView;
    if (!m_appsManager->appsInfoList(AppsListModel::Chat).isEmpty())
        return m_chatView;
    if (!m_appsManager->appsInfoList(AppsListModel::Internet).isEmpty())
        return m_internetView;

    return nullptr;
}

void FullScreenFrame::initConnection()
{
    connect(m_miniMode, &DImageButton::clicked, this, &FullScreenFrame::toMiniMode, Qt::QueuedConnection);

    connect(m_displayInter, &DBusDisplay::PrimaryChanged, this, &FullScreenFrame::updateGeometry);
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &FullScreenFrame::updateGeometry);

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &FullScreenFrame::layoutChanged, Qt::QueuedConnection);

    connect(m_scrollAnimation, &QPropertyAnimation::valueChanged, this, &FullScreenFrame::ensureScrollToDest);
    connect(m_scrollAnimation, &QPropertyAnimation::finished, this, &FullScreenFrame::refershCurrentFloatTitle, Qt::QueuedConnection);
    connect(m_navigationWidget, &NavigationWidget::scrollToCategory, this, &FullScreenFrame::scrollToCategory);
    connect(this, &FullScreenFrame::currentVisibleCategoryChanged, m_navigationWidget, &NavigationWidget::setCurrentCategory);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, m_navigationWidget, &NavigationWidget::refershCategoryVisible);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, this, &FullScreenFrame::refershCategoryVisible);
    connect(this, &FullScreenFrame::displayModeChanged, this, &FullScreenFrame::checkCategoryVisible);
    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &FullScreenFrame::searchTextChanged);
    connect(m_delayHideTimer, &QTimer::timeout, this, &FullScreenFrame::hide);
    connect(this, &FullScreenFrame::backgroundChanged, this, static_cast<void (FullScreenFrame::*)()>(&FullScreenFrame::update));

    // auto scroll when drag to app list box border
    connect(m_allAppsView, &AppGridView::requestScrollStop, m_autoScrollTimer, &QTimer::stop);
    connect(m_autoScrollTimer, &QTimer::timeout, [this] {
        m_appsArea->verticalScrollBar()->setValue(m_appsArea->verticalScrollBar()->value() + m_autoScrollStep);
    });
    connect(m_allAppsView, &AppGridView::requestScrollUp, [this] {
        m_autoScrollStep = -DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
        if (!m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();
    });
    connect(m_allAppsView, &AppGridView::requestScrollDown, [this] {
        m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
        if (!m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();
    });

    connect(m_allAppsView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_internetView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_chatView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_musicView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_videoView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_graphicsView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_gameView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_officeView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_readingView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_developmentView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_systemView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(m_othersView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);

//    connect(m_allAppsView, &AppListView::appBeDraged, m_appsManager, &AppsManager::handleDragedApp);
//    connect(m_allAppsView, &AppListView::appDropedIn, m_appsManager, &AppsManager::handleDropedApp);
//    connect(m_allAppsView, &AppListView::handleDragItems, m_appsManager, &AppsManager::handleDragedApp);

    connect(m_allAppsView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_internetView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_chatView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_musicView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_videoView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_graphicsView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_gameView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_officeView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_readingView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_developmentView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_systemView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(m_othersView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);

    connect(m_allAppsView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_internetView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_chatView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_musicView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_videoView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_graphicsView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_gameView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_officeView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_readingView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_developmentView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_systemView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(m_othersView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);

    connect(m_allAppsView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_internetView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_chatView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_musicView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_videoView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_graphicsView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_gameView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_officeView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_readingView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_developmentView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_systemView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_othersView, &AppGridView::clicked, this, &FullScreenFrame::hide);

    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_allAppsView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_internetView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_chatView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_musicView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_videoView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_graphicsView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_gameView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_officeView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_readingView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_developmentView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_systemView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::currentChanged, m_othersView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));

    connect(m_appsArea, &AppListArea::mouseEntered, this, &FullScreenFrame::refreshTitleVisible);
    connect(m_navigationWidget, &NavigationWidget::mouseEntered, this, &FullScreenFrame::refreshTitleVisible);

    connect(m_menuWorker, &MenuWorker::quitLauncher, this, &FullScreenFrame::hide);
    connect(m_menuWorker, &MenuWorker::unInstallApp, this, &FullScreenFrame::showPopupUninstallDialog);
    connect(m_navigationWidget, &NavigationWidget::toggleMode, [this]{
        m_searchWidget->clearFocus();
        m_searchWidget->clearSearchContent();
        updateDisplayMode(m_displayMode == GroupByCategory ? AllApps : GroupByCategory);
    });

    connect(m_appsManager, &AppsManager::updateCategoryView, this, &FullScreenFrame::checkCategoryVisible);
    connect(m_appsManager, &AppsManager::requestTips, this, &FullScreenFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &FullScreenFrame::hideTips);
    connect(m_appsManager, &AppsManager::dockPositionChanged, this, &FullScreenFrame::updateDockPosition);
}

void FullScreenFrame::showLauncher()
{
    show();
}

void FullScreenFrame::hideLauncher()
{
    hide();
}

bool FullScreenFrame::visible()
{
    return isVisible();
}

void FullScreenFrame::updateGeometry()
{
    const QRect rect = m_displayInter->primaryRect();
    setFixedSize(rect.size());
    move(rect.topLeft());

    qDebug() << "update geometry: " << rect;

    QFrame::updateGeometry();
}

void FullScreenFrame::moveCurrentSelectApp(const int key)
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

    do {
        if (index.isValid())
            break;

        const int realColumn = currentIndex.row() % column;
        const AppsListModel *model = static_cast<const AppsListModel *>(currentIndex.model());
        if (key == Qt::Key_Down || key == Qt::Key_Right || key == Qt::Key_Tab)
            model = nextCategoryModel(model);
        else
            model = prevCategoryModel(model);

        // need to keep column
        if (key == Qt::Key_Up || key == Qt::Key_Down)
            while (model && model->rowCount(QModelIndex()) <= realColumn)
                if (key == Qt::Key_Down)
                    model = nextCategoryModel(model);
                else
                    model = prevCategoryModel(model);
        else
            while (model && !model->rowCount(QModelIndex()))
                if (key == Qt::Key_Right || key == Qt::Key_Tab)
                    model = nextCategoryModel(model);
                else
                    model = prevCategoryModel(model);

        if (!model)
            break;

        const int count = model->rowCount(QModelIndex()) - 1;
        int realIndex = count;

        switch (key)
        {
        case Qt::Key_Down:
            index = model->index(realColumn);
            break;
        case Qt::Key_Up:
            while (realIndex && realIndex % column != realColumn)
                --realIndex;
            index = model->index(realIndex);
            break;
        case Qt::Key_Left:
        case Qt::Key_Backtab:
            index = model->index(count);
            break;
        case Qt::Key_Right:
        case Qt::Key_Tab:
            index = model->index(0);
            break;
        default:;
        }

    } while (false);

    const QModelIndex selectedIndex = index.isValid() ? index : currentIndex;
    m_appItemDelegate->setCurrentIndex(selectedIndex);
    ensureItemVisible(selectedIndex);

    update();
}

void FullScreenFrame::launchCurrentApp()
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

void FullScreenFrame::checkCategoryVisible()
{
    if (m_displayMode != GroupByCategory)
        return m_floatTitle->setVisible(false);

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

void FullScreenFrame::showPopupMenu(const QPoint &pos, const QModelIndex &context)
{
    qDebug() << "show menu" << pos << context << context.data(AppsListModel::AppNameRole).toString()
             << "app key:" << context.data(AppsListModel::AppKeyRole).toString();

    m_menuWorker->showMenuByAppItem(context, pos);
}

void FullScreenFrame::showPopupUninstallDialog(const QModelIndex &context)
{
    m_isConfirmDialogShown = true;

    DTK_WIDGET_NAMESPACE::DDialog unInstallDialog;
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    const QString appKey = context.data(AppsListModel::AppKeyRole).toString();
    QString appName = context.data(AppsListModel::AppNameRole).toString();
    unInstallDialog.setTitle(QString(tr("Are you sure to uninstall %1 ?")).arg(appName));
    QPixmap appIcon = context.data(AppsListModel::AppIconRole).value<QPixmap>();
    appIcon = appIcon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
//    unInstallDialog.deleteLater();
    m_isConfirmDialogShown = false;
}

void FullScreenFrame::ensureScrollToDest(const QVariant &value)
{
    Q_UNUSED(value);

    if (sender() != m_scrollAnimation)
        return;

    QPropertyAnimation *ani = qobject_cast<QPropertyAnimation *>(sender());

    if (m_scrollDest->y() != ani->endValue())
        ani->setEndValue(m_scrollDest->y());
}

void FullScreenFrame::ensureItemVisible(const QModelIndex &index)
{
    AppGridView *view = nullptr;
    const AppsListModel::AppCategory category = index.data(AppsListModel::AppCategoryRole).value<AppsListModel::AppCategory>();

    if (m_displayMode == Search || m_displayMode == AllApps)
        view = m_allAppsView;
    else
        view = categoryView(category);

    if (!view)
        return;

    m_appsArea->ensureVisible(0, view->indexYOffset(index) + view->pos().y(), 0, DLauncher::APPS_AREA_ENSURE_VISIBLE_MARGIN_Y);
    updateCurrentVisibleCategory();
    refershCurrentFloatTitle();
}

void FullScreenFrame::refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums)
{
    if (m_displayMode != GroupByCategory)
        return;

    QWidget *categoryTitle = this->categoryTitle(category);
    QWidget *categoryView = this->categoryView(category);

    if (categoryView)
        categoryView->setVisible(appNums);
    if (categoryTitle)
        categoryTitle->setVisible(appNums);
}

void FullScreenFrame::updateDisplayMode(const DisplayMode mode)
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
    m_navigationWidget->setButtonsVisible(isCategoryMode);

    m_allAppsView->setModel(m_displayMode == Search ? m_searchResultModel : m_allAppsModel);
    // choose nothing
    m_appItemDelegate->setCurrentIndex(QModelIndex());

    switch (m_displayMode) {
    case AllApps:
        m_launcherGsettings->set(DisplayModeKey, DisplayModeFree);
        break;
    case GroupByCategory:
        m_launcherGsettings->set(DisplayModeKey, DisplayModeCategory);
        break;
    default:
        break;
    }

    if (m_displayMode == GroupByCategory)
        scrollToCategory(m_currentCategory);
    else
        // scroll to top on group mode
        m_appsArea->verticalScrollBar()->setValue(0);

    hideTips();

    emit displayModeChanged(m_displayMode);
}

void FullScreenFrame::updateCurrentVisibleCategory()
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
    else if (!m_othersView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Others;
    else
        Q_ASSERT(false);

    if (m_currentCategory == currentVisibleCategory)
        return;

    m_currentCategory = currentVisibleCategory;

    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::updatePlaceholderSize()
{
    const AppGridView *view = lastVisibleView();
    Q_ASSERT(view);

    m_viewListPlaceholder->setFixedHeight(m_appsArea->height() - view->height() - DLauncher::APPS_AREA_BOTTOM_MARGIN);
}

void FullScreenFrame::updateDockPosition()
{
    m_calcUtil->calculateAppLayout(m_appsArea->size(), m_appsManager->dockPosition());
    setStyleSheet(getQssFromFile(":/skin/qss/main.qss"));
}

FullScreenFrame::DisplayMode FullScreenFrame::getDisplayMode()
{
    QString displayMode = m_launcherGsettings->get(DisplayModeKey).toString();

    if (displayMode == DisplayModeCategory) {
        return GroupByCategory;
    }

    return AllApps;
}

AppsListModel *FullScreenFrame::nextCategoryModel(const AppsListModel *currentModel)
{
    if (currentModel == nullptr)
        return m_internetModel;
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

AppsListModel *FullScreenFrame::prevCategoryModel(const AppsListModel *currentModel)
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

void FullScreenFrame::layoutChanged()
{
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

    m_floatTitle->move(m_appsArea->pos().x(), m_appsArea->y() - m_floatTitle->height() + 20);
}

void FullScreenFrame::searchTextChanged(const QString &keywords)
{
    m_appsManager->searchApp(keywords);

    if (keywords.isEmpty())
        updateDisplayMode(getDisplayMode());
    else
        updateDisplayMode(Search);
}
