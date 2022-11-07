// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowedframe.h"
#include "widgets/hseparator.h"
#include "global_util/util.h"
#include "dbusdockinterface.h"
#include "constants.h"
#include "iconcachemanager.h"

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

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

#define DOCK_FASHION    0
#define DOCK_EFFICIENT  1

DGUI_USE_NAMESPACE

using namespace DLauncher;

extern const QPoint widgetRelativeOffset(const QWidget *const self, const QWidget *w);

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
    , m_dockInter(new DBusDock(this))
    , m_menuWorker(new MenuWorker(this))
    , m_eventFilter(new SharedEventFilter(this))
    , m_windowHandle(this, this)
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_maskBg(new QWidget(this))
    , m_appsManager(AppsManager::instance())
    , m_appsView(new AppListView(this))
    , m_appsModel(new AppsListModel(AppsListModel::Custom, this))
    , m_searchModel(new AppsListModel(AppsListModel::Search, this))
    , m_leftBar(new MiniFrameRightBar(this))
    , m_switchBtn(new MiniFrameSwitchBtn(this))
    , m_tipsLabel(new QLabel(this))
    , m_delayHideTimer(new QTimer(this))
    , m_autoScrollTimer(new QTimer(this))
    , m_appearanceInter(new Appearance("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
    , m_displayMode(All)
    , m_calcUtil(CalculateUtil::instance())
    , m_focusPos(Applist)
    , m_modeToggleBtn(new ModeToggleButton(this))
    , m_searcherEdit(new DSearchEdit(this))
    , m_enterSearchEdit(false)
    , m_dockFrontInfc(new DBusDockInterface(this))
    , m_currentAppListIndex(m_appsView->currentIndex().row())
{
    if (!getDConfigValue("enableFullScreenMode", true).toBool())
        m_modeToggleBtn->hide();

    m_searcherEdit->setAccessibleName("searcherEdit");
    m_maskBg->setAccessibleName("MaskBg");
    m_switchBtn->setAccessibleName("switchBtn");
    m_modeToggleBtn->setAccessibleName("modeToggleBtn");
    m_tipsLabel->setAccessibleName("tipsLabel");

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

    m_appsView->setModel(m_appsModel);
    m_appsView->setItemDelegate(new AppListDelegate(this));

    m_appsView->installEventFilter(m_eventFilter);
    m_switchBtn->installEventFilter(m_eventFilter);
    m_switchBtn->setFocusPolicy(Qt::NoFocus);

    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(500, 50);
    m_tipsLabel->setVisible(false);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &WindowedFrame::resetWidgetStyle);

    m_delayHideTimer->setInterval(200);
    m_delayHideTimer->setSingleShot(true);

    m_autoScrollTimer->setInterval(DLauncher::APPS_AREA_AUTO_SCROLL_TIMER);
    m_autoScrollTimer->setSingleShot(false);

    m_leftBar->installEventFilter(m_eventFilter);
    m_leftBar->installEventFilter(this);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(9, 0, DLauncher::MINI_FRAME_LAYOUT_SPACE2, 0);
    searchLayout->addWidget(m_searcherEdit);
    searchLayout->setSpacing(6);
    searchLayout->addWidget(m_modeToggleBtn);

    QHBoxLayout *appsLayout = new QHBoxLayout;
    appsLayout->setContentsMargins(DLauncher::MINI_FRAME_LAYOUT_SPACE1, 0, 0, 0);
    appsLayout->addWidget(m_appsView);

    QHBoxLayout *switchLayout = new QHBoxLayout;
    switchLayout->setContentsMargins(DLauncher::MINI_FRAME_LAYOUT_SPACE1, 0, 11, 0);
    switchLayout->addWidget(m_switchBtn);

    QVBoxLayout *containLayout = new QVBoxLayout;
    containLayout->setSpacing(0);
    containLayout->setMargin(0);

    containLayout->setContentsMargins(0, 7, 0, 15);
    containLayout->addLayout(searchLayout);
    //containLayout->addWidget(new HSeparator);
    containLayout->addSpacing(6);
    containLayout->addLayout(appsLayout);
    containLayout->addLayout(switchLayout);

    m_rightWidget = new QWidget(this);
    m_rightWidget->setContentsMargins(0, 0, 0, 0);
    m_rightWidget->setLayout(containLayout);
    m_rightWidget->setFixedWidth(320);
    m_rightWidget->setAccessibleName("rightWidget");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_leftBar);
    mainLayout->addWidget(m_rightWidget);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::ClickFocus);
    setFixedHeight(538);
    setObjectName("MiniFrame");

    create();
    if (windowHandle()) {
        windowHandle()->setProperty("_d_dwayland_window-type", "launcher");
    }

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

    connect(m_leftBar, &MiniFrameRightBar::requestFrameHide, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &WindowedFrame::onWMCompositeChanged);
    connect(m_searcherEdit, &DSearchEdit::textChanged, this, &WindowedFrame::searchText, Qt::QueuedConnection);
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (WindowedFrame::*)(const QModelIndex &)>(&WindowedFrame::uninstallApp));
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_delayHideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_menuWorker.get(), &MenuWorker::notifyMenuDisplayState, m_appsView, &AppListView::setMenuVisible);

    connect(m_appsView, &QListView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_appsView, &QListView::clicked, this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_appsView, &QListView::entered, m_appsView, &AppListView::setCurrentIndex, Qt::QueuedConnection);
    connect(m_appsView, &AppListView::popupMenuRequested, m_menuWorker.get(), &MenuWorker::showMenuByAppItem);
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_appsView, &AppListView::menuHide); // 当菜单消失时通知菜单结束了
    connect(m_appsView, &AppListView::requestSwitchToCategory, this, &WindowedFrame::switchToCategory);
    connect(m_appsView, &AppListView::requestEnter, m_searchModel, [this](bool state) {
        if (!m_menuWorker.get()->isMenuVisible() && state) {
            m_appsModel->setDrawBackground(state);
        }
    });
    connect(m_appsView, &AppListView::requestEnter, m_menuWorker.get(), [this](bool state) {
        if (!m_menuWorker.get()->isMenuVisible() && state) {
            m_appsModel->setDrawBackground(state);
        }
    });

    connect(m_appsManager, &AppsManager::requestTips, this, &WindowedFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &WindowedFrame::hideTips);
    connect(m_switchBtn, &MiniFrameSwitchBtn::clicked, this, &WindowedFrame::onSwitchBtnClicked);
    connect(m_delayHideTimer, &QTimer::timeout, this, &WindowedFrame::prepareHideLauncher, Qt::QueuedConnection);

    connect(m_appearanceInter, &Appearance::OpacityChanged, this, &WindowedFrame::onOpacityChanged);
    connect(m_modeToggleBtn, &DToolButton::clicked, this, &WindowedFrame::onToggleFullScreen);

    QTimer::singleShot(1, this, &WindowedFrame::onWMCompositeChanged);
    onOpacityChanged(m_appearanceInter->opacity());

    connect(m_dockFrontInfc, &DBusDockInterface::geometryChanged, this, &WindowedFrame::onDockGeometryChanged);

    connect(this, &WindowedFrame::visibleChanged, this, &WindowedFrame::onHideMenu);

    // 状态切换
    m_switchBtn->updateStatus(All);
    m_modeToggleBtn->setIconSize(QSize(32,32));
    m_modeToggleBtn->setFixedSize(40, 40);
    m_modeToggleBtn->setFocusPolicy(Qt::NoFocus);

    // 重置控件样式
    resetWidgetStyle();
    m_maskBg->setAutoFillBackground(true);
    m_maskBg->setFixedSize(size());
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

    // 初始化所有应用数据
    if (!m_appsManager->isVaild())
        m_appsManager->refreshAllList();

    m_appsView->setCurrentIndex(QModelIndex());

    adjustSize();

    // 启动器跟随任务栏位置
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
    m_appsView->setMenuVisible(false);
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
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(0);
            break;
        case Computer:
            m_focusPos = Setting;
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(6);
            break;
        case Setting:
            m_focusPos = Power;
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(7);
            break;
        case Power:
            m_focusPos = Search;
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentCheck(false);
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
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(0);
            break;
        case Power:
            m_focusPos = Setting;
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(6);
            break;
        case Search:
            m_focusPos = Power;
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(7);
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
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(0);
        } else {
            m_leftBar->moveUp();
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
            m_leftBar->hideAllHoverState();
            m_leftBar->setCurrentIndex(0);
        } else {
            m_leftBar->moveDown();
        }
        break;
    }
    case Qt::Key_Left: {
        if (m_focusPos == Search || m_focusPos == Applist || m_focusPos == RightBottom || m_focusPos == Default) {
            m_focusPos = Applist;
            m_focusPos  = Computer;
            m_leftBar->setCurrentIndex(0);
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
        m_searchModel->setDrawBackground(true);
        m_leftBar->setCurrentCheck(false);
        m_appsView->setFocus();
    } else if (m_focusPos == RightBottom) {
        m_appsView->setCurrentIndex(QModelIndex());
        m_leftBar->setCurrentCheck(false);
        m_switchBtn->setFocus();
        return;
    } else if (m_focusPos == Search) {
        m_leftBar->setCurrentCheck(false);
    } else {
        m_appsView->setCurrentIndex(QModelIndex());
        m_leftBar->setCurrentCheck(true);
        m_leftBar->setFocus();
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
        m_leftBar->execCurrent();
        return;
    } else if (m_focusPos == RightBottom && !currentIdx.isValid()) {
        m_switchBtn->click();
        return;
    }

    if (m_displayMode == Category && m_appsModel->category() == AppsListModel::Category) {
        switchToCategory(currentIdx);
        return;
    }

    if (currentIdx.isValid() && currentIdx.model() == m_appsView->model()) {
        m_appsManager->launchApp(currentIdx);
    } else {
        if (currentIdx.isValid()) {
            m_appsManager->launchApp(m_appsView->model()->index(0, 0));
        } else {
            qWarning() << "Curren index is not valid";
            return;
        }
    }

    hideLauncher();
}

void WindowedFrame::uninstallApp(const QString &appKey)
{
    uninstallApp(m_appsModel->indexAt(appKey));
}

void WindowedFrame::uninstallApp(const QModelIndex &context)
{
    // TODO 这个变量后面如果图标不用线程加载的话应该也不需要了
    static bool UNINSTALL_DIALOG_SHOWN = false;

    if (UNINSTALL_DIALOG_SHOWN) {
        return;
    }

    UNINSTALL_DIALOG_SHOWN = true;
    DTK_WIDGET_NAMESPACE::DDialog unInstallDialog;
    unInstallDialog.setAccessibleName("unInstallDialog");
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    const QString appKey = context.data(AppsListModel::AppKeyRole).toString();
    unInstallDialog.setTitle(QString(tr("Are you sure you want to uninstall it?")));

    QPixmap pixmap = context.data(AppsListModel::AppDialogIconRole).value<QPixmap>();

    int size = (pixmap.size() / qApp->devicePixelRatio()).width();
    ItemInfo item = context.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo>();

    QPair<QString, int> tmpKey { cacheKey(item), size };

    // 命令行安装应用后，卸载应用的确认弹框偶现左上角图标呈齿轮的情况
    QPixmap appIcon;
    if (IconCacheManager::existInCache(tmpKey)) {
        IconCacheManager::getPixFromCache(tmpKey, appIcon);
        unInstallDialog.setIcon(appIcon);
    } else {
        static int tryNum = 0;
        UNINSTALL_DIALOG_SHOWN = false;
        ++tryNum;
        if (tryNum <= 5) {
            QTimer::singleShot(100, this, [ = ]() { uninstallApp(context); });
            return;
        } else {
            QIcon icon = QIcon(":/widgets/images/application-x-desktop.svg");
            appIcon = icon.pixmap(QSize(size, size));
            unInstallDialog.setIcon(appIcon);
        }
    }

    unInstallDialog.addButton(tr("Cancel"));
    unInstallDialog.addButton(tr("Uninstall"), false, DDialog::ButtonWarning);

    connect(&unInstallDialog, &DTK_WIDGET_NAMESPACE::DDialog::buttonClicked, [&](int clickedResult) {
        // 0 means "cancel" button clicked
        if (clickedResult == 0) {
            return;
        }

        m_appsManager->uninstallApp(appKey);
    });

    // hide frame
    QMetaObject::invokeMethod(this, &WindowedFrame::hideLauncher, Qt::QueuedConnection);

    unInstallDialog.exec();
    UNINSTALL_DIALOG_SHOWN = false;
}

/**分类模式下，进入到某一分类目录的处理
 * @brief WindowedFrame::switchToCategory
 * @param index 当前视图的模型索引
 */
void WindowedFrame::switchToCategory(const QModelIndex &index)
{
    m_focusPos = Applist;
    m_appsView->setModel(m_appsModel);
    m_appsView->setCurrentIndex(QModelIndex());
    m_appsModel->setCategory(index.data(AppsListModel::AppCategoryRole).value<AppsListModel::AppCategory>());
}

QPainterPath WindowedFrame::getCornerPath(AnchoredCornor direction)
{
    QPainterPath path;
    if (m_dockInter->displayMode() == DOCK_FASHION) {
       return path;
    }
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

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        palette.setColor(QPalette::Background, QColor(0, 0, 0, 0.3 * 255));
    else
        palette.setColor(QPalette::Background, QColor(255, 255, 255, 0.3 * 255));

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
    m_appsModel->setDrawBackground(false);
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
//    switch (prop) {
//    case Qt::ImEnabled:
//        return true;
//    case Qt::ImCursorRectangle:
//        return widgetRelativeOffset(this, m_searchWidget);
//    default: ;
//    }

    return QWidget::inputMethodQuery(prop);
}

void WindowedFrame::regionMonitorPoint(const QPoint &point, int flag)
{
    Q_UNUSED(flag);

    if (isWaylandDisplay()) {
        return;
    }

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
    if (watched == m_leftBar && event->type() == QEvent::Resize) {
        setFixedSize(m_rightWidget->width() + m_leftBar->width(), 538);
    }

    if (watched == m_appsView) {
        if (event->type() == QEvent::Wheel) {
            m_searcherEdit->lineEdit()->clearFocus();
        } else if (event->type() == QEvent::Leave && !m_menuWorker->isMenuVisible()) {
            // 菜单未显示时鼠标移出列表区域，取消应用选中状态，显示菜单时，移动鼠标菜单保持显示并且左侧图标不显示选中状态
            m_appsModel->setDrawBackground(false);
            m_searchModel->setDrawBackground(false);
        }
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

    // 第一次弹出菜单时会将界面设置为ApplicationDeactivate状态，导致再次弹出菜单时会满足条件从而隐藏界面
    // 此时需要判断下是否因为弹出菜单导致的界面ApplicationDeactivate状态
    if (isActiveWindow() && watched == qApp && event->type() == QEvent::ApplicationDeactivate && !m_menuWorker->isMenuShown()) {
        hideLauncher();
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
    if (m_wmHelper->hasComposite()) {
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

    update();
}

void WindowedFrame::adjustPosition()
{
    QRect r =  m_dockInter->frontendRect();
    QRect dockRect = QRect(scaledPosition(r.topLeft()),scaledPosition(r.bottomRight()));

    // 启动器高效模式和时尚模式与任务栏的间隙不同
    int dockSpacing = 0;
    if (m_dockInter->displayMode() == DOCK_FASHION)
        dockSpacing = 8;

    QPoint p;
    switch (m_dockInter->position()) {
    case DOCK_TOP:
        p = QPoint(dockRect.left(), dockRect.bottom() + dockSpacing + 1);
        break;
    case DOCK_BOTTOM:
        p = QPoint(dockRect.left(), dockRect.top() - height() - dockSpacing);
        break;
    case DOCK_LEFT:
        p = QPoint(dockRect.right() + dockSpacing + 1, dockRect.top());
        break;
    case DOCK_RIGHT:
        p = QPoint(dockRect.left() - width() - dockSpacing, dockRect.top());
        break;
    default:
        Q_UNREACHABLE_IMPL();
    }

    initAnchoredCornor();
    move(p);
}

void WindowedFrame::onToggleFullScreen()
{
    // 后台没有启动,切换全屏会造成后台默认数据和前台数据不统一,造成显示问题
    if (!m_appsManager->appNums(AppsListModel::All))
        return;

    // 切换到全屏就不绘制小窗口列表中的item
    AppListDelegate *delegate = static_cast<AppListDelegate *>(m_appsView->itemDelegate());
    if (delegate)
        delegate->setActived(false);

    m_modeToggleBtn->setHovered(false);

    // 全屏状态标识
    m_calcUtil->setFullScreen(true);

    // 同步本地当前是全屏的状态
    DDBusSender()
    .service("com.deepin.dde.daemon.Launcher")
    .interface("com.deepin.dde.daemon.Launcher")
    .path("/com/deepin/dde/daemon/Launcher")
    .property("Fullscreen")
    .set(true);
}

void WindowedFrame::onSwitchBtnClicked()
{
    if (m_displayMode == All) {
        m_appsModel->setCategory(AppsListModel::Category);
        m_displayMode = Category;
        m_appsView->setCurrentIndex(QModelIndex());
        m_focusPos = RightBottom;
    } else if (m_displayMode == Category && m_appsModel->category() != AppsListModel::Category) {
        m_appsModel->setCategory(AppsListModel::Category);
    } else {
        m_displayMode = All;
        m_appsModel->setCategory(AppsListModel::Custom);
    }

    m_switchBtn->updateStatus(m_displayMode);
    m_appsView->setModel(m_appsModel);

    // each time press "switch btn" must hide tips label.
    hideTips();
    m_searcherEdit->lineEdit()->clear();
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
    m_searcherEdit->lineEdit()->setFocus();
    if (text.isEmpty()) {
        m_appsView->setModel(m_appsModel);
        hideTips();
        m_switchBtn->setVisible(true);
        m_displayMode = Category;
    } else {
        m_switchBtn->setVisible(false);
        if (m_appsView->model() != m_searchModel) {
            m_appsView->setModel(m_searchModel);
            m_searchModel->setDrawBackground(true);
            m_focusPos = Search;
        }
        auto temp = text;
        m_appsManager->searchApp(temp.replace(" ",""));
        m_displayMode = All;
    }
}

void WindowedFrame::showTips(const QString &text)
{
    if (m_appsView->model() != m_searchModel)
        return;

    m_tipsLabel->setText(text);

    const QPoint center = m_appsView->rect().center() - m_tipsLabel->rect().center()*0.75;
    m_tipsLabel->move(center);
    m_tipsLabel->setVisible(true);
    m_tipsLabel->raise();
}

void WindowedFrame::hideTips()
{
    if (m_searchModel == m_appsView->model() && m_appsView->count())
        m_appsView->setCurrentIndex(m_appsView->indexAt(0));

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
    // recovery list view
    m_displayMode = All;
    m_appsModel->setCategory(AppsListModel::Custom);
    m_appsView->setModel(m_appsModel);

    // recovery switch button
    m_switchBtn->show();
    m_switchBtn->updateStatus(All);
    hideTips();

    m_focusPos = Applist;
    m_leftBar->setCurrentCheck(false);
}

void WindowedFrame::onOpacityChanged(const double value)
{
    setMaskAlpha(value * 255);
}

void WindowedFrame::onDockGeometryChanged()
{
    adjustSize();
    updatePosition();
    m_cornerPath = getCornerPath(m_anchoredCornor);
    m_windowHandle.setClipPath(m_cornerPath);
}

void WindowedFrame::updatePosition()
{
    if (!m_dockFrontInfc || !m_dockInter)
        return;

    QRect dockRect = m_dockFrontInfc->geometry();

    int dockSpacing = 0;
    if (m_dockInter->displayMode() == DOCK_FASHION)
        dockSpacing = 8;

    QPoint p;
    switch (m_dockInter->position()) {
    case DOCK_TOP:
        p = QPoint(dockRect.left(), dockRect.bottom() + dockSpacing + 1);
        break;
    case DOCK_BOTTOM:
        p = QPoint(dockRect.left(), dockRect.top() - height() - dockSpacing);
        break;
    case DOCK_LEFT:
        p = QPoint(dockRect.right() + dockSpacing + 1, dockRect.top());
        break;
    case DOCK_RIGHT:
        p = QPoint(dockRect.left() - width() - dockSpacing, dockRect.top());
        break;
    default:
        Q_UNREACHABLE_IMPL();
    }

    initAnchoredCornor();
    move(p);
}

void WindowedFrame::onHideMenu()
{
    if (m_menuWorker.get() && !isVisible())
        m_menuWorker.get()->onHideMenu();
    m_currentAppListIndex = -1;
}

void WindowedFrame:: paintEvent(QPaintEvent *e)
{
    DBlurEffectWidget::paintEvent(e);

    // 由于主窗口会根据控制中心设置的透明度改变而变化,
    // 所有左侧工具栏背景绘制应该在左侧工具中
    // 不然绘制出的背景会重新附上透明度达不到想要的效果
}
