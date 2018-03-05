/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "miniframe.h"
#include "dbusdock.h"
#include "historywidget.h"
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
#include "global_util/recentlyused.h"

#include "sharedeventfilter.h"

#include <QRect>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStyle>
#include <QScrollBar>

#include <ddialog.h>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

#define DOCK_FASHION    0
#define DOCK_EFFICIENT  1

extern const QPoint widgetRelativeOffset(const QWidget * const self, const QWidget *w);

const QPoint scaledPosition(const QPoint &xpos)
{
    const auto ratio = qApp->devicePixelRatio();
    QRect g = qApp->primaryScreen()->geometry();
    for (auto *screen : qApp->screens())
    {
        const QRect &sg = screen->geometry();
        const QRect &rg = QRect(sg.topLeft(), sg.size() * ratio);
        if (rg.contains(xpos))
        {
            g = rg;
            break;
        }
    }

    return g.topLeft() + (xpos - g.topLeft()) / ratio;
}

MiniFrame::MiniFrame(QWidget *parent)
    : DBlurEffectWidget(parent),

      m_menuWorker(new MenuWorker),
      m_windowHandle(this, this),
      m_wmHelper(DWindowManagerHelper::instance()),
      m_dockInter(new DBusDock(this)),
      m_eventFilter(new SharedEventFilter(this)),
      m_appsManager(AppsManager::instance()),
      m_calcUtil(CalculateUtil::instance()),

      m_delayHideTimer(new QTimer(this)),
      m_autoScrollTimer(new QTimer(this)),

      m_categoryWidget(new MiniCategoryWidget),
      m_bottomBar(new MiniFrameBottomBar),

#ifdef HISTORY_PANEL
      m_historyWidget(new HistoryWidget),
#endif
      m_appsView(nullptr),
      m_appsModel(new AppsListModel(AppsListModel::All)),
      m_searchModel(new AppsListModel(AppsListModel::Search))
{
    m_windowHandle.setShadowRadius(60);
    m_windowHandle.setEnableBlurWindow(false);
    m_windowHandle.setTranslucentBackground(true);

    m_bottomBar->setFixedHeight(40);
    m_categoryWidget->setFixedWidth(140);
    m_categoryWidget->setVisible(false);

    m_viewToggle = new DImageButton;
    m_viewToggle->setNormalPic(":/icons/skin/icons/category_normal_22px.png");
    m_viewToggle->setHoverPic(":/icons/skin/icons/category_hover_22px.png");
    m_viewToggle->setPressPic(":/icons/skin/icons/category_active_22px.png");
#ifdef HISTORY_PANEL
    m_historyToggle = new QPushButton("最近打开");
#endif
    m_modeToggle = new DImageButton;
    m_modeToggle->setNormalPic(":/icons/skin/icons/fullscreen_normal.png");
    m_modeToggle->setHoverPic(":/icons/skin/icons/fullscreen_hover.png");
    m_modeToggle->setPressPic(":/icons/skin/icons/fullscreen_press.png");

    m_searchWidget = new SearchWidget;
    m_searchWidget->setFixedWidth(180);
    m_searchWidget->edit()->installEventFilter(m_eventFilter);

    m_delayHideTimer->setInterval(200);
    m_delayHideTimer->setSingleShot(true);

    m_autoScrollTimer->setInterval(DLauncher::APPS_AREA_AUTO_SCROLL_TIMER);
    m_autoScrollTimer->setSingleShot(false);

    QHBoxLayout *viewHeaderLayout = new QHBoxLayout;
    viewHeaderLayout->addWidget(m_viewToggle);
#ifdef HISTORY_PANEL
    viewHeaderLayout->addWidget(m_historyToggle);
#endif
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
    m_appsArea->horizontalScrollBar()->setEnabled(false);

    QHBoxLayout *appsAreaLayout = new QHBoxLayout;
    appsAreaLayout->addWidget(m_categoryWidget);
    appsAreaLayout->addWidget(m_appsArea);
    appsAreaLayout->setSpacing(0);
    appsAreaLayout->setMargin(0);

    m_viewWrapper = new QLabel;
    m_viewWrapper->setLayout(appsAreaLayout);
    m_viewWrapper->setAlignment(Qt::AlignCenter);
    m_viewWrapper->setObjectName("ViewWrapper");
    m_viewWrapper->setStyleSheet("QLabel {"
                                 "color: rgba(238, 238, 238, .6);"
                                 "font-size: 22px;"
                                 "}");

#ifdef HISTORY_PANEL
    m_historyWidget->setVisible(false);
#endif

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addLayout(viewHeaderLayout);
    centralLayout->addWidget(m_viewWrapper);
#ifdef HISTORY_PANEL
    centralLayout->addWidget(m_historyWidget);
#endif
    centralLayout->addWidget(m_bottomBar);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(10, 0, 10, 0);

    setMaskColor(DarkColor);
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::ClickFocus);
    setFixedSize(550, 485);
    setLayout(centralLayout);
    setObjectName("MiniFrame");
    setStyleSheet(getQssFromFile(":/skin/qss/miniframe.qss"));

    installEventFilter(m_eventFilter);

    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (MiniFrame::*)(const QModelIndex &)>(&MiniFrame::uninstallApp));
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_delayHideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &MiniFrame::hideLauncher);
    connect(m_delayHideTimer, &QTimer::timeout, this, &MiniFrame::prepareHideLauncher);
    connect(m_searchWidget->edit(), &SearchLineEdit::textChanged, this, &MiniFrame::searchText, Qt::QueuedConnection);
    connect(m_modeToggle, &DImageButton::clicked, this, &MiniFrame::onToggleFullScreen, Qt::QueuedConnection);
#ifdef HISTORY_PANEL
    connect(m_historyToggle, &QPushButton::clicked, this, &MiniFrame::onToggleHistoryClicked, Qt::QueuedConnection);
#endif
    connect(m_viewToggle, &DImageButton::clicked, this, &MiniFrame::onToggleViewClicked, Qt::QueuedConnection);
    connect(m_categoryWidget, &MiniCategoryWidget::requestCategory, m_appsModel, &AppsListModel::setCategory, Qt::QueuedConnection);
    connect(m_categoryWidget, &MiniCategoryWidget::requestCategory, this, &MiniFrame::checkIndex, Qt::QueuedConnection);
    connect(m_categoryWidget, &MiniCategoryWidget::requestRight, this, &MiniFrame::focusRightPanel);
    connect(m_appsManager, &AppsManager::requestTips, m_viewWrapper, &QLabel::setText);
    connect(m_appsManager, &AppsManager::requestHideTips, m_viewWrapper, &QLabel::clear);
    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MiniFrame::onWMCompositeChanged);
    connect(m_bottomBar, &MiniFrameBottomBar::requestFrameHide, this, &MiniFrame::hideLauncher);

    QTimer::singleShot(1, this, &MiniFrame::reloadAppsView);
    QTimer::singleShot(1, this, &MiniFrame::onWMCompositeChanged);
}

MiniFrame::~MiniFrame()
{
    m_eventFilter->deleteLater();
}

void MiniFrame::showLauncher()
{
    if (visible())
        return;

    adjustPosition();

    qApp->processEvents();

    show();

    // reset env
    m_searchWidget->clearSearchContent();
    m_viewWrapper->clear();

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MiniFrame::adjustPosition, Qt::UniqueConnection);
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
        y_offset = static_cast<AppGridView *>(m_appsView)->indexYOffset(targetIndex);
        static_cast<AppItemDelegate *>(m_appsView->itemDelegate())->setCurrentIndex(targetIndex);
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

    // hide frame
    QTimer::singleShot(1, this, &MiniFrame::hideLauncher);

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
        hideLauncher();
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
        emit visibleChanged(true);
    });
}

void MiniFrame::hideEvent(QHideEvent *e)
{
    DBlurEffectWidget::hideEvent(e);

    QTimer::singleShot(1, this, [=] { emit visibleChanged(false); });
}

void MiniFrame::enterEvent(QEvent *e)
{
    DBlurEffectWidget::enterEvent(e);

    m_delayHideTimer->stop();

    raise();
    activateWindow();
    setFocus();
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
    const auto ratio = devicePixelRatioF();
    const int dockPos = m_dockInter->position();
    const QRect &r = m_dockInter->frontendRect();
    const QRect &dockRect = QRect(scaledPosition(r.topLeft()), r.size() / ratio);

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

        connect(appsView, &AppGridView::requestScrollStop, m_autoScrollTimer, &QTimer::stop);
        connect(appsView, &AppGridView::popupMenuRequested, this, &MiniFrame::showPopupMenu);
        connect(m_autoScrollTimer, &QTimer::timeout, this, [this] {
            m_appsArea->verticalScrollBar()->setValue(m_appsArea->verticalScrollBar()->value() + m_autoScrollStep);
        });
        connect(appsView, &AppGridView::requestScrollUp, this, [this] {
            m_autoScrollStep = -DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
            if (!m_autoScrollTimer->isActive())
                m_autoScrollTimer->start();
        });
        connect(appsView, &AppGridView::requestScrollDown, this, [this] {
            m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
            if (!m_autoScrollTimer->isActive())
                m_autoScrollTimer->start();
        });
        connect(delegate, &AppItemDelegate::requestUpdate,
                appsView, static_cast<void (AppGridView::*)(const QModelIndex&)>(&AppGridView::update));

        m_appsView = appsView;

        m_categoryWidget->setVisible(false);
        m_appsModel->setCategory(AppsListModel::All);
    } else {
        AppListView *appsView = new AppListView;
        appsView->setModel(m_appsModel);
        appsView->setItemDelegate(new AppListDelegate);
        m_appsView = appsView;

        m_categoryWidget->setVisible(true);
        m_appsModel->setCategory(m_categoryWidget->currentCategory());

        connect(appsView, &AppListView::popupMenuRequested, this, &MiniFrame::showPopupMenu);

        QTimer::singleShot(1, this, &MiniFrame::focusRightPanel);
    }

    connect(m_appsView, &QListView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_appsView, &QListView::clicked, this, &MiniFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_appsView, &QListView::entered, this, &MiniFrame::setCurrentIndex);

    m_appsBox->layout()->addWidget(m_appsView);

    CalculateUtil::instance()->calculateAppLayout(QSize(), 0);
}

void MiniFrame::onToggleFullScreen()
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
    m_viewWrapper->clear();

    QTimer::singleShot(1, this, &MiniFrame::reloadAppsView);
}

#ifdef HISTORY_PANEL
void MiniFrame::onToggleHistoryClicked()
{
    if (m_viewWrapper->isVisible())
    {
        m_viewWrapper->setVisible(false);
        m_historyWidget->setVisible(true);
    } else {
        m_viewWrapper->setVisible(true);
        m_historyWidget->setVisible(false);
    }
}
#endif

void MiniFrame::onWMCompositeChanged()
{
    if (m_wmHelper->hasComposite())
    {
        m_windowHandle.setWindowRadius(5);
        m_windowHandle.setBorderColor(QColor(255, 255, 255, .1 * 255));
    }
    else
    {
        m_windowHandle.setWindowRadius(0);
        m_windowHandle.setBorderColor(QColor("#2C3238"));
    }
}

void MiniFrame::prepareHideLauncher()
{
    if (!visible())
        return;

    if (geometry().contains(QCursor::pos()))
        return activateWindow(); /* get focus back */

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
        m_viewWrapper->clear();

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

void MiniFrame::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty())
    {
        m_searchWidget->edit()->setText(e->commitString());
        m_searchWidget->edit()->setFocus();
    }

    QWidget::inputMethodEvent(e);
}

QVariant MiniFrame::inputMethodQuery(Qt::InputMethodQuery prop) const
{
    switch (prop)
    {
    case Qt::ImEnabled:
        return true;
    case Qt::ImCursorRectangle:
        return widgetRelativeOffset(this, m_searchWidget->edit());
    default:;
    }

    return QWidget::inputMethodQuery(prop);
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
