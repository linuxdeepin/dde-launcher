#include "miniframe.h"
#include "dbusdock.h"
#include "worker/menuworker.h"
#include "widgets/searchwidget.h"
#include "widgets/minicategorywidget.h"
#include "widgets/miniframebottombar.h"
#include "view/appgridview.h"
#include "view/applistview.h"
#include "model/appslistmodel.h"
#include "delegate/appitemdelegate.h"
#include "delegate/applistdelegate.h"
#include "global_util/util.h"

#include "sharedeventfilter.h"

#include <QRect>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStyle>

#include <ddialog.h>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

#define DOCK_FASHION    0
#define DOCK_EFFICIENT  1

MiniFrame::MiniFrame(QWidget *parent)
    : DBlurEffectWidget(parent),

      m_menuWorker(new MenuWorker),
      m_windowHandle(this, this),
      m_dockInter(new DBusDock(this)),
      m_eventFilter(new SharedEventFilter(this)),
      m_appsManager(AppsManager::instance()),
      m_calcUtil(CalculateUtil::instance()),

      m_delayHideTimer(new QTimer(this)),

      m_categoryWidget(new MiniCategoryWidget),
      m_bottomBar(new MiniFrameBottomBar),

      m_appsView(nullptr),
      m_appsModel(new AppsListModel(AppsListModel::All)),
      m_searchModel(new AppsListModel(AppsListModel::Search))
{
    m_windowHandle.setShadowRadius(60);
    m_windowHandle.setShadowOffset(QPoint(0, 2));
    m_windowHandle.setWindowRadius(5);
    m_windowHandle.setEnableBlurWindow(false);
    m_windowHandle.setTranslucentBackground(true);
    m_windowHandle.setBorderColor(QColor(255, 255, 255, .1 * 255));

    m_bottomBar->setFixedHeight(40);
    m_categoryWidget->setFixedWidth(140);
    m_categoryWidget->setVisible(false);

    m_viewToggle = new DImageButton;
    m_viewToggle->setNormalPic(":/icons/skin/icons/category_normal_22px.svg");
    m_viewToggle->setHoverPic(":/icons/skin/icons/category_hover_22px.svg");
    m_viewToggle->setPressPic(":/icons/skin/icons/category_active_22px.svg");
    m_modeToggle = new DImageButton;
    m_modeToggle->setNormalPic(":/icons/skin/icons/fullscreen_normal.png");
    m_modeToggle->setHoverPic(":/icons/skin/icons/fullscreen_hover.png");
    m_modeToggle->setPressPic(":/icons/skin/icons/fullscreen_press.png");

    m_searchWidget = new SearchWidget;
    m_searchWidget->setFixedWidth(180);
    m_searchWidget->edit()->installEventFilter(m_eventFilter);

    m_delayHideTimer->setInterval(200);
    m_delayHideTimer->setSingleShot(true);

    QHBoxLayout *viewHeaderLayout = new QHBoxLayout;
    viewHeaderLayout->addWidget(m_viewToggle);
    viewHeaderLayout->addStretch();
    viewHeaderLayout->addWidget(m_searchWidget);
    viewHeaderLayout->addStretch();
    viewHeaderLayout->addWidget(m_modeToggle);
    viewHeaderLayout->setSpacing(0);
    viewHeaderLayout->setContentsMargins(8, 5, 8, 5);

    m_appsBox = new DVBoxWidget;

    m_appsArea = new QScrollArea;
    m_appsArea->setObjectName("AppsArea");
    m_appsArea->setWidget(m_appsBox);
    m_appsArea->setWidgetResizable(true);
    m_appsArea->setFocusPolicy(Qt::NoFocus);
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_appsArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHBoxLayout *appsAreaLayout = new QHBoxLayout;
    appsAreaLayout->addWidget(m_categoryWidget);
    appsAreaLayout->addWidget(m_appsArea);
    appsAreaLayout->setSpacing(0);
    appsAreaLayout->setMargin(0);

    m_viewWrapper = new QWidget;
    m_viewWrapper->setLayout(appsAreaLayout);
    m_viewWrapper->setObjectName("ViewWrapper");

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addLayout(viewHeaderLayout);
    centralLayout->addWidget(m_viewWrapper);
    centralLayout->addWidget(m_bottomBar);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(10, 0, 10, 0);

    setMaskColor(DarkColor);
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::ClickFocus);
    setFixedSize(550, 485);
    setLayout(centralLayout);
    setObjectName("MiniFrame");
    setStyleSheet(getQssFromFile(":/skin/qss/miniframe.qss"));

    installEventFilter(m_eventFilter);

    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (MiniFrame::*)(const QModelIndex &)>(&MiniFrame::uninstallApp));
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_delayHideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_delayHideTimer, &QTimer::timeout, this, &MiniFrame::prepareHideLauncher);
    connect(m_searchWidget->edit(), &SearchLineEdit::textChanged, this, &MiniFrame::searchText, Qt::QueuedConnection);
    connect(m_modeToggle, &DImageButton::clicked, this, &MiniFrame::toggleFullScreen, Qt::QueuedConnection);
    connect(m_viewToggle, &DImageButton::clicked, this, &MiniFrame::onToggleViewClicked, Qt::QueuedConnection);
    connect(m_categoryWidget, &MiniCategoryWidget::requestCategory, m_appsModel, &AppsListModel::setCategory, Qt::QueuedConnection);
    connect(m_categoryWidget, &MiniCategoryWidget::requestCategory, this, &MiniFrame::checkIndex, Qt::QueuedConnection);
    connect(m_categoryWidget, &MiniCategoryWidget::requestRight, this, &MiniFrame::focusRightPanel);

    QTimer::singleShot(1, this, &MiniFrame::reloadAppsView);
}

MiniFrame::~MiniFrame()
{

}

void MiniFrame::showLauncher()
{
    if (visible())
        return;

    // reset env
    m_searchWidget->clearSearchContent();

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MiniFrame::adjustPosition, Qt::QueuedConnection);
    QTimer::singleShot(1, this, &MiniFrame::adjustPosition);

    show();
}

void MiniFrame::hideLauncher()
{
    if (!visible())
        return;

    disconnect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MiniFrame::adjustPosition);

    hide();
}

bool MiniFrame::visible()
{
    return isVisible();
}

void MiniFrame::moveCurrentSelectApp(const int key)
{
    const int mode = m_calcUtil->displayMode();
    if (mode == GROUP_BY_CATEGORY)
    {
        if (key == Qt::Key_Left)
            return focusLeftPanel();
        else if (key == Qt::Key_Right)
            return;
    }

    const QModelIndex currentIdx = currentIndex();

    QModelIndex targetIndex;

    do {
        if (!currentIdx.isValid() ||
            currentIdx.model() != m_appsView->model() ||
            currentIdx.row() >= currentIdx.model()->rowCount(QModelIndex()))
        {
            targetIndex = m_appsView->model()->index(0, 0);
            break;
        }

        const int c = 0;
        const int r = currentIdx.row();
        const int column_per_line = m_calcUtil->appColumnCount();

        switch (key)
        {
        case Qt::Key_Backtab:
        case Qt::Key_Left:
            targetIndex = currentIdx.sibling(r - 1, c);
            break;
        case Qt::Key_Tab:
        case Qt::Key_Right:
            targetIndex = currentIdx.sibling(r + 1, c);
            break;
        case Qt::Key_Down:
            targetIndex = currentIdx.sibling(r + column_per_line, c);
            break;
        case Qt::Key_Up:
            targetIndex = currentIdx.sibling(r - column_per_line, c);
            break;
        default:;
        }

    } while (false);

    if (!targetIndex.isValid())
        return;

    int y_offset = 0;
    if (mode == ALL_APPS)
    {
        static_cast<AppItemDelegate *>(m_appsView->itemDelegate())->setCurrentIndex(targetIndex);
        y_offset = static_cast<AppGridView *>(m_appsView)->indexYOffset(targetIndex);
    } else {
        m_appsView->setCurrentIndex(targetIndex);
    }

    m_appsArea->ensureVisible(0, y_offset, 0, 150);
}

void MiniFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->setFocus(Qt::MouseFocusReason);
    m_searchWidget->edit()->setText(m_searchWidget->edit()->text() + ch);
}

void MiniFrame::launchCurrentApp()
{
    const QModelIndex currentIdx = currentIndex();

    if (currentIdx.isValid() && currentIdx.model() == m_appsView->model())
        m_appsManager->launchApp(currentIdx);
    else
        m_appsManager->launchApp(m_appsView->model()->index(0, 0));

    hideLauncher();
}

void MiniFrame::showPopupMenu(const QPoint &pos, const QModelIndex &context)
{
    m_menuWorker->showMenuByAppItem(context, pos);
}

void MiniFrame::uninstallApp(const QString &appKey)
{
    uninstallApp(m_appsModel->indexAt(appKey));
}

void MiniFrame::uninstallApp(const QModelIndex &context)
{
    static bool UNINSTALL_DIALOG_SHOWN = false;

    if (UNINSTALL_DIALOG_SHOWN)
        return;

    UNINSTALL_DIALOG_SHOWN = true;
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
    UNINSTALL_DIALOG_SHOWN = false;
}

bool MiniFrame::windowDeactiveEvent()
{
    if (isVisible() && !m_menuWorker->isMenuShown())
        m_delayHideTimer->start();

    return false;
}

void MiniFrame::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton)
        hide();
}

void MiniFrame::keyPressEvent(QKeyEvent *e)
{
    DBlurEffectWidget::keyPressEvent(e);

    switch (e->key())
    {
    case Qt::Key_Escape:    hideLauncher();     break;
    default:;
    }
}

void MiniFrame::showEvent(QShowEvent *e)
{
    DBlurEffectWidget::showEvent(e);

    QTimer::singleShot(1, this, [this] () {
        raise();
        activateWindow();
        setFocus();
    });
}

void MiniFrame::enterEvent(QEvent *e)
{
    DBlurEffectWidget::enterEvent(e);

    m_delayHideTimer->stop();

    raise();
    activateWindow();
    setFocus();
}

void MiniFrame::leaveEvent(QEvent *e)
{
    DBlurEffectWidget::leaveEvent(e);

    m_delayHideTimer->start();
}

void MiniFrame::checkIndex()
{
    if (m_calcUtil->displayMode() != GROUP_BY_CATEGORY)
        return;

    const QModelIndex idx = currentIndex();

    if (!idx.isValid() || idx.row() >= idx.model()->rowCount(QModelIndex()))
        m_appsView->setCurrentIndex(m_appsView->model()->index(0, 0));
}

void MiniFrame::adjustPosition()
{
    const int dockPos = m_dockInter->position();
    const QRect dockRect = m_dockInter->frontendRect();

    const int spacing = 10;
    const QSize s = size();
    QPoint p;

    // extra spacing for efficient mode
    if (m_dockInter->displayMode() == DOCK_EFFICIENT)
    {
        const QRect primaryRect = qApp->primaryScreen()->geometry();

        switch (dockPos)
        {
        case DOCK_TOP:
            p = QPoint(primaryRect.left() + spacing, dockRect.bottom() + spacing);
            break;
        case DOCK_BOTTOM:
            p = QPoint(primaryRect.left() + spacing, dockRect.top() - s.height() - spacing);
            break;
        case DOCK_LEFT:
            p = QPoint(dockRect.right() + spacing, primaryRect.top() + spacing);
            break;
        case DOCK_RIGHT:
            p = QPoint(dockRect.left() - s.width() - spacing, primaryRect.top() + spacing);
            break;
        default: Q_UNREACHABLE_IMPL();
        }
    } else {
        switch (dockPos)
        {
        case DOCK_TOP:
            p = QPoint(dockRect.left(), dockRect.bottom() + spacing);
            break;
        case DOCK_BOTTOM:
            p = QPoint(dockRect.left(), dockRect.top() - s.height() - spacing);
            break;
        case DOCK_LEFT:
            p = QPoint(dockRect.right() + spacing, dockRect.top());
            break;
        case DOCK_RIGHT:
            p = QPoint(dockRect.left() - s.width() - spacing, dockRect.top());
            break;
        default: Q_UNREACHABLE_IMPL();
        }
    }

    move(p);
}

void MiniFrame::reloadAppsView()
{
    delete m_appsView;
    m_appsView = nullptr;

    CalculateUtil *calc = CalculateUtil::instance();

    if (calc->displayMode() == ALL_APPS)
    {
        AppItemDelegate *delegate = new AppItemDelegate;
        AppGridView *appsView = new AppGridView;
        appsView->setModel(m_appsModel);
        appsView->setItemDelegate(delegate);
        appsView->setContainerBox(m_appsArea);
        appsView->setSpacing(0);
        m_appsView = appsView;

        m_categoryWidget->setVisible(false);
        m_appsModel->setCategory(AppsListModel::All);

        connect(appsView, &AppGridView::popupMenuRequested, this, &MiniFrame::showPopupMenu);
        connect(delegate, &AppItemDelegate::currentChanged,
                appsView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));
    } else {
        AppListView *appsView = new AppListView;
        appsView->setModel(m_appsModel);
        appsView->setItemDelegate(new AppListDelegate);
        m_appsView = appsView;

        m_categoryWidget->setVisible(true);
        m_appsModel->setCategory(AppsListModel::All);

        connect(appsView, &AppListView::popupMenuRequested, this, &MiniFrame::showPopupMenu);

        QTimer::singleShot(1, this, &MiniFrame::focusRightPanel);
    }

    connect(m_appsView, &QListView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_appsView, &QListView::clicked, this, &MiniFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_appsView, &QListView::entered, this, &MiniFrame::setCurrentIndex);

    m_appsBox->layout()->addWidget(m_appsView);

    CalculateUtil::instance()->calculateAppLayout(QSize(), 0);
}

void MiniFrame::toggleFullScreen()
{
    removeEventFilter(m_eventFilter);

    const QStringList args {
        "--print-reply",
        "--dest=com.deepin.dde.daemon.Launcher",
        "/com/deepin/dde/daemon/Launcher",
        "org.freedesktop.DBus.Properties.Set",
        "string:com.deepin.dde.daemon.Launcher",
        "string:Fullscreen",
        "variant:boolean:true"
    };

    QProcess::startDetached("dbus-send", args);
}

void MiniFrame::onToggleViewClicked()
{
    const int mode = m_calcUtil->displayMode();

    m_calcUtil->setDisplayMode(mode == ALL_APPS ? GROUP_BY_CATEGORY : ALL_APPS);

    // reset env
    m_searchWidget->clearSearchContent();

    QTimer::singleShot(1, this, &MiniFrame::reloadAppsView);
}

void MiniFrame::prepareHideLauncher()
{
    if (!visible())
        return;

    if (geometry().contains(QCursor::pos()))
        return;

    hideLauncher();
}

void MiniFrame::focusRightPanel()
{
    m_categoryWidget->clearFocus();
    setFocus();

    AppListDelegate *delegate = qobject_cast<AppListDelegate *>(m_appsView->itemDelegate());
    if (delegate)
        delegate->setActived(true);

    style()->unpolish(m_categoryWidget);
    style()->polish(m_categoryWidget);
    setStyleSheet(styleSheet());
}

void MiniFrame::focusLeftPanel()
{
    Q_ASSERT(m_categoryWidget->isVisible());

    clearFocus();
    m_categoryWidget->setFocus();

    AppListDelegate *delegate = qobject_cast<AppListDelegate *>(m_appsView->itemDelegate());
    if (delegate)
        delegate->setActived(false);

    style()->unpolish(m_categoryWidget);
    style()->polish(m_categoryWidget);
    setStyleSheet(styleSheet());
}

void MiniFrame::setCurrentIndex(const QModelIndex &index)
{
    if (m_calcUtil->displayMode() == ALL_APPS)
        static_cast<AppItemDelegate *>(m_appsView->itemDelegate())->setCurrentIndex(index);
    else
        m_appsView->setCurrentIndex(index);
}

void MiniFrame::searchText(const QString &text)
{
    static int last_mode;

    if (text.isEmpty())
    {
        if (last_mode != m_calcUtil->displayMode())
        {
            m_calcUtil->setDisplayMode(last_mode);
            reloadAppsView();
        }

        m_appsView->setModel(m_appsModel);
    } else {
        if (m_appsView->model() != m_searchModel)
        {
            last_mode = m_calcUtil->displayMode();
            if (last_mode == GROUP_BY_CATEGORY)
            {
                m_calcUtil->setDisplayMode(ALL_APPS);
                reloadAppsView();
            }

            m_appsView->setModel(m_searchModel);
        }

        m_appsManager->searchApp(text.trimmed());
    }
}

const QModelIndex MiniFrame::currentIndex() const
{
    const int mode = m_calcUtil->displayMode();

    const QModelIndex currentIndex =
            mode == ALL_APPS
                ? static_cast<AppItemDelegate *>(m_appsView->itemDelegate())->currentIndex()
                : m_appsView->currentIndex();

    return currentIndex;
}
