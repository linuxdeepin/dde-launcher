/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * Maintainer: rekols <rekols@foxmail.com>
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

#include "windowedframe.h"
#include "widgets/hseparator.h"
#include "global_util/util.h"

#include <DDBusSender>
#include <ddialog.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QEvent>
#include <QTimer>
#include <QDebug>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

#define DOCK_FASHION    0
#define DOCK_EFFICIENT  1

extern const QPoint widgetRelativeOffset(const QWidget * const self, const QWidget *w);

inline const QPoint scaledPosition(const QPoint &xpos)
{
    const auto ratio = qApp->devicePixelRatio();
    QRect g = qApp->primaryScreen()->geometry();
    for (auto *screen : qApp->screens()) {
        const QRect &sg = screen->geometry();
        const QRect &rg = QRect(sg.topLeft(), sg.size() * ratio);
        if (rg.contains(xpos)) {
            g = rg;
            break;
        }
    }

    return g.topLeft() + (xpos - g.topLeft()) / ratio;
}

WindowedFrame::WindowedFrame(QWidget *parent)
    : QWidget(parent)
    , m_dockInter(new DBusDock(this))
    , m_menuWorker(new MenuWorker)
    , m_eventFilter(new SharedEventFilter(this))
    , m_windowHandle(this, this)
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_appsManager(AppsManager::instance())
    , m_appsView(new AppListView)
    , m_appsModel(new AppsListModel(AppsListModel::All))
    , m_usedModel(new AppsListModel(AppsListModel::Used))
    , m_searchModel(new AppsListModel(AppsListModel::Search))
    , m_searchWidget(new SearchWidget)
    , m_rightBar(new MiniFrameRightBar)
    , m_switchBtn(new MiniFrameSwitchBtn)
    , m_tipsLabel(new QLabel(this))
    , m_delayHideTimer(new QTimer)
    , m_autoScrollTimer(new QTimer)
    , m_regionMonitor(new DRegionMonitor(this))
    , m_displayMode(Used)
{
    DBlurEffectWidget *bgWidget = new DBlurEffectWidget(this);
    bgWidget->setMaskColor(DBlurEffectWidget::DarkColor);
    bgWidget->setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    m_windowHandle.setShadowRadius(60);
    m_windowHandle.setBorderWidth(0);
    m_windowHandle.setShadowOffset(QPoint(0, -1));
    m_windowHandle.setEnableBlurWindow(false);
    m_windowHandle.setTranslucentBackground(true);

    m_appsView->setModel(m_usedModel);
    m_appsView->setItemDelegate(new AppListDelegate);

    m_searchWidget->installEventFilter(m_eventFilter);

    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(500, 50);
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setStyleSheet("color:rgba(238, 238, 238, .6);"
                               "font-size:22px;");

    m_delayHideTimer->setInterval(200);
    m_delayHideTimer->setSingleShot(true);

    m_autoScrollTimer->setInterval(DLauncher::APPS_AREA_AUTO_SCROLL_TIMER);
    m_autoScrollTimer->setSingleShot(false);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addSpacing(10);
    searchLayout->addWidget(m_searchWidget);
    searchLayout->addSpacing(10);

    QHBoxLayout *appsLayout = new QHBoxLayout;
    appsLayout->addSpacing(10);
    appsLayout->addWidget(m_appsView);
    appsLayout->addSpacing(10);

    QHBoxLayout *switchLayout = new QHBoxLayout;
    switchLayout->addSpacing(10);
    switchLayout->addWidget(m_switchBtn);
    switchLayout->addSpacing(10);

    QVBoxLayout *containLayout = new QVBoxLayout;
    containLayout->setSpacing(0);
    containLayout->setMargin(0);

    containLayout->addSpacing(10);
    containLayout->addLayout(searchLayout);
    containLayout->addSpacing(10);
    containLayout->addWidget(new HSeparator);
    containLayout->addSpacing(4);
    containLayout->addLayout(appsLayout);
    containLayout->addLayout(switchLayout);
    containLayout->addSpacing(15);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(containLayout);
    mainLayout->addWidget(m_rightBar);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    m_switchBtn->updateStatus(Used);

    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::ClickFocus);
    setFixedSize(520, 502);
    setLayout(mainLayout);
    setObjectName("MiniFrame");
    setStyleSheet(getQssFromFile(":/skin/qss/miniframe.qss"));

    bgWidget->resize(size());

    initAnchoredCornor();
    installEventFilter(m_eventFilter);

    // auto scroll when drag to app list box border
    connect(m_appsView, &AppListView::requestScrollStop, m_autoScrollTimer, &QTimer::stop);
    connect(m_autoScrollTimer, &QTimer::timeout, [this] {
        m_appsView->verticalScrollBar()->setValue(m_appsView->verticalScrollBar()->value() + m_autoScrollStep);
    });
    connect(m_appsView, &AppListView::requestScrollUp, [this] {
        m_autoScrollStep = -DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
        if (!m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();
    });
    connect(m_appsView, &AppListView::requestScrollDown, [this] {
        m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
        if (!m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();
    });

    connect(m_rightBar, &MiniFrameRightBar::modeToggleBtnClicked, this, &WindowedFrame::onToggleFullScreen);
    connect(m_rightBar, &MiniFrameRightBar::requestFrameHide, this, &WindowedFrame::hideLauncher);
    connect(m_switchBtn, &MiniFrameSwitchBtn::jumpButtonClicked, this, &WindowedFrame::onJumpBtnClicked);

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &WindowedFrame::onWMCompositeChanged);
    connect(m_searchWidget->edit(), &SearchLineEdit::textChanged, this, &WindowedFrame::searchText, Qt::QueuedConnection);
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (WindowedFrame::*)(const QModelIndex &)>(&WindowedFrame::uninstallApp));
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_delayHideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &WindowedFrame::hideLauncher);

    connect(m_appsView, &QListView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_appsView, &QListView::clicked, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_appsView, &QListView::entered, m_appsView, &AppListView::setCurrentIndex);
    connect(m_appsView, &AppListView::popupMenuRequested, m_menuWorker.get(), &MenuWorker::showMenuByAppItem);

    connect(m_appsManager, &AppsManager::requestTips, this, &WindowedFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &WindowedFrame::hideTips);
    connect(m_appsManager, &AppsManager::newItemCreated, this, &WindowedFrame::onNewItemCreated);
    connect(m_switchBtn, &QPushButton::clicked, this, &WindowedFrame::onSwitchBtnClicked);
    connect(m_delayHideTimer, &QTimer::timeout, this, &WindowedFrame::prepareHideLauncher);

    connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, [=] (const QPoint &point) {
            if (!geometry().contains(point)) {
                hideLauncher();
            }
    });

    QTimer::singleShot(1, this, &WindowedFrame::onWMCompositeChanged);

    if (m_appsManager->isHaveNewInstall()) {
        m_switchBtn->showJumpBtn();
    }
}

WindowedFrame::~WindowedFrame()
{
    m_eventFilter->deleteLater();
}

void WindowedFrame::showLauncher()
{
    if (visible() || m_delayHideTimer->isActive())
        return;

    m_searchWidget->clearSearchContent();
    qApp->processEvents();

    adjustPosition();
    show();

    m_regionMonitor->registerRegion();

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &WindowedFrame::adjustPosition, Qt::UniqueConnection);
}

void WindowedFrame::hideLauncher()
{
    if (!visible()) {
        return;
    }

    disconnect(m_dockInter, &DBusDock::FrontendRectChanged, this, &WindowedFrame::adjustPosition);

    m_regionMonitor->unregisterRegion();

    hide();

    // clean all state
    recoveryAll();
}

bool WindowedFrame::visible()
{
    return isVisible();
}

void WindowedFrame::moveCurrentSelectApp(const int key)
{
    const QModelIndex currentIdx = m_appsView->currentIndex();
    QModelIndex targetIndex;

    const int row = currentIdx.row();

    switch (key) {
    case Qt::Key_Down:
    case Qt::Key_Right:
    case Qt::Key_Tab:
        targetIndex = currentIdx.sibling(row + 1, 0);
        break;
    case Qt::Key_Up: case Qt::Key_Left:
        targetIndex = currentIdx.sibling(row - 1, 0);
        break;
    }

    if (!currentIdx.isValid() || !targetIndex.isValid()) {
        targetIndex = m_appsView->model()->index(0, 0);
    }

    m_appsView->setCurrentIndex(targetIndex);
}

void WindowedFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->setFocus(Qt::MouseFocusReason);
    m_searchWidget->edit()->setText(m_searchWidget->edit()->text() + ch);
}

void WindowedFrame::launchCurrentApp()
{
    const QModelIndex currentIdx = m_appsView->currentIndex();

    if (currentIdx.isValid() && currentIdx.model() == m_appsView->model()) {
        m_appsManager->launchApp(currentIdx);
    } else {
        m_appsManager->launchApp(m_appsView->model()->index(0, 0));
    }

    hideLauncher();
}

void WindowedFrame::uninstallApp(const QString &appKey)
{
    uninstallApp(m_appsModel->indexAt(appKey));
}

void WindowedFrame::uninstallApp(const QModelIndex &context)
{
    static bool UNINSTALL_DIALOG_SHOWN = false;

    if (UNINSTALL_DIALOG_SHOWN) {
        return;
    }

    UNINSTALL_DIALOG_SHOWN = true;
    DTK_WIDGET_NAMESPACE::DDialog unInstallDialog;
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    const QString appKey = context.data(AppsListModel::AppKeyRole).toString();
    QString appName = context.data(AppsListModel::AppNameRole).toString();
    unInstallDialog.setTitle(QString(tr("Are you sure to uninstall %1 ?")).arg(appName));
    QPixmap appIcon = context.data(AppsListModel::AppDialogIconRole).value<QPixmap>();
    unInstallDialog.setIconPixmap(appIcon);

    QString message = tr("All dependencies will be removed together");
    unInstallDialog.setMessage(message);
    QStringList buttons;
    buttons << tr("Cancel") << tr("Confirm");
    unInstallDialog.addButtons(buttons);

    connect(&unInstallDialog, &DTK_WIDGET_NAMESPACE::DDialog::buttonClicked, [&] (int clickedResult) {
        // 0 means "cancel" button clicked
        if (clickedResult == 0) {
            return;
        }

        m_appsManager->uninstallApp(appKey);
    });

    // hide frame
    QTimer::singleShot(1, this, &WindowedFrame::hideLauncher);

    unInstallDialog.exec();
    UNINSTALL_DIALOG_SHOWN = false;
}

bool WindowedFrame::windowDeactiveEvent()
{
    if (isVisible() && !m_menuWorker->isMenuShown()) {
        m_delayHideTimer->start();
    }

    return false;
}

QPainterPath WindowedFrame::getCornerPath(AnchoredCornor direction)
{
    const QRect rect = this->rect();
    const QPoint topLeft = rect.topLeft();
    const QPoint topRight = rect.topRight();
    const QPoint bottomLeft = rect.bottomLeft();
    const QPoint bottomRight = rect.bottomRight();

    QPainterPath path;

    switch (direction) {
    case TopLeft:
        path.moveTo(topLeft.x() + m_radius, topLeft.y());
        path.lineTo(topRight.x(), topRight.y());
        path.lineTo(bottomRight.x(), bottomRight.y());
        path.lineTo(bottomLeft.x(), bottomLeft.y());
        path.arcTo(topLeft.x(), topLeft.y(), m_radius * 2, m_radius * 2, -180, -90);
        break;

    case TopRight:
        path.moveTo(topLeft.x(), topLeft.y());
        path.lineTo(topRight.x() - m_radius, topRight.y());
        path.arcTo(topRight.x() - m_radius * 2, topRight.y(), m_radius * 2, m_radius * 2, 90, -90);
        path.lineTo(bottomRight.x(), bottomRight.y());
        path.lineTo(bottomLeft.x(), bottomLeft.y());
        break;

    case BottomLeft:
        path.moveTo(topLeft.x(), topLeft.y());
        path.lineTo(topRight.x(), topRight.y());
        path.lineTo(bottomRight.x(), bottomRight.y());
        path.lineTo(bottomLeft.x() + m_radius, bottomLeft.y());
        path.arcTo(bottomLeft.x(), bottomLeft.y() - m_radius * 2, m_radius * 2, m_radius * 2, -90, -90);
        break;

    case BottomRight:
        path.moveTo(topLeft.x(), topLeft.y());
        path.lineTo(topRight.x(), topRight.y());
        path.lineTo(bottomRight.x(), bottomRight.y() - m_radius);
        path.arcTo(bottomRight.x() - m_radius * 2, bottomRight.y() - m_radius * 2, m_radius * 2, m_radius * 2, 0, -90);
        path.lineTo(bottomLeft.x(), bottomLeft.y());
        break;
    default:;
    }

    return path;
}

void WindowedFrame::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setClipPath(m_cornerPath);
    painter.fillPath(m_cornerPath, Qt::red);

    m_windowHandle.setClipPath(m_cornerPath);
}

void WindowedFrame::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    // PM dont't want auto-hide when click WindowedFrame blank area.
//    if (e->button() == Qt::LeftButton) {
//        hideLauncher();
//    }
}

void WindowedFrame::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);

    switch (e->key()) {
    case Qt::Key_Escape:
        hideLauncher();
        break;
    default:
        break;
    }
}

void WindowedFrame::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);

    QTimer::singleShot(1, this, [this] () {
        raise();
        activateWindow();
        setFocus();
        emit visibleChanged(true);
    });
}

void WindowedFrame::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);

    QTimer::singleShot(1, this, [=] { emit visibleChanged(false); });
}

void WindowedFrame::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);

    m_delayHideTimer->stop();

    raise();
    activateWindow();
    setFocus();
}

void WindowedFrame::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        m_searchWidget->edit()->setText(e->commitString());
        m_searchWidget->edit()->setFocus();
    }

    QWidget::inputMethodEvent(e);
}

QVariant WindowedFrame::inputMethodQuery(Qt::InputMethodQuery prop) const
{
    switch (prop) {
    case Qt::ImEnabled:
        return true;
    case Qt::ImCursorRectangle:
        return widgetRelativeOffset(this, m_searchWidget->edit());
    default: ;
    }

    return QWidget::inputMethodQuery(prop);
}

void WindowedFrame::initAnchoredCornor()
{
    if (m_dockInter->displayMode() == DOCK_EFFICIENT && m_wmHelper->hasComposite()) {

        const int dockPos = m_dockInter->position();

        switch (dockPos) {
        case DOCK_TOP:
            m_anchoredCornor = BottomRight;
            break;
        case DOCK_BOTTOM:
            m_anchoredCornor = TopRight;
            break;
        case DOCK_LEFT:
            m_anchoredCornor = BottomRight;
            break;
        case DOCK_RIGHT:
            m_anchoredCornor = BottomLeft;
            break;
        }

    } else {
        m_anchoredCornor = Normal;
    }

    // when change calculated only once.
    m_cornerPath = getCornerPath(m_anchoredCornor);

    update();
}

void WindowedFrame::adjustPosition()
{
    const auto ratio = devicePixelRatioF();
    const int dockPos = m_dockInter->position();
    const QRect &r = m_dockInter->frontendRect();
    QRect dockRect = QRect(scaledPosition(r.topLeft()), r.size() / ratio);

    const int dockSpacing = 0;
    const int screenSpacing = 0;
    const auto &s = size();
    QPoint p;

    // extra spacing for efficient mode
    if (m_dockInter->displayMode() == DOCK_EFFICIENT) {
        const QRect primaryRect = qApp->primaryScreen()->geometry();

        switch (dockPos) {
        case DOCK_TOP:
            p = QPoint(primaryRect.left() + screenSpacing, dockRect.bottom() + dockSpacing + 1);
            break;
        case DOCK_BOTTOM:
            p = QPoint(primaryRect.left() + screenSpacing, dockRect.top() - s.height() - dockSpacing + 1);
            break;
        case DOCK_LEFT:
            p = QPoint(dockRect.right() + dockSpacing + 1, primaryRect.top() + screenSpacing);
            break;
        case DOCK_RIGHT:
            p = QPoint(dockRect.left() - s.width() - dockSpacing + 1, primaryRect.top() + screenSpacing);
            break;
        default:
            Q_UNREACHABLE_IMPL();
        }
    } else {
        switch (dockPos) {
        case DOCK_TOP:
            p = QPoint(dockRect.left(), dockRect.bottom() + dockSpacing + 1);
            break;
        case DOCK_BOTTOM:
            p = QPoint(dockRect.left(), dockRect.top() - s.height() - dockSpacing);
            break;
        case DOCK_LEFT:
            p = QPoint(dockRect.right() + dockSpacing + 1, dockRect.top());
            break;
        case DOCK_RIGHT:
            p = QPoint(dockRect.left() - s.width() - dockSpacing, dockRect.top());
            break;
        default:
            Q_UNREACHABLE_IMPL();
        }
    }

    initAnchoredCornor();
    move(p);
}

void WindowedFrame::onToggleFullScreen()
{
    removeEventFilter(m_eventFilter);

    DDBusSender()
            .service("com.deepin.dde.daemon.Launcher")
            .interface("com.deepin.dde.daemon.Launcher")
            .path("/com/deepin/dde/daemon/Launcher")
            .property("Fullscreen")
            .set(true);
}

void WindowedFrame::onSwitchBtnClicked()
{
    m_switchBtn->hideJumpBtn();

    if (m_displayMode == Used) {
        m_displayMode = All;
        m_appsView->setModel(m_appsModel);
        m_switchBtn->updateStatus(All);
    } else {
        m_displayMode = Used;
        m_appsView->setModel(m_usedModel);
        m_switchBtn->updateStatus(Used);
        if (m_appsManager->isHaveNewInstall()) {
            m_switchBtn->showJumpBtn();
        }
    }

    hideTips();
}

void WindowedFrame::onJumpBtnClicked()
{
    onSwitchBtnClicked();
    m_switchBtn->hideJumpBtn();
    m_appsView->scrollToBottom();
}

void WindowedFrame::onWMCompositeChanged()
{
    if (m_wmHelper->hasComposite()) {
        m_windowHandle.setWindowRadius(5);
        m_windowHandle.setBorderColor(QColor(255, 255, 255, .1 * 255));
    } else {
        m_windowHandle.setWindowRadius(0);
        m_windowHandle.setBorderColor(QColor("#2C3238"));
    }
}

void WindowedFrame::onNewItemCreated()
{
    if (m_appsView->model() == m_usedModel) {
        m_switchBtn->showJumpBtn();
    }
}

void WindowedFrame::searchText(const QString &text)
{
    if (text.isEmpty()) {
        m_appsView->setModel((m_displayMode == All) ? m_appsModel : m_usedModel);
        hideTips();
    } else {
        if (m_appsView->model() != m_searchModel) {
            m_appsView->setModel(m_searchModel);
        }

        m_appsManager->searchApp(text.trimmed());
    }
}

void WindowedFrame::showTips(const QString &text)
{
    if (m_appsView->model() != m_searchModel)
        return;

    m_tipsLabel->setText(text);

    const QPoint center = m_appsView->rect().center() - m_tipsLabel->rect().center();
    m_tipsLabel->move(center);
    m_tipsLabel->setVisible(true);
    m_tipsLabel->raise();
}

void WindowedFrame::hideTips()
{
    m_tipsLabel->setVisible(false);
}

void WindowedFrame::prepareHideLauncher()
{
    if (!visible()) {
        return;
    }

    if (geometry().contains(QCursor::pos())) {
        return activateWindow(); /* get focus back */
    }

    hideLauncher();
}

void WindowedFrame::recoveryAll()
{
    // recovery list view
    m_displayMode = Used;
    m_appsView->setModel(m_usedModel);

    // recovery switch button
    m_switchBtn->updateStatus(Used);
    hideTips();
    if (m_appsManager->isHaveNewInstall()) {
        m_switchBtn->showJumpBtn();
    }
}
