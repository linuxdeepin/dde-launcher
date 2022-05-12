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

#include "fullscreenframe.h"
#include "constants.h"
#include "xcb_misc.h"
#include "sharedeventfilter.h"
#include "constants.h"
#include "iconcachemanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QClipboard>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>
#include <QScrollBar>
#include <QKeyEvent>
#include <QProcess>
#include <QScroller>

#include <DWindowManagerHelper>
#include <DDBusSender>
#include <DDialog>
#include <DGuiApplicationHelper>
#include <DPlatformWindowHandle>

DGUI_USE_NAMESPACE

static const QString WallpaperKey = "pictureUri";
static const QString DisplayModeKey = "display-mode";
static const QString DisplayModeFree = "free";
static const QString DisplayModeCategory = "category";

const QPoint widgetRelativeOffset(const QWidget *const self, const QWidget *w)
{
    QPoint offset;
    while (w && w != self) {
        offset += w->pos();
        w = qobject_cast<QWidget *>(w->parent());
    }

    return offset;
}

/**
 * @brief FullScreenFrame::FullScreenFrame
 * 全屏模式下的界面类
 * @param parent
 */
FullScreenFrame::FullScreenFrame(QWidget *parent)
    : BoxFrame(parent)
    , m_menuWorker(new MenuWorker(this))
    , m_eventFilter(new SharedEventFilter(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_appsManager(AppsManager::instance())
    , m_delayHideTimer(new QTimer(this))
    , m_clearCacheTimer(new QTimer(this))
    , m_searchWidget(new SearchWidget(this))
    , m_contentFrame(new QFrame(this))
    , m_appsIconBox(new DHBoxWidget(m_contentFrame))
    , m_appsScrollArea(new QScrollArea(this))
    , m_appItemLayout(new QHBoxLayout(this))
    , m_appItemWidget(new QWidget(this))
    , m_tipsLabel(new QLabel(this))
    , m_appItemDelegate(new AppItemDelegate(this))
    , m_multiPagesView(new MultiPagesView(AppsListModel::All, this))
    , m_internetMultiPagesView(new MultiPagesView(AppsListModel::Internet, this))
    , m_chatMultiPagesView(new MultiPagesView(AppsListModel::Chat, this))
    , m_musicMultiPagesView(new MultiPagesView(AppsListModel::Music, this))
    , m_videoMultiPagesView(new MultiPagesView(AppsListModel::Video, this))
    , m_graphicsMultiPagesView(new MultiPagesView(AppsListModel::Graphics, this))
    , m_gameMultiPagesView(new MultiPagesView(AppsListModel::Game, this))
    , m_officeMultiPagesView(new MultiPagesView(AppsListModel::Office, this))
    , m_readingMultiPagesView(new MultiPagesView(AppsListModel::Reading, this))
    , m_developMultiPagesView(new MultiPagesView(AppsListModel::Development, this))
    , m_systemMultiPagesView(new MultiPagesView(AppsListModel::System, this))
    , m_othersMultiPagesView(new MultiPagesView(AppsListModel::Others, this))
    , m_topSpacing(new QFrame(this))
    , m_bottomSpacing(new QFrame(this))
    , m_focusIndex(0)
    , m_mousePressSeconds(0)
//    , m_animationGroup(new ScrollParallelAnimationGroup(this))
    , m_mousePressState(false)
    , m_bMousePress(false)
    , m_nMousePos(0)
    , m_scrollValue(0)
    , m_scrollStart(0)
    , m_curScreen(m_appsManager->currentScreen())
    , m_dirWidget(new AppDirWidget(this))
{
    initAccessibleName();
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_InputMethodEnabled, true);

    create();
    if (windowHandle()) {
        windowHandle()->setProperty("_d_dwayland_window-type", "launcher");
    }

#if (DTK_VERSION <= DTK_VERSION_CHECK(2, 0, 9, 9))
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
#else
    auto compositeChanged = [ = ] {
        if (DWindowManagerHelper::instance()->windowManagerName() == DWindowManagerHelper::WMName::KWinWM)
            setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
        else
            setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
    };

    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, compositeChanged);
    compositeChanged();
#endif

    // 全局键盘按键事件处理、搜索框字符输入处理
    installEventFilter(m_eventFilter);

    initConnection();
    initUI();
}

FullScreenFrame::~FullScreenFrame()
{
}

void FullScreenFrame::exit()
{
    qApp->quit();
}

int FullScreenFrame::dockPosition()
{
    return m_appsManager->dockPosition();
}

/**
 * @brief FullScreenFrame::addViewEvent
 * 处理全屏模式下视图中的鼠标事件
  * @param pView 当前列表视图
 */
void FullScreenFrame::addViewEvent(AppGridView *pView)
{
    connect(pView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(pView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(pView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(pView, &AppGridView::clicked, this, &FullScreenFrame::hide);

    connect(m_appsManager, &AppsManager::dataChanged, [ & ](AppsListModel::AppCategory category) {
        if (category == AppsListModel::Dir)
            m_dirWidget->update();
    });
    connect(pView, &AppGridView::clicked, [ & ](const QModelIndex &index) {
        if (!index.isValid())
            return;

        bool isDir = index.data(AppsListModel::ItemIsDirRole).toBool();
        if (!isDir) {
            m_appsManager->launchApp(index);
            this->hide();
        } else {
            m_appsManager->setDirAppInfoList(index);
            m_dirWidget->show();
        }
    });
    connect(pView, &AppGridView::requestMouseRelease,this,  [ = ]() {
            m_mousePressState = false;
    });
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, pView, qOverload<>(&AppGridView::update));
}

void FullScreenFrame::onHideMenu()
{
    if (m_menuWorker.get() && !isVisible())
        m_menuWorker->onHideMenu();
}

void FullScreenFrame::showTips(const QString &tips)
{
    if (m_displayMode != SEARCH)
        return;

    m_tipsLabel->setText(tips);
    QFont font(m_tipsLabel->font());
    font.setPointSize(30);
    QColor color(255, 255, 255, 0.5 * 255);
    QPalette palette(m_tipsLabel->palette());
    palette.setColor(QPalette::WindowText, color);
    m_tipsLabel->setPalette(palette);
    m_tipsLabel->setFont(font);

    // 根据文字内容的边界矩形设置label大小, 避免在葡萄牙语等语言下内容被截断而显示不全
    QFontMetricsF fontMetric(m_tipsLabel->font());
    int width = qCeil(fontMetric.boundingRect(m_tipsLabel->text()).width());
    int height = qCeil(fontMetric.boundingRect(m_tipsLabel->text()).height());

    m_tipsLabel->setFixedSize(width, height);

    const QPoint center = rect().center() - m_tipsLabel->rect().center();
    m_tipsLabel->move(center);
    m_tipsLabel->setVisible(true);
    m_tipsLabel->raise();
}

void FullScreenFrame::hideTips()
{
    m_tipsLabel->setVisible(false);
}

void FullScreenFrame::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Minus) {
        if (!e->modifiers().testFlag(Qt::ControlModifier))
            return;

        e->accept();
        if (m_calcUtil->decreaseIconSize())
            emit m_appsManager->layoutChanged(AppsListModel::All);
    } else if (e->key() == Qt::Key_Equal) {
        if (!e->modifiers().testFlag(Qt::ControlModifier))
            return;
        e->accept();
        if (m_calcUtil->increaseIconSize())
            emit m_appsManager->layoutChanged(AppsListModel::All);
    } else if (e->key() == Qt::Key_V &&
               e->modifiers().testFlag(Qt::ControlModifier)) {
        const QString &clipboardText = QApplication::clipboard()->text();

        // support Ctrl+V shortcuts.
        if (!clipboardText.isEmpty()) {
            m_searchWidget->edit()->lineEdit()->setText(clipboardText);
            m_searchWidget->edit()->lineEdit()->setFocus();
            m_focusIndex = SearchEdit;
        }
    }
}

void FullScreenFrame::showEvent(QShowEvent *e)
{
    // 显示后加载当前模式其他ratio的资源，预加载全屏另一种模式当前ratio的资源
    if (m_calcUtil->displayMode() == GROUP_BY_CATEGORY) {
        emit m_appsManager->loadOtherRatioIcon(GROUP_BY_CATEGORY);
        emit m_appsManager->loadCurRationIcon(ALL_APPS);
    } else {
        emit m_appsManager->loadOtherRatioIcon(ALL_APPS);
        emit m_appsManager->loadCurRationIcon(GROUP_BY_CATEGORY);
    }

    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();

    if (QApplication::platformName() != "wayland")
        XcbMisc::instance()->set_deepin_override(winId());

    if (!m_appsManager->isVaild())
        m_appsManager->refreshAllList();

    updateDockPosition();

    QFrame::showEvent(e);

    QTimer::singleShot(0, this, [ this ]() {
        raise();
        activateWindow();
        m_searchWidget->raise();
        emit visibleChanged(true);
    });

    m_clearCacheTimer->stop();

    m_canResizeDockPosition = true;
}

void FullScreenFrame::hideEvent(QHideEvent *e)
{
    BoxFrame::hideEvent(e);

    QTimer::singleShot(1, this, [ = ] { emit visibleChanged(false); });
}

void FullScreenFrame::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    m_searchWidget->clearSearchContent();
    m_mousePressState = true;
    m_mousePressSeconds =  QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_mouseMovePos = e->pos();
    m_mousePressPos = e->pos();

    m_startPoint = e->globalPos();

    // 全屏模式下支持全屏范围滑动翻页
    mousePressDrag(e);
}

void FullScreenFrame::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_mousePressState || e->button() == Qt::RightButton)
        return;

    int categoryCount = m_appsManager->getVisibleCategoryCount();

    if (categoryCount <= 2)
        return;

    qint64 mouseReleaseSeconds =  QDateTime::currentDateTime().toMSecsSinceEpoch();
    int horizontalXOffset = e->pos().x() - m_mousePressPos.x();

// TODO: 优化点
//    if (mouseReleaseSeconds - m_mousePressSeconds > DLauncher::MOUSE_PRESS_TIME_DIFF) {
//        if (horizontalXOffset < 0)
//            m_animationGroup->setScrollType(Scroll_Next);
//        else
//            m_animationGroup->setScrollType(Scroll_Prev);
//    }

    m_mouseMovePos = e->pos();

    // 全屏模式下支持全屏范围滑动翻页
    mouseMoveDrag(e);
}

void FullScreenFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !m_mousePressState)
        return;

    int diff_x = qAbs(e->pos().x() - m_mousePressPos.x());
    int diff_y = qAbs(e->pos().y() - m_mousePressPos.y());
    // 小范围位置变化，当作没有变化，针对触摸屏
    if ((e->source() == Qt::MouseEventSynthesizedByQt && diff_x < DLauncher::TOUCH_DIFF_THRESH && diff_y < DLauncher::TOUCH_DIFF_THRESH)
            || (e->source() != Qt::MouseEventSynthesizedByQt && e->pos() == m_mousePressPos )) {
        hide();
    }
    m_mousePressState = false;

    // 全屏分类模式才支持鼠标拖动触发分页的操作，鼠标小范围移动不触发分页
    if ((m_calcUtil->displayMode() != ALL_APPS) || (abs(e->globalX() - m_startPoint.x()) < DLauncher::SLIDE_DIFF_THRESH))
        return;

    // 全屏模式下支持全屏范围滑动翻页
    mouseReleaseDrag(e);
}

void FullScreenFrame::wheelEvent(QWheelEvent *e)
{
    if (m_displayMode == GROUP_BY_CATEGORY) {
        int pageStep = 200;
        int value = m_appsScrollArea->horizontalScrollBar()->value();
        if (e->angleDelta().y() < 0 ||  e-> angleDelta().x() < 0)
           m_appsScrollArea->horizontalScrollBar()->setValue(value + pageStep);
        else
           m_appsScrollArea->horizontalScrollBar()->setValue(value - pageStep);
    }
    QFrame::wheelEvent(e);
}

bool FullScreenFrame::eventFilter(QObject *o, QEvent *e)
{
    if (m_displayMode == GROUP_BY_CATEGORY && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyPress = static_cast<QKeyEvent *>(e);
        if (keyPress && keyPress->key() == Qt::Key_Tab) {
        }
    }

    // we filter some key events from LineEdit, to implements cursor move.
    if (o == m_searchWidget->edit()->lineEdit() && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyPress = static_cast<QKeyEvent *>(e);
        if (keyPress->key() == Qt::Key_Left || keyPress->key() == Qt::Key_Right) {
            QKeyEvent *event = new QKeyEvent(keyPress->type(), keyPress->key(), keyPress->modifiers());
            qApp->postEvent(this, event);
            return true;
        } else if (keyPress->key() == Qt::Key_Tab) {
            m_focusIndex = SearchEdit;
            moveCurrentSelectApp(Qt::Key_Tab);
            return true;
        }
    } else if (o == m_contentFrame && e->type() == QEvent::Resize && m_canResizeDockPosition) {
        updateDockPosition();
    }

    return false;
}

void FullScreenFrame::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->setText(e->commitString());
        m_searchWidget->edit()->lineEdit()->setFocus();
        m_focusIndex =  SearchEdit;
    }

    QWidget::inputMethodEvent(e);
}

QVariant FullScreenFrame::inputMethodQuery(Qt::InputMethodQuery prop) const
{
    switch (prop) {
    case Qt::ImEnabled:
        return true;
    case Qt::ImCursorRectangle:
        return widgetRelativeOffset(this, m_searchWidget->edit()->lineEdit());
    default:
        break;
    }

    return QWidget::inputMethodQuery(prop);
}

void FullScreenFrame::initUI()
{
    m_appsScrollArea->horizontalScrollBar()->setRange(0, 1920);
    m_dirWidget->setVisible(false);

    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(500, 50);
    m_tipsLabel->setVisible(false);

    m_delayHideTimer->setInterval(500);
    m_delayHideTimer->setSingleShot(true);

    m_clearCacheTimer->setSingleShot(true);
    m_clearCacheTimer->setInterval(DLauncher::CLEAR_CACHE_TIMER * 1000);

    QPalette palette;
    QColor colorButton(255, 255, 255, 0.15 * 255);
    QColor colorText(255, 255, 255);
    palette.setColor(QPalette::Button, colorButton);
    palette.setColor(QPalette::Text, colorText);
    palette.setColor(QPalette::ButtonText, colorText);

    m_searchWidget->edit()->lineEdit()->setPalette(palette);
    m_searchWidget->categoryBtn()->setPalette(palette);
    m_searchWidget->toggleModeBtn()->setPalette(palette);

    m_searchWidget->categoryBtn()->installEventFilter(m_eventFilter);
    m_searchWidget->edit()->lineEdit()->installEventFilter(this);
    m_searchWidget->installEventFilter(m_eventFilter);
    m_searchWidget->showToggle();
    m_appItemDelegate->installEventFilter(m_eventFilter);
    initAppView();

    // 自由排序模式，设置大小调整方式为固定方式
    // 启动时默认按屏幕大小设置自由排序widget的大小
    // 启动时全屏自由模式设置控件大小，解决模式切换界面抖动问题
    const int appsContentWidth = m_calcUtil->getScreenSize().width();
    const int appsContentHeight = m_calcUtil->getScreenSize().height() - DLauncher::APPS_AREA_TOP_MARGIN;

    m_appsIconBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_appsIconBox->layout()->setSpacing(0);
    m_appsIconBox->layout()->addWidget(m_multiPagesView, 0, Qt::AlignCenter);
    m_appsIconBox->setFixedSize(appsContentWidth, appsContentHeight);
    m_multiPagesView->setFixedSize(appsContentWidth, appsContentHeight);

    m_appsScrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    m_appsScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_appsScrollArea->setFixedSize(appsContentWidth, appsContentHeight);
    m_appsScrollArea->setWidgetResizable(true);
    m_appsScrollArea->setWidget(m_appItemWidget);
    m_appsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsScrollArea->setAutoFillBackground(false);
    m_appsScrollArea->setLineWidth(0);
    m_appsScrollArea->horizontalScrollBar()->setRange(0, 1920);
    m_appsScrollArea->horizontalScrollBar()->setPageStep(200);

    m_appItemWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appItemWidget->setAttribute(Qt::WA_TranslucentBackground);

    // todo: 默认给出足够的空间，这里后面需要check下。
    m_appItemWidget->setLayout(m_appItemLayout);
    m_appItemLayout->setContentsMargins(20, 50, 20, 20);

    // 设置搜索控件大小
    QVBoxLayout *scrollVLayout = new QVBoxLayout;
    scrollVLayout->setContentsMargins(0, DLauncher::APPS_AREA_TOP_MARGIN, 0, 0);
    scrollVLayout->setMargin(0);
    scrollVLayout->setSpacing(0);
    scrollVLayout->addWidget(m_appsIconBox, 0, Qt::AlignCenter);
    scrollVLayout->addWidget(m_appsScrollArea, 0, Qt::AlignHCenter);

    m_contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentFrame->setFrameStyle(QFrame::NoFrame);
    m_contentFrame->setLayout(scrollVLayout);
    m_contentFrame->installEventFilter(this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_topSpacing);
    mainLayout->addWidget(m_searchWidget);
    mainLayout->addWidget(m_contentFrame);
    mainLayout->addWidget(m_bottomSpacing);

    setLayout(mainLayout);

    // 获取搜索控件,应用分类导航控件默认大小
    m_calcUtil->setSearchWidgetSizeHint(m_searchWidget->sizeHint());
}

void FullScreenFrame::initAppView()
{
    // 自由
    m_multiPagesView->setDataDelegate(m_appItemDelegate);
    m_multiPagesView->updatePageCount(AppsListModel::All);
    m_multiPagesView->installEventFilter(this);

    m_filterModel->setSourceModel(m_allAppsModel);
    m_filterModel->setFilterRole(AppsListModel::AppRawItemInfoRole);
    m_filterModel->setFilterKeyColumn(0);
    m_filterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    // 分类
    m_internetMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_internetMultiPagesView->updatePageCount(AppsListModel::Internet);
    m_chatMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_chatMultiPagesView->updatePageCount(AppsListModel::Chat);
    m_musicMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_musicMultiPagesView->updatePageCount(AppsListModel::Music);
    m_videoMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_videoMultiPagesView->updatePageCount(AppsListModel::Video);
    m_graphicsMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_graphicsMultiPagesView->updatePageCount(AppsListModel::Graphics);
    m_gameMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_gameMultiPagesView->updatePageCount(AppsListModel::Game);
    m_officeMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_officeMultiPagesView->updatePageCount(AppsListModel::Office);
    m_readingMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_readingMultiPagesView->updatePageCount(AppsListModel::Reading);
    m_developMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_developMultiPagesView->updatePageCount(AppsListModel::Development);
    m_systemMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_systemMultiPagesView->updatePageCount(AppsListModel::System);
    m_othersMultiPagesView->setDataDelegate(m_appItemDelegate);
    m_othersMultiPagesView->updatePageCount(AppsListModel::Others);

    // 偏上
    m_appItemLayout->addWidget(m_internetMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_chatMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_musicMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_videoMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_graphicsMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_gameMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_officeMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_readingMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_developMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_systemMultiPagesView, 0, Qt::AlignTop);
    m_appItemLayout->addWidget(m_othersMultiPagesView, 0, Qt::AlignTop);

    m_appItemLayout->setSpacing(40);
    m_appItemLayout->setMargin(0);
}

void FullScreenFrame::initConnection()
{
    connect(m_multiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);

    connect(m_internetMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_chatMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_musicMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_videoMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_graphicsMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_gameMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_officeMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_readingMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_developMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_systemMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_othersMultiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &FullScreenFrame::layoutChanged, Qt::QueuedConnection);

    connect(this, &FullScreenFrame::displayModeChanged, this, &FullScreenFrame::categoryListChanged);
    connect(this, &FullScreenFrame::visibleChanged, this, &FullScreenFrame::onHideMenu);

    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &FullScreenFrame::searchTextChanged);
    connect(m_delayHideTimer, &QTimer::timeout, this, &FullScreenFrame::hideLauncher, Qt::QueuedConnection);

    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &FullScreenFrame::hideLauncher);
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (FullScreenFrame::*)(const QModelIndex &)>(&FullScreenFrame::uninstallApp));
    connect(m_searchWidget, &SearchWidget::toggleMode, [this] {
        // 显示后加载当前模式其他ratio的资源，预加载全屏另一种模式当前ratio的资源
        if (m_calcUtil->displayMode() == GROUP_BY_CATEGORY) {
            m_appsManager->loadOtherRatioIcon(ALL_APPS);
            m_appsManager->loadCurRationIcon(GROUP_BY_CATEGORY);
        } else {
            m_appsManager->loadOtherRatioIcon(GROUP_BY_CATEGORY);
            m_appsManager->loadCurRationIcon(ALL_APPS);
        }

        updateDisplayMode(m_calcUtil->displayMode() == GROUP_BY_CATEGORY ? ALL_APPS : GROUP_BY_CATEGORY);
    });

    connect(m_appsManager, &AppsManager::categoryListChanged, this, &FullScreenFrame::categoryListChanged);
    connect(m_appsManager, &AppsManager::requestTips, this, &FullScreenFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &FullScreenFrame::hideTips);
    connect(m_appsManager, &AppsManager::IconSizeChanged, this, &FullScreenFrame::updateDockPosition);
    connect(m_appsManager, &AppsManager::dataChanged, this, &FullScreenFrame::refreshPageView);

    connect(m_curScreen, &QScreen::geometryChanged, this, &FullScreenFrame::onScreenInfoChange);
    connect(m_curScreen, &QScreen::orientationChanged, this, &FullScreenFrame::onScreenInfoChange);
    connect(qApp, &QApplication::primaryScreenChanged, this, &FullScreenFrame::onScreenInfoChange);
}

void FullScreenFrame::initAccessibleName()
{
    setObjectName("LauncherFrame");
    setAccessibleName("FullScrreenFrame");
    m_topSpacing->setAccessibleName("topspacing");
    m_bottomSpacing->setAccessibleName("BottomSpacing");
    m_searchWidget->setAccessibleName("searchWidget");
    m_contentFrame->setAccessibleName("ContentFrame");
    m_appsScrollArea->setAccessibleName("appsScrollArea");
    m_tipsLabel->setAccessibleName("tipsLabel");
    m_appItemDelegate->setObjectName("appItemDelegate");

    m_multiPagesView->setAccessibleName("allAppPagesView");
    m_internetMultiPagesView->setAccessibleName("internetBoxWidget");
    m_chatMultiPagesView->setAccessibleName("chatBoxWidget");
    m_musicMultiPagesView->setAccessibleName("musicBoxWidget");
    m_videoMultiPagesView->setAccessibleName("videoBoxWidget");
    m_graphicsMultiPagesView->setAccessibleName("graphicsBoxWidget");
    m_gameMultiPagesView->setAccessibleName("gameBoxWidget");
    m_officeMultiPagesView->setAccessibleName("officeBoxWidget");
    m_readingMultiPagesView->setAccessibleName("readingBoxWidget");
    m_developMultiPagesView->setAccessibleName("developmentBoxWidget");
    m_systemMultiPagesView->setAccessibleName("systemBoxWidget");
    m_othersMultiPagesView->setAccessibleName("othersBoxWidget");

    m_searchWidget->categoryBtn()->setAccessibleName("search_categoryBtn");
    m_searchWidget->edit()->setAccessibleName("FullScreenSearchEdit");
}

void FullScreenFrame::showLauncher()
{
    m_focusIndex = 1;
    m_appItemDelegate->setCurrentIndex(QModelIndex());

    // 启动器跟随任务栏位置
    updateGeometry();

    m_searchWidget->categoryBtn()->clearFocus();
    m_searchWidget->edit()->clearEdit();
    m_searchWidget->edit()->clear();

    updateDisplayMode(m_calcUtil->displayMode());

    m_searchWidget->edit()->lineEdit()->clearFocus();

    setFixedSize(m_appsManager->currentScreen()->geometry().size());
    show();
}

void FullScreenFrame::hideLauncher()
{
    if (!isVisible())
        return;

    m_searchWidget->clearSearchContent();
    hide();
}

bool FullScreenFrame::visible()
{
    return isVisible();
}

void FullScreenFrame::updateGeometry()
{
    QRect rect = m_appsManager->currentScreen()->geometry();

    // 设置启动器显示位置，触发BoxFrame::moveEvent事件．
    setGeometry(rect);

    QFrame::updateGeometry();
}

void FullScreenFrame::moveCurrentSelectApp(const int key)
{
    m_searchWidget->edit()->lineEdit()->clearFocus();
    if (Qt::Key_Tab == key || Qt::Key_Backtab == key) {
        nextTabWidget(key);
        return;
    }

    if (Qt::Key_Undo == key) {
        auto  oldStr =  m_searchWidget->edit()->lineEdit()->text();
        m_searchWidget->edit()->lineEdit()->undo();

        if (!oldStr.isEmpty() && oldStr == m_searchWidget->edit()->lineEdit()->text())
            m_searchWidget->edit()->lineEdit()->clear();

        return;
    }

    if (Qt::Key_Space == key) {
        if (m_searchWidget->categoryBtn()->hasFocus())
            m_searchWidget->categoryBtn()->click();

        return;
    }

    if (m_focusIndex == CategoryTital) {
        switch (key) {
        case Qt::Key_Backtab:
        case Qt::Key_Left:
            return;
        case Qt::Key_Right:
            return;
        case Qt::Key_Down:
            m_focusIndex = FirstItem;
            break;
        default:
            break;
        }
    }

    const QModelIndex curModelIndex = m_appItemDelegate->currentIndex();
    // move operation should be start from a valid location, if not, just init it.
    if (!curModelIndex.isValid()) {
        if (m_displayMode == ALL_APPS) {
            m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(0));
            update();
            return;
        }
    }

    const int column = m_calcUtil->appColumnCount();
    QModelIndex index;

    // calculate destination sibling by keys, it may cause an invalid position.
    switch (key) {
    case Qt::Key_Backtab:
    case Qt::Key_Left:
        index = curModelIndex.sibling(curModelIndex.row() - 1, 0);
        break;
    case Qt::Key_Right:
        index = curModelIndex.sibling(curModelIndex.row() + 1, 0);
        break;
    case Qt::Key_Up:
        index = curModelIndex.sibling(curModelIndex.row() - column, 0);
        break;
    case Qt::Key_Down:
        index = curModelIndex.sibling(curModelIndex.row() + column, 0);
        break;
    default:
        break;
    }

    // valid verify and UI adjustment.
    const QModelIndex selectedIndex = index.isValid() ? index : curModelIndex;
    m_appItemDelegate->setCurrentIndex(selectedIndex);
    update();
}

void FullScreenFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->lineEdit()->setFocus();

    // -1 means backspace key pressed
    if (ch == static_cast<const char>(-1)) {
        m_searchWidget->edit()->lineEdit()->backspace();
        return;
    }

    if (!m_searchWidget->edit()->lineEdit()->selectedText().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->backspace();
    }
    m_focusIndex =  SearchEdit;

    m_searchWidget->edit()->lineEdit()->setText(m_searchWidget->edit()->lineEdit()->text() + ch);
}

void FullScreenFrame::launchCurrentApp()
{
    if (m_searchWidget->edit()->lineEdit()->text().simplified().isEmpty() && ( m_searchWidget->categoryBtn()->hasFocus() ||  m_focusIndex == CategoryChangeBtn)) {
        QMouseEvent btnPress(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(m_searchWidget->categoryBtn(), &btnPress);
        QMouseEvent btnRelease(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(m_searchWidget->categoryBtn(), &btnRelease);
        return;
    }

    const QModelIndex &index = m_appItemDelegate->currentIndex();

    if (index.isValid() && !index.data(AppsListModel::AppDesktopRole).toString().isEmpty()) {
        const AppsListModel::AppCategory category = index.data(AppsListModel::AppGroupRole).value<AppsListModel::AppCategory>();

        if ((category == AppsListModel::All && m_displayMode == ALL_APPS) ||
                (category == AppsListModel::Search && m_displayMode == SEARCH) ||
                (m_displayMode == GROUP_BY_CATEGORY && category != AppsListModel::All && category != AppsListModel::Search)) {
            m_appsManager->launchApp(index);

            hide();
            return;
        }
    }

    switch (m_displayMode) {
    case SEARCH:
    case ALL_APPS:            m_appsManager->launchApp(m_multiPagesView->getAppItem(0));     break;
    }

    hide();
}

void FullScreenFrame::regionMonitorPoint(const QPoint &point, int flag)
{
    QRect dockRect = m_appsManager->dockGeometry();
    QRect visiblableRect = m_menuWorker->menuGeometry();

    if (flag == DLauncher::MOUSE_LEFTBUTTON) {
        // 左键点击时
        if (!m_menuWorker->isMenuShown() && !m_isConfirmDialogShown && !m_delayHideTimer->isActive()) {
            if (dockRect.contains(point)) {
                m_delayHideTimer->start();
            }
        }

        if (m_menuWorker->isMenuShown() && !visiblableRect.contains(point)) {
            m_delayHideTimer->start();
        }
    }
}

/**
 * @brief FullScreenFrame::categoryListChanged 处理全屏分类模式下各个分类逻辑
 */
void FullScreenFrame::categoryListChanged()
{
    if (m_displayMode != GROUP_BY_CATEGORY)
        return ;

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

    m_menuWorker->showMenuByAppItem(pos, context);
}

void FullScreenFrame::uninstallApp(const QString &appKey)
{
    if(m_displayMode != GROUP_BY_CATEGORY){
        int currentPage = m_multiPagesView->currentPage();
        uninstallApp(m_multiPagesView->pageModel(currentPage)->indexAt(appKey));
     }
}

void FullScreenFrame::uninstallApp(const QModelIndex &context)
{
    if (m_isConfirmDialogShown)
        return;

    m_isConfirmDialogShown = true;

    DTK_WIDGET_NAMESPACE::DDialog unInstallDialog;
    unInstallDialog.setWindowState(unInstallDialog.windowState() & ~Qt::WindowFullScreen);
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    const QString appKey = context.data(AppsListModel::AppKeyRole).toString();
    unInstallDialog.setTitle(QString(tr("Are you sure you want to uninstall it?")));
    QPixmap appIcon = context.data(AppsListModel::AppDialogIconRole).value<QPixmap>();
    unInstallDialog.setIcon(appIcon);
    unInstallDialog.setAccessibleName("Imge-unInstallDialog");

    QStringList buttons;
    buttons << tr("Cancel") << tr("Confirm");
    unInstallDialog.addButtons(buttons);

    connect(&unInstallDialog, &DTK_WIDGET_NAMESPACE::DDialog::buttonClicked, [&](int clickedResult) {
        // 0 means "cancel" button clicked
        if (clickedResult == 0)
            return;

        m_appsManager->uninstallApp(appKey, m_displayMode);
    });

    unInstallDialog.exec();
    m_isConfirmDialogShown = false;
}

void FullScreenFrame::refreshPageView(AppsListModel::AppCategory category)
{
    if (AppsListModel::Search == category) {
        m_searchModeWidget->setSearchModel(m_filterModel);
    } else {
        m_multiPagesView->updatePageCount(category);
        m_multiPagesView->showCurrentPage(m_multiPagesView->currentPage());
    }
}

void FullScreenFrame::onScreenInfoChange()
{
    m_curScreen->disconnect();
    m_curScreen = m_appsManager->currentScreen();

    setFixedSize(m_curScreen->size());
    scaledBackground();
    scaledBlurBackground();
    update();

    connect(m_curScreen, &QScreen::geometryChanged, this, &FullScreenFrame::onScreenInfoChange);
    connect(m_curScreen, &QScreen::orientationChanged, this, &FullScreenFrame::onScreenInfoChange);
}

/**
 * @brief FullScreenFrame::updateDisplayMode 处理全屏模式切换
 * @param mode 全屏自由模式或者全屏分类模式
 */
void FullScreenFrame::updateDisplayMode(const int mode)
{
    if (m_displayMode == mode)
        return;

    m_displayMode = mode;

    emit displayModeChanged(m_displayMode);

    // 切换全屏应用列表展现模式, 修改对应的schemaid = com.deepin.dde.launcherd的xml文件
    switch (m_displayMode) {
    case ALL_APPS:
        m_calcUtil->setDisplayMode(ALL_APPS);
        break;
    case GROUP_BY_CATEGORY:
        m_calcUtil->setDisplayMode(GROUP_BY_CATEGORY);
        break;
    default:
        break;
    }

    if (m_displayMode == GROUP_BY_CATEGORY) {
        // 隐藏自由模式显示
        m_appsIconBox->setVisible(false);

        // 隐藏搜索模式
        m_searchModeWidget->setVisible(false);

        // 再显示分类模式
        m_appsScrollArea->setVisible(true);

        // 确认分类模式的应用大小总是相同的， 通过计算设置分类视图的大小
        CalculateUtil::instance()->calculateAppLayout(m_contentFrame->size(), GROUP_BY_CATEGORY);
        layoutChanged();
    } else if (m_displayMode == ALL_APPS) {
        // 隐藏分类模式显示
        m_appsScrollArea->setVisible(false);

        // 隐藏搜索模式
        m_searchModeWidget->setVisible(false);

        // 再显示自由显示模式
        m_appsIconBox->setVisible(true);

        m_multiPagesView->setModel(AppsListModel::All);
        CalculateUtil::instance()->calculateAppLayout(m_contentFrame->size(), ALL_APPS);
        m_multiPagesView->updatePosition(m_displayMode);
        layoutChanged();
    } else {
        // 隐藏自由模式显示
        m_appsIconBox->setVisible(false);
        // 隐藏分类模式
        m_appsScrollArea->setVisible(false);
        layoutChanged();
    }

    m_appItemDelegate->setCurrentIndex(QModelIndex());

    // 搜索模式下的文字描述
    hideTips();
}

/**
 * @brief FullScreenFrame::updateDockPosition 更新任务栏在桌面上的位置
 */
void FullScreenFrame::updateDockPosition()
{
    const QRect dockGeometry = m_appsManager->dockGeometry();

    int bottomMargin = 20;

    m_topSpacing->setFixedHeight(30);
    m_bottomSpacing->setFixedHeight(bottomMargin);

    switch (m_appsManager->dockPosition()) {
    case DLauncher::DOCK_POS_TOP:
        m_topSpacing->setFixedHeight(30 + dockGeometry.height());
        bottomMargin = m_topSpacing->height() + DLauncher::APPS_AREA_TOP_MARGIN;
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(0);
        break;
    case DLauncher::DOCK_POS_BOTTOM:
        bottomMargin += dockGeometry.height();
        m_bottomSpacing->setFixedHeight(bottomMargin);
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(0);
        break;
    case DLauncher::DOCK_POS_LEFT:
        m_searchWidget->setLeftSpacing(dockGeometry.width());
        m_searchWidget->setRightSpacing(0);
        break;
    case DLauncher::DOCK_POS_RIGHT:
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(dockGeometry.width());
        break;
    default:
        break;
    }
    // 全屏自由 + 全屏分类 界面布局没有差异，v23
    m_calcUtil->calculateAppLayout(m_contentFrame->size() - QSize(0, DLauncher::APPS_AREA_TOP_MARGIN), m_displayMode);
}

void FullScreenFrame::nextTabWidget(int key)
{
    if (Qt::Key_Backtab == key) {
        -- m_focusIndex;
        if (m_displayMode == GROUP_BY_CATEGORY) {
            if (m_focusIndex < FirstItem) m_focusIndex = CategoryTital;
        } else {
            if (m_focusIndex < FirstItem) m_focusIndex = CategoryChangeBtn;
        }
    } else if (Qt::Key_Tab == key) {
        ++ m_focusIndex;
        if (m_displayMode == GROUP_BY_CATEGORY) {
            if (m_focusIndex > CategoryTital) m_focusIndex = FirstItem;
        } else {
            if (m_focusIndex > CategoryChangeBtn) m_focusIndex = FirstItem;
        }
    } else {
        return;
    }

    switch (m_focusIndex) {
    case FirstItem: {
        m_searchWidget->categoryBtn()->clearFocus();
        if (m_displayMode == ALL_APPS)
            m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(0));
        update();
    }
    break;
    case SearchEdit: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        m_searchWidget->edit()->lineEdit()->setFocus();
    }
    break;
    case CategoryChangeBtn: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        m_searchWidget->categoryBtn()->setFocus();
    }
    break;
    }
}

void FullScreenFrame::setMultiWidgetVisible(bool state)
{
    m_internetMultiPagesView->setVisible(state);
    m_chatMultiPagesView->setVisible(state);
    m_musicMultiPagesView->setVisible(state);
    m_videoMultiPagesView->setVisible(state);
    m_graphicsMultiPagesView->setVisible(state);
    m_gameMultiPagesView->setVisible(state);
    m_officeMultiPagesView->setVisible(state);
    m_readingMultiPagesView->setVisible(state);
    m_developMultiPagesView->setVisible(state);
    m_systemMultiPagesView->setVisible(state);
    m_othersMultiPagesView->setVisible(state);

    m_internetMultiPagesView->updatePosition(m_displayMode);
    m_chatMultiPagesView->updatePosition(m_displayMode);
    m_musicMultiPagesView->updatePosition(m_displayMode);
    m_videoMultiPagesView->updatePosition(m_displayMode);
    m_graphicsMultiPagesView->updatePosition(m_displayMode);
    m_gameMultiPagesView->updatePosition(m_displayMode);
    m_officeMultiPagesView->updatePosition(m_displayMode);
    m_readingMultiPagesView->updatePosition(m_displayMode);
    m_developMultiPagesView->updatePosition(m_displayMode);
    m_systemMultiPagesView->updatePosition(m_displayMode);
    m_othersMultiPagesView->updatePosition(m_displayMode);
}

/**
 * @brief FullScreenFrame::mousePressDrag 处理全屏区域按住鼠标拖动翻页
 * @param e
 */
void FullScreenFrame::mousePressDrag(QMouseEvent *e)
{
    m_bMousePress = true;
    m_nMousePos = e->x();
    m_scrollValue = m_multiPagesView->getListArea()->horizontalScrollBar()->value();
    m_scrollStart = m_scrollValue;

    // 这里会导致单元测试程序异常崩溃,暂时屏蔽
    // if(e->button() != Qt::RightButton) m_multiPagesView->updateGradient();
}

void FullScreenFrame::mouseMoveDrag(QMouseEvent *e)
{
    int pageCount = m_multiPagesView->pageCount();
    int curPage = m_multiPagesView->currentPage();

    if (!m_bMousePress)
        return;

    int nDiff = m_nMousePos - e->x();

    // 处于首页继续向右滑动
    if (curPage == 0 && nDiff < 0)
        return;

    // 处于尾页继续向左滑动
    if (curPage == pageCount -1 && nDiff > 0)
        return;

    m_multiPagesView->getListArea()->horizontalScrollBar()->setValue(nDiff + m_scrollValue);
}

void FullScreenFrame::mouseReleaseDrag(QMouseEvent *e)
{
    int curPage = m_multiPagesView->currentPage();
    int nDiff = m_nMousePos - e->x();

    if (nDiff > DLauncher::TOUCH_DIFF_THRESH) {
        // 加大范围来避免手指点击触摸屏抖动问题
        m_multiPagesView->showCurrentPage(curPage + 1);
    } else if (nDiff < -DLauncher::TOUCH_DIFF_THRESH) {
        // 加大范围来避免手指点击触摸屏抖动问题
        m_multiPagesView->showCurrentPage(curPage - 1);
    } else {
       int nScroll = m_multiPagesView->getListArea()->horizontalScrollBar()->value();
        //多个分页是点击直接隐藏
        if (nScroll == m_scrollStart && curPage != 1)
            emit m_multiPagesView->getAppGridViewList()[curPage]->clicked(QModelIndex());
        else if (nScroll - m_scrollStart > DLauncher::MOUSE_MOVE_TO_NEXT)
            m_multiPagesView->showCurrentPage(curPage + 1);
        else if (nScroll - m_scrollStart < -DLauncher::MOUSE_MOVE_TO_NEXT)
            m_multiPagesView->showCurrentPage(curPage - 1);
        else
            m_multiPagesView->showCurrentPage(curPage);
    }

    m_bMousePress = false;

    m_multiPagesView->setGradientVisible(false);
}

AppsListModel::AppCategory FullScreenFrame::nextCategoryType(const AppsListModel::AppCategory category)
{
    AppsListModel::AppCategory nextCategory = AppsListModel::AppCategory(category + 1);
    if (nextCategory > AppsListModel::Others)
        nextCategory = AppsListModel::Internet;

    if (m_appsManager->appNums(nextCategory) <= 0)
        nextCategory = nextCategoryType(nextCategory);

    return nextCategory;
}

/**
 * @brief FullScreenFrame::prevCategoryType
 * 若当前分类下app数为0,，则自动显示上一个分类app
 * @param category 当前分类类型
 * @return 应用分类类型
 */
AppsListModel::AppCategory FullScreenFrame::prevCategoryType(const AppsListModel::AppCategory category)
{
    AppsListModel::AppCategory prevCategory = AppsListModel::AppCategory(category - 1);
    if (prevCategory < AppsListModel::Internet)
        prevCategory = AppsListModel::Others;

    if (m_appsManager->appNums(prevCategory) <= 0)
        prevCategory = prevCategoryType(prevCategory);

    return prevCategory;
}

const QScreen *FullScreenFrame::currentScreen()
{
    return m_appsManager->currentScreen();
}

void FullScreenFrame::layoutChanged()
{
    // 全屏模式下给控件设置整个屏幕大小
    QSize boxSize = m_contentFrame->size() - QSize(0, DLauncher::APPS_AREA_TOP_MARGIN);
    if (m_displayMode == ALL_APPS) {
        QSize boxSize = m_contentFrame->size() - QSize(0, DLauncher::APPS_AREA_TOP_MARGIN);
        m_appsIconBox->setFixedSize(boxSize);
        m_multiPagesView->setFixedSize(boxSize);
        m_multiPagesView->updatePosition(m_displayMode);
    } else if (m_displayMode == GROUP_BY_CATEGORY){
        for (int i = 0; i < m_appItemLayout->count(); i++) {
            MultiPagesView *pView = qobject_cast<MultiPagesView *>(m_appItemLayout->itemAt(i)->widget());
            if (!pView)
                return;

            pView->updatePosition(pView->getCategory());
        }
    } else {
        m_searchModeWidget->setFixedSize(boxSize);
    }
}

void FullScreenFrame::searchTextChanged(const QString &keywords, bool enableUpdateMode)
{
    if (!enableUpdateMode)
        return;

    if (keywords.isEmpty()) {
        updateDisplayMode(m_calcUtil->displayMode());
    } else {
        updateDisplayMode(SEARCH);
        QRegExp regExp(keywords.trimmed(), Qt::CaseInsensitive);
        m_filterModel->setFilterRegExp(regExp);
        m_searchModeWidget->setSearchModel(m_filterModel);
    }

    if (m_searchWidget->edit()->lineEdit()->text().isEmpty())
        m_searchWidget->edit()->lineEdit()->clearFocus();
}
