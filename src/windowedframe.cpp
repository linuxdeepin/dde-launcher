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
#include "global_util/util.h"
#include "dbusdockinterface.h"
#include "constants.h"
#include "iconcachemanager.h"
#include "appitemdelegate.h"

#include <DWindowManagerHelper>
#include <DForeignWindow>
#include <DGuiApplicationHelper>
#include <DDBusSender>
#include <DDialog>
#include <QLabel>

#include <QApplication>
#include <QHBoxLayout>
#include <QClipboard>
#include <QScrollBar>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QEvent>
#include <QTimer>
#include <QDebug>
#include <QDBusConnection>

#include <qpa/qplatformwindow.h>

DGUI_USE_NAMESPACE

using namespace DLauncher;

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

/**
 * @brief WindowedFrame::WindowedFrame
 * 启动器小窗口模式下 界面处理类
 * @param parent
 */
WindowedFrame::WindowedFrame(QWidget *parent)
    : DBlurEffectWidget(parent)
#ifdef USE_AM_API
    , m_amDbusDockInter(new AMDBusDockInter(this))
#else
    , m_dockInter(new DBusDock(this))
#endif
    , m_menuWorker(new MenuWorker(this))
    , m_eventFilter(new SharedEventFilter(this))
    , m_windowHandle(this, this)
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_maskBg(new QWidget(this))
    , m_appsManager(AppsManager::instance())
    , m_appsModel(new AppsListModel(AppsListModel::TitleMode, this))
    , m_allAppsModel(new AppsListModel(AppsListModel::WindowedAll, this))
    , m_collectionModel(new AppsListModel(AppsListModel::Collect, this))
    , m_filterModel(new SortFilterProxyModel(this))
    , m_searchWidget(new SearchModeWidget(this))
    , m_bottomBtn(new MiniFrameRightBar(this))
    , m_appsView(new AppListView(this))
    , m_collectionView(new AppGridView(AppGridView::MainView, this))
    , m_allAppView(new AppGridView(AppGridView::MainView, this))
    , m_tipsLabel(new QLabel(this))
    , m_delayHideTimer(new QTimer(this))
    , m_autoScrollTimer(new QTimer(this))
    , m_appearanceInter(new Appearance("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
    , m_displayMode(All)
    , m_autoScrollStep(DLauncher::APPS_AREA_AUTO_SCROLL_STEP)
    , m_calcUtil(CalculateUtil::instance())
    , m_focusPos(Applist)
    , m_modeToggleBtn(new ModeToggleButton(this))
    , m_searcherEdit(new DSearchEdit(this))
    , m_enterSearchEdit(false)
    , m_curScreen(m_appsManager->currentScreen())
    , m_modeSwitch(new ModeSwitch(this))
{
    if (!getDConfigValue("enable-full-screen-mode", true).toBool())
        m_modeToggleBtn->hide();

    initUi();
    initConnection();
    setAccessibleName();
}

void WindowedFrame::initUi()
{
    setMaskColor(DBlurEffectWidget::AutoColor);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    m_appearanceInter->setSync(false, false);
    m_searcherEdit->installEventFilter(this);
    qApp->installEventFilter(this);

    m_windowHandle.setShadowRadius(60);
    m_windowHandle.setBorderWidth(0);
    m_windowHandle.setShadowOffset(QPoint(0, -1));
    m_windowHandle.setEnableBlurWindow(true);
    m_windowHandle.setTranslucentBackground(false);

    AppListDelegate *itemDelegate = new AppListDelegate(this);
    AppItemDelegate *appItemDelegate  = new AppItemDelegate(this);

    m_appsView->setModel(m_appsModel);
    m_appsView->setItemDelegate(itemDelegate);
    m_appsView->setAcceptDrops(false);
    m_appsView->installEventFilter(m_eventFilter);

    m_collectionView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_collectionView->setModel(m_collectionModel);
    m_collectionView->setItemDelegate(appItemDelegate);
    m_collectionView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_collectionView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_collectionView->setDragEnabled(true);
    m_collectionView->setDragDropMode(QAbstractItemView::DragDrop);
    m_collectionView->setAcceptDrops(true);

    m_filterModel->setSourceModel(m_allAppsModel);
    m_filterModel->setFilterRole(AppsListModel::AppRawItemInfoRole);
    m_filterModel->setFilterKeyColumn(0);
    m_filterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_allAppView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_allAppView->setModel(m_allAppsModel);
    m_allAppView->setItemDelegate(appItemDelegate);
    m_allAppView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_allAppView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_allAppView->setAcceptDrops(false);

    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(500, 50);
    m_tipsLabel->setVisible(false);
    m_delayHideTimer->setInterval(200);
    m_delayHideTimer->setSingleShot(true);

    m_autoScrollTimer->setInterval(DLauncher::APPS_AREA_AUTO_SCROLL_TIMER);
    m_autoScrollTimer->setSingleShot(false);

    setFixedSize(780, 600);
    QHBoxLayout *mainHlayout = new QHBoxLayout;
    mainHlayout->setContentsMargins(20, 22, 10, 10);
    mainHlayout->setSpacing(10);

    // 分类模式标题 +  按钮组
    QHBoxLayout *topHLayout = new QHBoxLayout;
    topHLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setText(tr("App Category"));
    topHLayout->addWidget(titleLabel);
    topHLayout->addWidget(m_modeSwitch);

    // 左侧列表 + 控件
    QVBoxLayout *leftVLayout = new QVBoxLayout;
    leftVLayout->addLayout(topHLayout);
    leftVLayout->addWidget(m_appsView);
    leftVLayout->addWidget(m_bottomBtn);

    // 右边控件布局器
    QVBoxLayout *rightVLayout = new QVBoxLayout;
    rightVLayout->setMargin(0);
    rightVLayout->setContentsMargins(0, 0, 0, 0);

    // 右边水平布局器
    QHBoxLayout *rightHLayout = new QHBoxLayout;
    rightHLayout->setMargin(0);
    rightHLayout->setContentsMargins(0, 0, 0, 0);
    rightHLayout->addWidget(m_searcherEdit);
    rightHLayout->addWidget(m_modeToggleBtn);

    // 收藏应用
    QVBoxLayout *commonUseVLayout = new QVBoxLayout;
    commonUseVLayout->setMargin(0);
    commonUseVLayout->setSpacing(0);
    commonUseVLayout->setContentsMargins(0, 0, 0, 0);
    m_collectLabel = new QLabel(tr("Collected Apps"), this);
    commonUseVLayout->addWidget(m_collectLabel);
    commonUseVLayout->addWidget(m_collectionView);

    // 所有应用
    QVBoxLayout *allAppVLayout = new QVBoxLayout;
    allAppVLayout->setMargin(0);
    allAppVLayout->setSpacing(0);
    allAppVLayout->setContentsMargins(0, 0, 0, 0);
    m_allAppLabel = new QLabel(tr("All Apps"), this);
    allAppVLayout->addWidget(m_allAppLabel);
    allAppVLayout->addWidget(m_allAppView);

    rightVLayout->addLayout(rightHLayout);
    rightVLayout->addLayout(commonUseVLayout);
    rightVLayout->addLayout(allAppVLayout);
    rightVLayout->addWidget(m_searchWidget);

    m_searchWidget->hide();

    mainHlayout->addLayout(leftVLayout);
    mainHlayout->addLayout(rightVLayout);
    mainHlayout->setStretch(0, 1);
    mainHlayout->setStretch(1, 2);
    setLayout(mainHlayout);

    setRightViewVisibleState(false);

    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("MiniFrame");

    create();
    if (windowHandle()) {
        windowHandle()->setProperty("_d_dwayland_window-type", "launcher");
    }

    initAnchoredCornor();
    installEventFilter(m_eventFilter);

    // 状态切换
    m_modeToggleBtn->setIconSize(QSize(32,32));
    m_modeToggleBtn->setFixedSize(40, 40);
    m_modeToggleBtn->setFocusPolicy(Qt::NoFocus);

    // 重置控件样式
    resetWidgetStyle();
    m_maskBg->setAutoFillBackground(true);
    m_maskBg->setFixedSize(size());
}

void WindowedFrame::initConnection()
{
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, [this] {
        m_collectionView->setSpacing(m_calcUtil->appItemSpacing());
    });

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, [this] {
        m_allAppView->setSpacing(m_calcUtil->appItemSpacing());
    });

    connect(m_modeSwitch, &ModeSwitch::buttonClicked, this, &WindowedFrame::onButtonClick);

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

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &WindowedFrame::onWMCompositeChanged);
    connect(m_searcherEdit, &DSearchEdit::textChanged, this, &WindowedFrame::searchText, Qt::QueuedConnection);

    // 右键菜单
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, m_appsManager, QOverload<const QModelIndex &>::of(&AppsManager::uninstallApp));
    connect(m_appsManager, &AppsManager::requestHideLauncher, this, &WindowedFrame::hideLauncher);
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_delayHideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_appsView, &AppListView::menuHide);
    connect(m_menuWorker.get(), &MenuWorker::requestEditCollected, m_appsManager, &AppsManager::onEditCollected);
    connect(m_menuWorker.get(), &MenuWorker::requestMoveToTop, m_appsManager, &AppsManager::onMoveToFirstInCollected);

    // 左侧应用列表
    connect(m_appsView, &QListView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_appsView, &QListView::clicked, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_appsView, &QListView::entered, m_appsView, &AppListView::setCurrentIndex, Qt::QueuedConnection);
    connect(m_appsView, &AppListView::popupMenuRequested, m_menuWorker.get(), &MenuWorker::showMenuByAppItem);

    connect(m_collectionView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_collectionView, &AppGridView::clicked, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_collectionView, &AppGridView::entered, m_collectionView, &AppGridView::setCurrentIndex, Qt::QueuedConnection);
    connect(m_collectionView, &AppGridView::popupMenuRequested, m_menuWorker.get(), &MenuWorker::showMenuByAppItem);

    // 所有应用
    connect(m_allAppView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_allAppView, &AppGridView::clicked, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_allAppView, &AppGridView::entered, m_allAppView, &AppGridView::setCurrentIndex, Qt::QueuedConnection);
    connect(m_allAppView, &AppGridView::popupMenuRequested, m_menuWorker.get(), &MenuWorker::showMenuByAppItem);

    // 搜索页面
    connect(m_searchWidget, &SearchModeWidget::connectViewEvent, this , &WindowedFrame::addViewEvent);

    connect(m_appsManager, &AppsManager::requestTips, this, &WindowedFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &WindowedFrame::hideTips);
    connect(m_appsManager, &AppsManager::dataChanged, this, &WindowedFrame::refreshView);
    connect(m_delayHideTimer, &QTimer::timeout, this, &WindowedFrame::prepareHideLauncher, Qt::QueuedConnection);

    connect(m_appearanceInter, &Appearance::OpacityChanged, this, &WindowedFrame::onOpacityChanged);
    connect(m_modeToggleBtn, &DToolButton::clicked, this, &WindowedFrame::onToggleFullScreen);

    QTimer::singleShot(1, this, &WindowedFrame::onWMCompositeChanged);
    onOpacityChanged(m_appearanceInter->opacity());

    connect(m_curScreen, &QScreen::geometryChanged, this, &WindowedFrame::onScreenInfoChange);
    connect(m_curScreen, &QScreen::orientationChanged, this, &WindowedFrame::onScreenInfoChange);
    connect(qApp, &QApplication::primaryScreenChanged, this, &WindowedFrame::onScreenInfoChange);

    connect(this, &WindowedFrame::visibleChanged, this, &WindowedFrame::onHideMenu);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &WindowedFrame::resetWidgetStyle);

    connect(m_bottomBtn, &MiniFrameRightBar::requestFrameHide, this, &WindowedFrame::hideLauncher);
}

void WindowedFrame::setAccessibleName()
{
    m_searcherEdit->setAccessibleName("searcherEdit");
    m_maskBg->setAccessibleName("MaskBg");
    m_modeToggleBtn->setAccessibleName("modeToggleBtn");
    m_searcherEdit->setAccessibleName("WindowedSearcherEdit");
    m_tipsLabel->setAccessibleName("tipsLabel");
    m_allAppView->setAccessibleName("allAppView");
    m_collectionView->setAccessibleName("collectView");
    m_appsView->setAccessibleName("appsView");
    m_searchWidget->setAccessibleName("searchWidget");
}

void WindowedFrame::setRightViewVisibleState(bool searched)
{
    m_collectLabel->setVisible(!searched);
    m_allAppLabel->setVisible(!searched);
    m_collectionView->setVisible(!searched);
    m_allAppView->setVisible(!searched);

    // 搜索控件
    m_searchWidget->setVisible(searched);

    update();
}

void WindowedFrame::showLauncher()
{
    if (m_delayHideTimer->isActive())
        return;

    // 显示后加载小窗口其他资源
    emit m_appsManager->loadOtherIcon();

    qApp->processEvents();
    activateWindow();
    setFocus(Qt::ActiveWindowFocusReason);

    m_appsView->setCurrentIndex(QModelIndex());

    adjustSize();
    adjustPosition();
    m_cornerPath = getCornerPath(m_anchoredCornor);
    m_windowHandle.setClipPath(m_cornerPath);
    show();
}

void WindowedFrame::hideLauncher()
{
    if (!visible()) {
        return;
    }

    m_delayHideTimer->stop();

    m_searcherEdit->lineEdit()->clear();
    m_searcherEdit->clearEdit();
    // clean all state
    recoveryAll();

    hide();
}

bool WindowedFrame::visible()
{
    return isVisible();
}

/**
 * @brief WindowedFrame::moveCurrentSelectApp 处理键盘事件
 * @param key 键盘按键名
 */
void WindowedFrame::moveCurrentSelectApp(const int key)
{
    if(Qt::Key_Undo == key) {
        auto  oldStr =  m_searcherEdit->lineEdit()->text();
        m_searcherEdit->lineEdit()->undo();
        if (!oldStr.isEmpty() && oldStr == m_searcherEdit->lineEdit()->text()) {
            m_searcherEdit->lineEdit()->clear();
        }
        return;
    }

    const QModelIndex currentIdx = m_appsView->currentIndex();
    QModelIndex targetIndex;

    const int row = currentIdx.row();
    switch (key) {
    case Qt::Key_Tab: {
        switch (m_focusPos) {
        case Default:
            m_focusPos = RightBottom;
            break;
        case RightBottom:
            m_focusPos = Computer;
            break;
        case Computer:
            m_focusPos = Setting;
            break;
        case Setting:
            m_focusPos = Power;
            break;
        case Power:
            m_focusPos = Search;
            m_searcherEdit->lineEdit()->setFocus();
            break;
        case Search:
            if (m_appsView->model()->rowCount() != 0 && m_appsView->model()->columnCount() != 0) {
                targetIndex = m_appsView->model()->index(0, 0);
            }
            m_focusPos = Applist;
            break;
        case Applist:
            m_focusPos = RightBottom;
            break;
        }
        break;
    }
    case Qt::Key_Backtab: {
        switch (m_focusPos) {
        case Default:
            m_focusPos = RightBottom;
            break;
        case RightBottom:
            m_focusPos = Applist;
            if (m_appsView->model()->rowCount() != 0 && m_appsView->model()->columnCount() != 0) {
                targetIndex = m_appsView->model()->index(0, 0);
            }
            break;
        case Computer:
            m_focusPos = RightBottom;
            break;
        case Setting:
            m_focusPos = Computer;
            break;
        case Power:
            m_focusPos = Setting;
            break;
        case Search:
            m_focusPos = Power;
            break;
        case Applist:
            m_focusPos = Search;
            m_searcherEdit->lineEdit()->setFocus();
            break;
        }
        break;
    }
    case Qt::Key_Up: {
        if (m_focusPos == Applist || m_focusPos == Search) {
            targetIndex = currentIdx.sibling(row - 1, 0);
            if (!currentIdx.isValid() || !targetIndex.isValid()) {
                targetIndex = m_appsView->model()->index(m_appsView->model()->rowCount() - 1, 0);
            }
        } else if (m_focusPos == Default) {
            m_focusPos = Computer;
        } else {
        }
        break;
    }
    case Qt::Key_Down: {
        if (m_focusPos == Applist || m_focusPos == Search) {
            targetIndex = currentIdx.sibling(row + 1, 0);
            if (!currentIdx.isValid() || !targetIndex.isValid()) {
                targetIndex = m_appsView->model()->index(0, 0);
            }
        } else if (m_focusPos == Default) {
            m_focusPos = Computer;
        } else {
        }
        break;
    }
    case Qt::Key_Left: {
        if (m_focusPos == Search || m_focusPos == Applist || m_focusPos == RightBottom || m_focusPos == Default) {
            m_focusPos = Applist;
            m_focusPos  = Computer;
        }
        break;
    }
    case Qt::Key_Right: {
        if (m_focusPos == Computer || m_focusPos == Setting || m_focusPos == Power || m_focusPos == Default) {
            m_focusPos = Applist;
            if (m_appsView->model()->rowCount() != 0 && m_appsView->model()->columnCount() != 0) {
                targetIndex = m_appsView->model()->index(0, 0);
            }
        }
        break;
    }
    default:
        break;
    }

    if (m_focusPos == Applist) {
        m_appsModel->setDrawBackground(true);
        m_appsView->setFocus();
    } else if (m_focusPos == RightBottom) {
        m_appsView->setCurrentIndex(QModelIndex());
        return;
    } else if (m_focusPos == Search) {
    } else {
        m_appsView->setCurrentIndex(QModelIndex());
        return;
    }

    // Hover conflict with the mouse, temporarily blocking the signal
    m_appsView->blockSignals(true);
    m_appsView->setCurrentIndex(targetIndex);
    m_appsView->blockSignals(false);
}

void WindowedFrame::appendToSearchEdit(const char ch)
{
    m_searcherEdit->lineEdit()->setFocus();

    //-1 means backspace key pressed
    if (ch == static_cast<const char>(-1)) {
        m_searcherEdit->lineEdit()->backspace();
        return;
    }

    if (!m_searcherEdit->lineEdit()->selectedText().isEmpty()) {
        m_searcherEdit->lineEdit()->backspace();
    }

    m_searcherEdit->lineEdit()->setText(m_searcherEdit->lineEdit()->text() + ch);

}

void WindowedFrame::launchCurrentApp()
{
    const QModelIndex currentIdx = m_appsView->currentIndex();

    if (m_focusPos == Computer || m_focusPos == Setting || m_focusPos == Power) {
        return;
    } else if (m_focusPos == RightBottom && !currentIdx.isValid()) {
        return;
    }

    if (currentIdx.isValid() && currentIdx.model() == m_appsView->model()) {
        m_appsManager->launchApp(currentIdx);
    } else {
        m_appsManager->launchApp(m_appsView->model()->index(0, 0));
    }

    hideLauncher();
}

void WindowedFrame::uninstallApp(const QString &appKey)
{
    m_appsManager->uninstallApp(m_appsModel->indexAt(appKey));
}

QPainterPath WindowedFrame::getCornerPath(AnchoredCornor direction)
{
#ifdef USE_AM_API
    if (m_amDbusDockInter->displayMode() == DLauncher::DOCK_FASHION)
#else
    if (m_dockInter->displayMode() == DLauncher::DOCK_FASHION)
#endif
       return QPainterPath();

    QPainterPath path;
    const QRect rect = this->rect();
    const QPoint topLeft = rect.topLeft();
    const QPoint topRight = rect.topRight();
    const QPoint bottomLeft = rect.bottomLeft();
    const QPoint bottomRight = rect.bottomRight();

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

void WindowedFrame::resetWidgetStyle()
{
    QPalette palette = m_tipsLabel->palette();
    palette.setBrush(QPalette::WindowText, palette.brightText());
    m_tipsLabel->setPalette(palette);

    palette = m_maskBg->palette();

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        QColor color(Qt::black);
        color.setAlpha(static_cast<int>(255 * 0.3));
        palette.setColor(QPalette::Background, color);
    } else {
        QColor color(Qt::white);
        color.setAlpha(static_cast<int>(255 * 0.3));
        palette.setColor(QPalette::Background, color);
    }

    m_maskBg->setPalette(palette);
}

void WindowedFrame::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    // PM don't want auto-hide when click WindowedFrame blank area.
//    if (e->button() == Qt::LeftButton) {
//        hideLauncher();
//    }
}

void WindowedFrame::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        hideLauncher();
    } else if (e->key() == Qt::Key_V &&  e->modifiers().testFlag(Qt::ControlModifier)) {
        const QString &clipboardText = QApplication::clipboard()->text();

        // support Ctrl+V shortcuts.
        if (!clipboardText.isEmpty()) {
            m_searcherEdit->lineEdit()->setText(clipboardText);
            m_searcherEdit->lineEdit()->setFocus();
        }
    }
}

void WindowedFrame::showEvent(QShowEvent *e)
{
    AppListDelegate * delegate = static_cast<AppListDelegate *>(m_appsView->itemDelegate());
    if (delegate) {
        delegate->setActived(true);
    }

    QWidget::showEvent(e);
    m_calcUtil->calculateAppLayout(m_allAppView->size());
    m_calcUtil->calculateAppLayout(m_collectionView->size());

    QTimer::singleShot(1, this, [this]() {
        raise();
        activateWindow();
        setFocus();
        emit visibleChanged(true);
    });
    m_focusPos = Applist;
}

void WindowedFrame::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);

    QTimer::singleShot(1, this, [ = ] { emit visibleChanged(false); });
}

void WindowedFrame::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);

    m_delayHideTimer->stop();

    activateWindow();
}

void WindowedFrame::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        m_searcherEdit->lineEdit()->setText(e->commitString());
    }

    QWidget::inputMethodEvent(e);
}

QVariant WindowedFrame::inputMethodQuery(Qt::InputMethodQuery prop) const
{
    return QWidget::inputMethodQuery(prop);
}

void WindowedFrame::regionMonitorPoint(const QPoint &point, int flag)
{
    Q_UNUSED(flag);
    auto windowList = DWindowManagerHelper::instance()->currentWorkspaceWindows();

    for (auto window : windowList) {
        if (window->handle()->geometry().contains(point)) {
            if (window->wmClass() == "onboard") return;
        }
    }

    if (!windowHandle()->handle()->geometry().contains(point)) {
        if (m_menuWorker->isMenuShown() && m_menuWorker->menuGeometry().contains(point)) {
            return;
        }

        hideLauncher();
    }
}

bool WindowedFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_appsView && event->type() == QEvent::Wheel) {
        m_searcherEdit->lineEdit()->clearFocus();
    }

    if (m_enterSearchEdit && watched->objectName() == QString("MiniFrameWindow")) {
        if (event->type()== QEvent::TouchBegin) {
            m_searcherEdit->lineEdit()->setFocus();
            m_enterSearchEdit = false;
        }
    }

    if (watched == m_searcherEdit && event->type() == QEvent::Enter)
        m_enterSearchEdit = true;

    // 清除行编辑器的焦点(已被dtk封装了)
    if (event->type() == QEvent::KeyPress && m_searcherEdit->lineEdit()->hasFocus()) {
        QKeyEvent *keyPress = static_cast<QKeyEvent *>(event);
        if (keyPress && keyPress->key() == Qt::Key_Tab)
            m_searcherEdit->lineEdit()->clearFocus();
    }

    return QWidget::eventFilter(watched, event);
}

void WindowedFrame::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, [ = ] {
        initAnchoredCornor();
        m_cornerPath = getCornerPath(m_anchoredCornor);
        m_windowHandle.setClipPath(m_cornerPath);
        // event.size() 第一次启动有时候会很大或者很小的负数,直接用固定的size
        m_maskBg->setFixedSize(size());
        m_maskBg->move(0,0);
    });

    return DBlurEffectWidget::resizeEvent(event);
}

void WindowedFrame::initAnchoredCornor()
{
    if (!m_wmHelper->hasComposite()) {
        m_anchoredCornor = Normal;
    } else {
#ifdef USE_AM_API
        const int dockPos = m_amDbusDockInter->position();
#else
        const int dockPos = m_dockInter->position();
#endif

        switch (dockPos) {
        case DLauncher::DOCK_TOP:
            m_anchoredCornor = BottomRight;
            break;
        case DLauncher::DOCK_BOTTOM:
            m_anchoredCornor = TopRight;
            break;
        case DLauncher::DOCK_LEFT:
            m_anchoredCornor = BottomRight;
            break;
        case DLauncher::DOCK_RIGHT:
            m_anchoredCornor = BottomLeft;
            break;
        }
    }

    update();
}

void WindowedFrame::adjustPosition()
{
    // 启动器高效模式和时尚模式与任务栏的间隙不同
    int dockSpacing = 0;

#ifdef USE_AM_API
    QRect rect =  m_amDbusDockInter->frontendWindowRect();
    int dockPos = m_amDbusDockInter->position();
    if (m_amDbusDockInter->displayMode() == DLauncher::DOCK_FASHION)
        dockSpacing = 8;
#else
    QRect rect =  m_dockInter->frontendRect();
    int dockPos = m_dockInter->position();
    if (m_dockInter->displayMode() == DLauncher::DOCK_FASHION)
        dockSpacing = 8;
#endif

    QRect dockRect = QRect(scaledPosition(rect.topLeft()),scaledPosition(rect.bottomRight()));

    QPoint launcherPoint;
    switch (dockPos) {
    case DLauncher::DOCK_TOP:
        launcherPoint = QPoint(dockRect.left(), dockRect.bottom() + dockSpacing + 1);
        break;
    case DLauncher::DOCK_BOTTOM:
        launcherPoint = QPoint(dockRect.left(), dockRect.top() - height() - dockSpacing);
        break;
    case DLauncher::DOCK_LEFT:
        launcherPoint = QPoint(dockRect.right() + dockSpacing + 1, dockRect.top());
        break;
    case DLauncher::DOCK_RIGHT:
        launcherPoint = QPoint(dockRect.left() - width() - dockSpacing, dockRect.top());
        break;
    default:
        Q_UNREACHABLE_IMPL();
    }

    initAnchoredCornor();
    move(launcherPoint);
}

void WindowedFrame::onToggleFullScreen()
{
    // 后台没有启动,切换全屏会造成后台默认数据和前台数据不统一,造成显示问题
    if (!m_appsManager->appNums(AppsListModel::FullscreenAll))
        return;

    // 切换到全屏就不绘制小窗口列表中的item
    AppListDelegate *delegate = static_cast<AppListDelegate *>(m_appsView->itemDelegate());
    if (delegate)
        delegate->setActived(false);

    // 全屏状态标识
    m_calcUtil->setFullScreen(true);

    // 同步本地当前是全屏的状态
    DDBusSender()
            .service(DBUS_DAEMON_SERVICE_NAME)
            .interface(DBUS_DAEMON_SERVICE_NAME)
            .path(DBUS_DAEMON_PATH_NAME)
            .property("Fullscreen")
            .set(true);
}

void WindowedFrame::onWMCompositeChanged()
{
    if (m_wmHelper->hasComposite())
        m_radius = 18;
    else
        m_radius = 0;

    initAnchoredCornor();
    m_cornerPath = getCornerPath(m_anchoredCornor);
    m_windowHandle.setClipPath(m_cornerPath);
}

void WindowedFrame::searchText(const QString &text)
{
    if (text.isEmpty()) {
        setRightViewVisibleState(false);
        hideTips();
    } else {
        emit searchApp(text.trimmed());

        setRightViewVisibleState(true);
        QRegExp regExp(text.trimmed(), Qt::CaseInsensitive);
        m_filterModel->setFilterRegExp(regExp);
        m_searchWidget->setSearchModel(m_filterModel);
        m_focusPos = Search;
    }
}

void WindowedFrame::showTips(const QString &text)
{
    const QPoint center = m_searchWidget->rect().center();
    m_tipsLabel->setText(text);
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
    if (!visible())
        return;

    if (geometry().contains(QCursor::pos()) || m_menuWorker->menuGeometry().contains(QCursor::pos()))
        return activateWindow(); /* get focus back */

    hideLauncher();
}

void WindowedFrame::recoveryAll()
{
    m_displayMode = All;
    hideTips();

    m_focusPos = Applist;
}

void WindowedFrame::onOpacityChanged(const double value)
{
    setMaskAlpha(value * 255);
}

void WindowedFrame::onScreenInfoChange()
{
    m_curScreen->disconnect();
    m_curScreen = m_appsManager->currentScreen();

    adjustSize();
    updatePosition();
    m_cornerPath = getCornerPath(m_anchoredCornor);
    m_windowHandle.setClipPath(m_cornerPath);

    connect(m_curScreen, &QScreen::geometryChanged, this, &WindowedFrame::onScreenInfoChange);
    connect(m_curScreen, &QScreen::orientationChanged, this, &WindowedFrame::onScreenInfoChange);
}

void WindowedFrame::updatePosition()
{
   // 该接口获取数据是翻转后的数据
   DBusDockInterface inter;
   QRect dockGeo = inter.geometry();
   QRect dockRect = QRect(scaledPosition(dockGeo.topLeft()),scaledPosition(dockGeo.bottomRight()));

    int dockSpacing = 0;
#ifdef USE_AM_API
    int dockPos = m_amDbusDockInter->position();
    if (m_amDbusDockInter->displayMode() == DLauncher::DOCK_FASHION)
        dockSpacing = 8;
#else
    int dockPos = m_dockInter->position();
    if (m_dockInter->displayMode() == DLauncher::DOCK_FASHION)
        dockSpacing = 8;
#endif

    QPoint launcherPoint;
    switch (dockPos) {
    case DLauncher::DOCK_TOP:
        launcherPoint = QPoint(dockRect.left(), dockRect.bottom() + dockSpacing + 1);
        break;
    case DLauncher::DOCK_BOTTOM:
        launcherPoint = QPoint(dockRect.left(), dockRect.top() - height() - dockSpacing);
        break;
    case DLauncher::DOCK_LEFT:
        launcherPoint = QPoint(dockRect.right() + dockSpacing + 1, dockRect.top());
        break;
    case DLauncher::DOCK_RIGHT:
        launcherPoint = QPoint(dockRect.left() - width() - dockSpacing, dockRect.top());
        break;
    default:
        Q_UNREACHABLE_IMPL();
    }

    initAnchoredCornor();
    move(launcherPoint);
}

void WindowedFrame::onHideMenu()
{
    if (m_menuWorker.get() && !isVisible())
        m_menuWorker.get()->onHideMenu();
}

void WindowedFrame::addViewEvent(AppGridView *pView)
{
    connect(pView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(pView, &AppGridView::clicked, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(pView, &AppGridView::entered, m_appsView, &AppListView::setCurrentIndex, Qt::QueuedConnection);
    connect(pView, &AppGridView::popupMenuRequested, m_menuWorker.get(), &MenuWorker::showMenuByAppItem);
}

void WindowedFrame::refreshView(const AppsListModel::AppCategory category)
{
    switch (category) {
    case AppsListModel::Collect:
        m_collectionModel->clearDraggingIndex();
        m_collectionView->update();
        break;
    default:
        break;
    }
}

void WindowedFrame::onButtonClick(int buttonid)
{
    m_appsModel->setCategory((buttonid == 1) ? AppsListModel::TitleMode : AppsListModel::LetterMode);
    m_appsView->setCurrentIndex(QModelIndex());
}

void WindowedFrame:: paintEvent(QPaintEvent *e)
{
    DBlurEffectWidget::paintEvent(e);

    // 由于主窗口会根据控制中心设置的透明度改变而变化,
    // 所有左侧工具栏背景绘制应该在左侧工具中
    // 不然绘制出的背景会重新附上透明度达不到想要的效果
}
