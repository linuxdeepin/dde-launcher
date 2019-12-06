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
#include "global_util/constants.h"
#include "global_util/xcb_misc.h"
#include "src/boxframe/backgroundmanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QClipboard>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>
#include <QScrollBar>
#include <QKeyEvent>
#include <QGraphicsEffect>
#include <QProcess>
#include <DWindowManagerHelper>

#include <ddialog.h>
#include <QScroller>

#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

#include "sharedeventfilter.h"

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

FullScreenFrame::FullScreenFrame(QWidget *parent) :
    BoxFrame(parent),
    m_menuWorker(new MenuWorker),
    m_eventFilter(new SharedEventFilter(this)),

    m_calcUtil(CalculateUtil::instance()),
    m_appsManager(AppsManager::instance()),
    m_delayHideTimer(new QTimer(this)),
    m_autoScrollTimer(new QTimer(this)),
    m_clearCacheTimer(new QTimer(this)),
    m_navigationWidget(new NavigationWidget(this)),
    m_searchWidget(new SearchWidget(this)),
    m_appsArea(new AppListArea),
    m_appsHbox(new DHBoxWidget),
    m_viewListPlaceholder(new QWidget),
    m_tipsLabel(new QLabel(this)),
    m_appItemDelegate(new AppItemDelegate),

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

    m_internetBoxWidget(new BlurBoxWidget),
    m_chatBoxWidget(new BlurBoxWidget),
    m_musicBoxWidget(new BlurBoxWidget),
    m_videoBoxWidget(new BlurBoxWidget),
    m_graphicsBoxWidget(new BlurBoxWidget),
    m_gameBoxWidget(new BlurBoxWidget),
    m_officeBoxWidget(new BlurBoxWidget),
    m_readingBoxWidget(new BlurBoxWidget),
    m_developmentBoxWidget(new BlurBoxWidget),
    m_systemBoxWidget(new BlurBoxWidget),
    m_othersBoxWidget(new BlurBoxWidget),

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

    m_floatTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Internet"), this)),
    m_internetTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Internet"))),
    m_chatTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Chat"))),
    m_musicTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Music"))),
    m_videoTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Video"))),
    m_graphicsTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Graphics"))),
    m_gameTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Game"))),
    m_officeTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Office"))),
    m_readingTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Reading"))),
    m_developmentTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Development"))),
    m_systemTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "System"))),
    m_othersTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", "Others")))
    , m_topSpacing(new QFrame)
    , m_bottomSpacing(new QFrame)
    , m_contentFrame(new QFrame)
{
    m_focusIndex = 0;
    m_pageCurrent = 0;
    m_iconViewActive = QIcon(":/widgets/images/page_indicator_active.svg");
    m_iconView = QIcon(":/widgets/images/page_indicator.svg");
    int pageCount = m_appsManager->getPageCount();
    for (int i = 0; i < pageCount; i++) {
        AppGridView *pView = new AppGridView;
        m_pageAppsViewList.push_back(pView);

        AppsListModel *pModel = new AppsListModel(AppsListModel::All);
        pModel->setPageIndex(i);
        m_pageAppsModelList.push_back(pModel);

        DFloatingButton *pBtn = new DFloatingButton(this);
        QIcon iconSelect = (i == m_pageCurrent) ? m_iconViewActive : m_iconView;
        pBtn->setIcon(iconSelect);
        pBtn->setIconSize(QSize(20, 20));
        pBtn->setFixedSize(QSize(20, 20));
        pBtn->setBackgroundRole(DPalette::Button);
        connect(pBtn, &DFloatingButton::clicked, this, &FullScreenFrame::pageBtnClick);
        m_floatBtnList.push_back(pBtn);
    }

    setFocusPolicy(Qt::NoFocus);

    setAttribute(Qt::WA_InputMethodEnabled, true);
    m_currentBox = 0;
#if (DTK_VERSION <= DTK_VERSION_CHECK(2, 0, 9, 9))
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
#else
    auto compositeChanged = [ = ] {
        if (DWindowManagerHelper::instance()->windowManagerName() == DWindowManagerHelper::WMName::KWinWM)
        {
            setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
        } else
        {
            setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
        }
    };

    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, compositeChanged);
    compositeChanged();
#endif

    setObjectName("LauncherFrame");

    installEventFilter(m_eventFilter);

    initUI();
    initConnection();

    updateDisplayMode(m_calcUtil->displayMode());
    updateDockPosition();
}

FullScreenFrame::~FullScreenFrame()
{

}

void FullScreenFrame::exit()
{
    qApp->quit();
}

void FullScreenFrame::showByMode(const qlonglong mode)
{
    qDebug() << mode;
}

int FullScreenFrame::dockPosition()
{
    return m_appsManager->dockPosition();
}

void FullScreenFrame::scrollToPage(const AppsListModel::AppCategory &category)
{
    QWidget *dest = categoryView(category);
    if (!dest)
        return;

    m_currentCategory = category;

    m_scrollDest = dest;
    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(m_appsArea->horizontalScrollBar()->value());
    m_scrollAnimation->setEndValue(dest->x());
    m_scrollAnimation->start();

    for (int i = 0; i < m_appsManager->getPageCount(); i++) {
        QIcon iconSelect = (i == m_pageCurrent) ? m_iconViewActive : m_iconView;
        m_floatBtnList[i]->setIcon(iconSelect);
    }
}

void FullScreenFrame::scrollToCategory(const AppsListModel::AppCategory &category)
{
    AppsListModel::AppCategory tempMode = category;
    if (tempMode < AppsListModel::Internet)
        tempMode  = AppsListModel::Internet;
    QWidget *dest = categoryBoxWidget(tempMode);

    if (!dest)
        return;

    m_currentCategory = tempMode;

    m_currentBox = m_currentCategory - 4;

    const int  temp = (qApp->primaryScreen()->geometry().size().width() / 2 -  LEFT_PADDING * 2) / 2 ;

    m_scrollDest = dest;
    int endValue = dest->x() - temp;
    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(m_appsArea->horizontalScrollBar()->value());
    m_scrollAnimation->setEndValue(endValue);
    m_scrollAnimation->start();

    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::scrollToBlurBoxWidget(BlurBoxWidget *category)
{
    QWidget *dest = category;

    if (!dest)
        return;

    m_currentCategory =  AppsListModel::AppCategory(m_currentBox + 4);
    const int  temp = (qApp->primaryScreen()->geometry().size().width() / 2 -  LEFT_PADDING * 2) / 2 ;
    m_scrollDest = dest;

    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(m_appsArea->horizontalScrollBar()->value());
    m_scrollAnimation->setEndValue(dest->x() - temp);
    m_scrollAnimation->start();

    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::showTips(const QString &tips)
{
    if (m_displayMode != SEARCH)
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
    QTimer::singleShot(0, this, [ = ] {
        updateBackground();
        updateDockPosition();
    });

    QFrame::resizeEvent(e);
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
            m_searchWidget->edit()->setText(clipboardText);
            m_searchWidget->setFocus();
            m_searchWidget->edit()->setFocus();
        }
    }
}

void FullScreenFrame::showEvent(QShowEvent *e)
{
    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();
    //updateCurrentVisibleCategory();
    // TODO: Do we need this in showEvent ???
    XcbMisc::instance()->set_deepin_override(winId());
    // To make sure the window is placed at the right position.
    updateGeometry();
    updateBackground();
    updateDockPosition();

    // force refresh
    if (!m_appsManager->isVaild()) {
        m_appsManager->refreshAllList();
    }

    QFrame::showEvent(e);

    QTimer::singleShot(0, this, [this]() {
        raise();
        activateWindow();
        m_floatTitle->raise();
        m_searchWidget->raise();
        emit visibleChanged(true);
    });

    m_clearCacheTimer->stop();

    if (m_displayMode != ALL_APPS) {
        refreshTitleVisible();
    }
}

void FullScreenFrame::hideEvent(QHideEvent *e)
{
    BoxFrame::hideEvent(e);

    QTimer::singleShot(1, this, [ = ] { emit visibleChanged(false); });

    m_clearCacheTimer->start();
}

void FullScreenFrame::pageBtnClick()
{
    for (int i = 0; i < m_floatBtnList.size(); i++) {
        if (sender() == m_floatBtnList[i]) {
            m_pageCurrent = i;
            emit scrollChanged(AppsListModel::All);
            break;
        }
    }
}

void FullScreenFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        return;

    hide();
}

void FullScreenFrame::wheelEvent(QWheelEvent *e)
{

    if (m_displayMode == GROUP_BY_CATEGORY) {
        if (m_scrollAnimation->state() == m_scrollAnimation->Running) return;
        static int  wheelTime = 0;
        if (e->angleDelta().y() < 0) {
            wheelTime++;
        } else {
            wheelTime--;
        }

        if (wheelTime >= DLauncher::WHOOLTIME_TO_SCROOL || wheelTime <= -DLauncher::WHOOLTIME_TO_SCROOL) {
            int boxWidgetLen = sizeof(m_BoxWidget) / sizeof(m_BoxWidget[0]);
            if (wheelTime > 0) {
                m_currentBox++;
                if (m_currentBox > boxWidgetLen - 1) {
                    m_currentBox = 0;
                }
            } else {
                m_currentBox--;
                if (m_currentBox < 0) {
                    m_currentBox = boxWidgetLen - 1;
                }
            }

            if (!m_BoxWidget[m_currentBox]->isVisible()) {
                if (wheelTime > 0) {
                    m_currentBox++;
                    if (m_currentBox > boxWidgetLen - 1) {
                        m_currentBox = 0;
                    }
                } else {
                    m_currentBox--;
                    if (m_currentBox < 0) {
                        m_currentBox = boxWidgetLen - 1;
                    }
                }

            }
            scrollToBlurBoxWidget(m_BoxWidget[m_currentBox]);
            wheelTime = 0;
        }
        return;
    }

    if (m_scrollAnimation->state() == QPropertyAnimation::Running)
        return;

    int page = m_pageCurrent;
    if (e->delta() > 0) {
        if (m_pageCurrent - 1 >= 0)
            -- m_pageCurrent;
    } else if (e->delta() < 0) {
        if (m_pageCurrent + 1 < m_appsManager->getPageCount())
            ++ m_pageCurrent;
    }
    if (page != m_pageCurrent)
        emit scrollChanged(AppsListModel::All);
}

///
/// \brief FullScreenFrame::eventFilter
/// \param o
/// \param e
/// \return
///
bool FullScreenFrame::eventFilter(QObject *o, QEvent *e)
{
    // we filter some key events from LineEdit, to implements cursor move.
    if (o == m_searchWidget->edit() && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyPress = static_cast<QKeyEvent *>(e);
        if (keyPress->key() == Qt::Key_Left || keyPress->key() == Qt::Key_Right) {
            QKeyEvent *event = new QKeyEvent(keyPress->type(), keyPress->key(), keyPress->modifiers());
            qApp->postEvent(this, event);
            return true;
        }
    } else if ((o == m_appsArea->viewport() && e->type() == QEvent::Wheel)
               || (o == m_appsArea && e->type() == QEvent::Scroll)) {

    } else if (o == m_appsArea->viewport() && e->type() == QEvent::Resize) {
        const int pos = m_appsManager->dockPosition();
        m_calcUtil->calculateAppLayout(static_cast<QResizeEvent *>(e)->size() - QSize(LEFT_PADDING + RIGHT_PADDING, 0), pos);
        updatePlaceholderSize();

    }

    return false;
}

void FullScreenFrame::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        m_searchWidget->edit()->setText(e->commitString());
        m_searchWidget->edit()->setFocus();
    }

    QWidget::inputMethodEvent(e);
}

QVariant FullScreenFrame::inputMethodQuery(Qt::InputMethodQuery prop) const
{
    switch (prop) {
    case Qt::ImEnabled:
        return true;
    case Qt::ImCursorRectangle:
        return widgetRelativeOffset(this, m_searchWidget->edit());
    default:;
    }

    return QWidget::inputMethodQuery(prop);
}

void FullScreenFrame::initUI()
{
    m_searchWidget->showToggle();

    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(500, 50);
    m_tipsLabel->setVisible(false);

    m_delayHideTimer->setInterval(500);
    m_delayHideTimer->setSingleShot(true);

    m_autoScrollTimer->setInterval(DLauncher::APPS_AREA_AUTO_SCROLL_TIMER);
    m_autoScrollTimer->setSingleShot(false);

    m_clearCacheTimer->setSingleShot(true);
    m_clearCacheTimer->setInterval(DLauncher::CLEAR_CACHE_TIMER * 1000);

    m_appsArea->setObjectName("AppBox");
    m_appsArea->viewport()->setAutoFillBackground(false);
    m_appsArea->setWidgetResizable(true);
    m_appsArea->setFocusPolicy(Qt::NoFocus);
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_appsArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->viewport()->installEventFilter(this);
    m_appsArea->installEventFilter(this);

    m_searchWidget->edit()->lineEdit()->installEventFilter(m_eventFilter);
    m_searchWidget->categoryBtn()->installEventFilter(m_eventFilter);
    m_searchWidget->installEventFilter(m_eventFilter);
    m_appItemDelegate->installEventFilter(m_eventFilter);

    QHBoxLayout *iconHLayout = new QHBoxLayout;
    iconHLayout->setSpacing(10);
    iconHLayout->addStretch();

    DHBoxWidget *allAppHbox = new DHBoxWidget;
    for (int i = 0; i < m_appsManager->getPageCount(); i++) {
        QString name = QString("page%1").arg(i + 1);
        m_pageAppsViewList[i]->setAccessibleName(name);
        m_pageAppsViewList[i]->setModel(m_pageAppsModelList[i]);
        m_pageAppsViewList[i]->setItemDelegate(m_appItemDelegate);
        m_pageAppsViewList[i]->setContainerBox(m_appsArea);

        m_appsHbox->layout()->addWidget(m_pageAppsViewList[i]);
        m_appsHbox->layout()->addSpacing(RIGHT_PADDING);
        m_appsArea->addWidget(m_pageAppsViewList[i]->viewport());

        iconHLayout->addWidget(m_floatBtnList[i]);
    }
    iconHLayout->addStretch();

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
    m_internetTitle->setTextVisible(true);
    m_chatTitle->setTextVisible(true);
    m_musicTitle->setTextVisible(true);
    m_videoTitle->setTextVisible(true);
    m_graphicsTitle->setTextVisible(true);
    m_gameTitle->setTextVisible(true);
    m_officeTitle->setTextVisible(true);
    m_readingTitle->setTextVisible(true);
    m_developmentTitle->setTextVisible(true);
    m_systemTitle->setTextVisible(true);
    m_othersTitle->setTextVisible(true);

    m_internetBoxWidget->qvLayout->addWidget(m_internetTitle, m_internetView->width() / 2, Qt::AlignHCenter);
    m_internetBoxWidget->qvLayout->addWidget(m_internetView);
    m_appsHbox->layout()->addWidget(m_internetBoxWidget);

    m_chatBoxWidget->qvLayout->addWidget(m_chatTitle, m_chatView->width() / 2, Qt::AlignHCenter);
    m_chatBoxWidget->qvLayout->addWidget(m_chatView);
    m_appsHbox->layout()->addWidget(m_chatBoxWidget);

    m_musicBoxWidget->qvLayout->addWidget(m_musicTitle, m_musicView->width() / 2, Qt::AlignHCenter);
    m_musicBoxWidget->layout()->addWidget(m_musicView);
    m_appsHbox->layout()->addWidget(m_musicBoxWidget);

    m_videoBoxWidget->qvLayout->addWidget(m_videoTitle, m_videoView->width() / 2, Qt::AlignHCenter);
    m_videoBoxWidget->qvLayout->addWidget(m_videoView);
    m_appsHbox->layout()->addWidget(m_videoBoxWidget);

    m_graphicsBoxWidget->qvLayout->addWidget(m_graphicsTitle, m_graphicsView->width() / 2, Qt::AlignHCenter);
    m_graphicsBoxWidget->layout()->addWidget(m_graphicsView);
    m_appsHbox->layout()->addWidget(m_graphicsBoxWidget);

    m_gameBoxWidget->qvLayout->addWidget(m_gameTitle, m_gameView->width() / 2, Qt::AlignHCenter);
    m_gameBoxWidget->layout()->addWidget(m_gameView);
    m_appsHbox->layout()->addWidget(m_gameBoxWidget);

    m_officeBoxWidget->qvLayout->addWidget(m_officeTitle, m_officeView->width() / 2, Qt::AlignHCenter);
    m_officeBoxWidget->layout()->addWidget(m_officeView);
    m_appsHbox->layout()->addWidget(m_officeBoxWidget);

    m_readingBoxWidget->qvLayout->addWidget(m_readingTitle, m_readingView->width() / 2, Qt::AlignHCenter);
    m_readingBoxWidget->layout()->addWidget(m_readingView);
    m_appsHbox->layout()->addWidget(m_readingBoxWidget);

    m_developmentBoxWidget->qvLayout->addWidget(m_developmentTitle, m_developmentView->width() / 2, Qt::AlignHCenter);
    m_developmentBoxWidget->layout()->addWidget(m_developmentView);
    m_appsHbox->layout()->addWidget(m_developmentBoxWidget);

    m_systemBoxWidget->qvLayout->addWidget(m_systemTitle, m_systemView->width() / 2, Qt::AlignHCenter);
    m_systemBoxWidget->layout()->addWidget(m_systemView);
    m_appsHbox->layout()->addWidget(m_systemBoxWidget);

    m_othersBoxWidget->qvLayout->addWidget(m_othersTitle, m_othersView->width() / 2, Qt::AlignHCenter);
    m_othersBoxWidget->layout()->addWidget(m_othersView);
    m_appsHbox->layout()->addWidget(m_othersBoxWidget);

    m_appsHbox->layout()->addWidget(m_viewListPlaceholder);
    m_appsHbox->layout()->addStretch();
    m_appsHbox->layout()->setSpacing(DLauncher::APPHBOX_SPACING);
    m_appsHbox->layout()->setContentsMargins(0, DLauncher::APPS_AREA_TOP_MARGIN, 0, 0);

    m_appsArea->addWidget(m_internetTitle);
    m_appsArea->addWidget(m_internetView->viewport());
    m_appsArea->addWidget(m_chatTitle);
    m_appsArea->addWidget(m_chatView->viewport());
    m_appsArea->addWidget(m_musicTitle);
    m_appsArea->addWidget(m_musicView->viewport());
    m_appsArea->addWidget(m_videoTitle);
    m_appsArea->addWidget(m_videoView->viewport());
    m_appsArea->addWidget(m_graphicsTitle);
    m_appsArea->addWidget(m_graphicsView->viewport());
    m_appsArea->addWidget(m_gameTitle);
    m_appsArea->addWidget(m_gameView->viewport());
    m_appsArea->addWidget(m_officeTitle);
    m_appsArea->addWidget(m_officeView->viewport());
    m_appsArea->addWidget(m_readingTitle);
    m_appsArea->addWidget(m_readingView->viewport());
    m_appsArea->addWidget(m_developmentTitle);
    m_appsArea->addWidget(m_developmentView->viewport());
    m_appsArea->addWidget(m_systemTitle);
    m_appsArea->addWidget(m_systemView->viewport());
    m_appsArea->addWidget(m_othersTitle);
    m_appsArea->addWidget(m_othersView->viewport());
    m_appsArea->addWidget(m_viewListPlaceholder);

    m_contentFrame = new QFrame;
    m_contentFrame->setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout *scrollVLayout = new QVBoxLayout;
    scrollVLayout->setMargin(0);
    scrollVLayout->setSpacing(0);

    QHBoxLayout *scrollHLayout = new QHBoxLayout;
    scrollHLayout->setMargin(0);
    scrollHLayout->setSpacing(0);
    scrollHLayout->addSpacing(LEFT_PADDING);
    scrollHLayout->addWidget(m_appsHbox, 0, Qt::AlignTop);
    scrollHLayout->addSpacing(RIGHT_PADDING);

    scrollVLayout->addLayout(scrollHLayout);

    m_contentFrame->setLayout(scrollVLayout);

    m_appsArea->setWidget(m_contentFrame);

    //m_navigationWidget->setFixedWidth(width());
    m_navigationWidget->setFixedHeight(m_calcUtil->instance()->navigationHeight());
    m_navigationWidget->show();

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->addSpacing(0);
    m_mainLayout->addWidget(m_topSpacing);
    m_mainLayout->addWidget(m_searchWidget);
    m_mainLayout->addWidget(m_navigationWidget, 0, Qt::AlignHCenter);
    m_mainLayout->addWidget(m_appsArea);
    m_mainLayout->addLayout(iconHLayout);
    m_mainLayout->addWidget(m_bottomSpacing);

    setLayout(m_mainLayout);

    // animation
    m_scrollAnimation = new QPropertyAnimation(m_appsArea->horizontalScrollBar(), "value");
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);
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
    if (m_displayMode != GROUP_BY_CATEGORY)
        return m_floatTitle->setVisible(false);

    CategoryTitleWidget *sourceTitle = categoryTitle(m_currentCategory);
    if (!sourceTitle)
        return;
}

CategoryTitleWidget *FullScreenFrame::categoryTitle(const AppsListModel::AppCategory category) const
{
    CategoryTitleWidget *dest = nullptr;

    switch (category) {
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

    switch (category) {
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
    case AppsListModel::All:            view = m_pageAppsViewList[m_pageCurrent];   break;
    default:;
    }

    return view;
}


BlurBoxWidget *FullScreenFrame::categoryBoxWidget(const AppsListModel::AppCategory category) const
{
    BlurBoxWidget *view = nullptr;

    switch (category) {
    case AppsListModel::Internet:       view = m_internetBoxWidget;      break;
    case AppsListModel::Chat:           view = m_chatBoxWidget;          break;
    case AppsListModel::Music:          view = m_musicBoxWidget;         break;
    case AppsListModel::Video:          view = m_videoBoxWidget;         break;
    case AppsListModel::Graphics:       view = m_graphicsBoxWidget;      break;
    case AppsListModel::Game:           view = m_gameBoxWidget;          break;
    case AppsListModel::Office:         view = m_officeBoxWidget;        break;
    case AppsListModel::Reading:        view = m_readingBoxWidget;       break;
    case AppsListModel::Development:    view = m_developmentBoxWidget;   break;
    case AppsListModel::System:         view = m_systemBoxWidget;        break;
    case AppsListModel::Others:         view = m_othersBoxWidget;        break;
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
    connect(m_appsArea, &AppListArea::increaseIcon, this, [ = ] { m_calcUtil->increaseIconSize(); emit m_appsManager->layoutChanged(AppsListModel::All); });
    connect(m_appsArea, &AppListArea::decreaseIcon, this, [ = ] { m_calcUtil->decreaseIconSize(); emit m_appsManager->layoutChanged(AppsListModel::All); });

    connect(qApp, &QApplication::primaryScreenChanged, this, &FullScreenFrame::updateGeometry);
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &FullScreenFrame::updateGeometry);
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &FullScreenFrame::layoutChanged, Qt::QueuedConnection);
    //connect(m_scrollAnimation, &QPropertyAnimation::valueChanged, this, &FullScreenFrame::ensureScrollToDest);
    connect(m_scrollAnimation, &QPropertyAnimation::finished, this, &FullScreenFrame::refershCurrentFloatTitle, Qt::QueuedConnection);
    connect(m_navigationWidget, &NavigationWidget::scrollToCategory, this, &FullScreenFrame::scrollToCategory);
    connect(this, &FullScreenFrame::scrollChanged, this, &FullScreenFrame::scrollToPage);

    connect(this, &FullScreenFrame::currentVisibleCategoryChanged, m_navigationWidget, &NavigationWidget::setCurrentCategory);
    connect(this, &FullScreenFrame::currentVisibleCategoryChanged, this, &FullScreenFrame::setblurboxChange);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, m_navigationWidget, &NavigationWidget::refershCategoryVisible);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, this, &FullScreenFrame::refershCategoryVisible);
    connect(this, &FullScreenFrame::displayModeChanged, this, &FullScreenFrame::checkCategoryVisible);
    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &FullScreenFrame::searchTextChanged);
    connect(m_delayHideTimer, &QTimer::timeout, this, &FullScreenFrame::hide, Qt::QueuedConnection);

    connect(m_clearCacheTimer, &QTimer::timeout, m_appsManager, &AppsManager::clearCache);

    // auto scroll when drag to app list box border
    connect(m_pageAppsViewList[0], &AppGridView::requestScrollStop, m_autoScrollTimer, &QTimer::stop);
    connect(m_autoScrollTimer, &QTimer::timeout, [this] {
        m_appsArea->horizontalScrollBar()->setValue(m_appsArea->horizontalScrollBar()->value() + m_autoScrollStep);
    });
    connect(m_pageAppsViewList[0], &AppGridView::requestScrollUp, [this] {
        m_autoScrollStep = -DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
        if (!m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();
    });
    connect(m_pageAppsViewList[0], &AppGridView::requestScrollDown, [this] {
        m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
        if (!m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();
    });

    for (auto pageView : m_pageAppsViewList) {
        connect(pageView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
        connect(pageView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
        connect(pageView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
        connect(pageView, &AppGridView::clicked, this, &FullScreenFrame::hide);
        connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, pageView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    }

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

    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_internetView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_chatView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_musicView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_videoView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_graphicsView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_gameView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_officeView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_readingView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_developmentView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_systemView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_othersView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &FullScreenFrame::hideLauncher);
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (FullScreenFrame::*)(const QModelIndex &)>(&FullScreenFrame::uninstallApp));
    connect(m_searchWidget, &SearchWidget::toggleMode, [this] {
        m_searchWidget->clearFocus();
        m_searchWidget->clearSearchContent();
        updateDisplayMode(m_displayMode == GROUP_BY_CATEGORY ? ALL_APPS : GROUP_BY_CATEGORY);
    });

    connect(m_appsManager, &AppsManager::categoryListChanged, this, &FullScreenFrame::checkCategoryVisible);
    connect(m_appsManager, &AppsManager::requestTips, this, &FullScreenFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &FullScreenFrame::hideTips);
    connect(m_appsManager, &AppsManager::dockGeometryChanged, this, &FullScreenFrame::updateDockPosition);


}

void FullScreenFrame::showLauncher()
{
    show();
    setFixedSize(qApp->primaryScreen()->geometry().size());
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
    const QRect rect = qApp->primaryScreen()->geometry();

    setGeometry(rect);

    QFrame::updateGeometry();
}

///
/// NOTE(sbw): for design, user can change current item by keys.
/// so we need calculate which item will be hovered after key event processed.
///
/// \brief FullScreenFrame::moveCurrentSelectApp
/// \param key
///
void FullScreenFrame::moveCurrentSelectApp(const int key)
{
    if (Qt::Key_Tab == key || Qt::Key_Backtab == key) {
        nextTabWidget();
        return;
    }

    if (Qt::Key_Space == key) {
        if (m_searchWidget->categoryBtn()->hasFocus()) {
            QMouseEvent btnPress(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(m_searchWidget->categoryBtn(), &btnPress);
            QMouseEvent btnRelease(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(m_searchWidget->categoryBtn(), &btnRelease);
        }
        return;
    }

    const QModelIndex currentIndex = m_appItemDelegate->currentIndex();

    // move operation should be start from a valid location, if not, just init it.
    if (!currentIndex.isValid()) {
        m_appItemDelegate->setCurrentIndex(m_displayMode == GROUP_BY_CATEGORY ? m_internetView->indexAt(0) : m_pageAppsViewList[m_pageCurrent]->indexAt(0));
        update();
        return;
    }

    const int column = m_calcUtil->appColumnCount();
    QModelIndex index;

    // calculate destination sibling by keys, it may cause an invalid position.
    switch (key) {
    case Qt::Key_Backtab:
    case Qt::Key_Left:      index = currentIndex.sibling(currentIndex.row() - 1, 0);        break;
    //case Qt::Key_Tab: break;
    case Qt::Key_Right:     index = currentIndex.sibling(currentIndex.row() + 1, 0);        break;
    case Qt::Key_Up:        index = currentIndex.sibling(currentIndex.row() - column, 0);   break;
    case Qt::Key_Down:      index = currentIndex.sibling(currentIndex.row() + column, 0);   break;
    default:;
    }

    //to next page
    if (m_displayMode == ALL_APPS && !index.isValid()) {
        int page = m_pageCurrent;
        int itemSelect = 0;
        if (Qt::Key_Left == key || Qt::Key_Up == key) {
            if (m_pageCurrent - 1 >= 0) {
                -- m_pageCurrent;
                itemSelect = m_calcUtil->appPageItemCount() - 1;
            }
        } else {
            if (m_pageCurrent + 1 < m_appsManager->getPageCount())
                ++ m_pageCurrent;
        }
        if (page != m_pageCurrent)
            emit scrollChanged(AppsListModel::All);

        index = m_pageAppsViewList[m_pageCurrent]->indexAt(itemSelect);
    }

    // now, we need to check and fix if destination is invalid.
    do {
        if (index.isValid())
            break;

        // the column number of destination, when moving up/down, columns shouldn't be changed.
        //const int realColumn = currentIndex.row() % column;

        const AppsListModel *model = static_cast<const AppsListModel *>(currentIndex.model());
        if (key == Qt::Key_Down || key == Qt::Key_Right)
            model = nextCategoryModel(model);
        else if (key == Qt::Key_Up || key == Qt::Key_Left)
            model = prevCategoryModel(model);
        // if we can't find any available model which contains that column. this move operate should be abort.
        if (!model)
            break;
        index = model->index(0);
    } while (false);

    // valid verify and UI adjustment.
    const QModelIndex selectedIndex = index.isValid() ? index : currentIndex;
    m_appItemDelegate->setCurrentIndex(selectedIndex);
    ensureItemVisible(selectedIndex);
    update();
}

void FullScreenFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->setFocus(Qt::MouseFocusReason);

    // -1 means backspace key pressed
    if (ch == -1) {
//        m_searchWidget->edit()->backspace();
        m_searchWidget->edit()->lineEdit()->backspace();
        return;
    }

    if (!m_searchWidget->edit()->lineEdit()->selectedText().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->backspace();
    }

    m_searchWidget->edit()->setText(m_searchWidget->edit()->text() + ch);
}

void FullScreenFrame::launchCurrentApp()
{
    if (m_searchWidget->categoryBtn()->hasFocus()) {
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
    case ALL_APPS:            m_appsManager->launchApp(m_pageAppsViewList[m_pageCurrent]->indexAt(0));     break;
    case GROUP_BY_CATEGORY:   m_appsManager->launchApp(m_internetView->indexAt(0));    break;
    }

    hide();
}

bool FullScreenFrame::windowDeactiveEvent()
{
    return false;
}

void FullScreenFrame::regionMonitorPoint(const QPoint &point)
{
    if (!m_menuWorker->isMenuShown() && !m_isConfirmDialogShown && !m_delayHideTimer->isActive()) {
        if (m_appsManager->dockGeometry().contains(point)) {
            m_delayHideTimer->start();
            hideLauncher();
        }
    }
}

void FullScreenFrame::checkCategoryVisible()
{
    if (m_displayMode != GROUP_BY_CATEGORY)
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

    m_menuWorker->showMenuByAppItem(pos, context);
}

void FullScreenFrame::uninstallApp(const QString &appKey)
{
    for (auto pageModel : m_pageAppsModelList)
        uninstallApp(pageModel->indexAt(appKey));
}

void FullScreenFrame::uninstallApp(const QModelIndex &context)
{
    if (m_isConfirmDialogShown)
        return;

    m_isConfirmDialogShown = true;

    DTK_WIDGET_NAMESPACE::DDialog unInstallDialog;
    unInstallDialog.setWindowFlags(Qt::Dialog | unInstallDialog.windowFlags());
    unInstallDialog.setWindowModality(Qt::WindowModal);

    const QString appKey = context.data(AppsListModel::AppKeyRole).toString();
    QString appName = context.data(AppsListModel::AppNameRole).toString();
    unInstallDialog.setTitle(QString(tr("Are you sure you want to uninstall %1 ?")).arg(appName));
    QPixmap appIcon = context.data(AppsListModel::AppDialogIconRole).value<QPixmap>();
    unInstallDialog.setIconPixmap(appIcon);

    QStringList buttons;
    buttons << tr("Cancel") << tr("Confirm");
    unInstallDialog.addButtons(buttons);

//    connect(&unInstallDialog, SIGNAL(buttonClicked(int, QString)), this, SLOT(handleUninstallResult(int, QString)));
    connect(&unInstallDialog, &DTK_WIDGET_NAMESPACE::DDialog::buttonClicked, [&](int clickedResult) {
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

    if (m_scrollDest->x() != ani->endValue())
        ani->setEndValue(m_scrollDest->x());
}

void FullScreenFrame::ensureItemVisible(const QModelIndex &index)
{
    AppGridView *view = nullptr;
    const AppsListModel::AppCategory category = index.data(AppsListModel::AppCategoryRole).value<AppsListModel::AppCategory>();

    if (m_displayMode == SEARCH || m_displayMode == ALL_APPS)
        view = m_pageAppsViewList[m_pageCurrent];
    else
        view = categoryView(category);

    if (!view)
        return;

    // m_appsArea->ensureVisible(0, view->indexYOffset(index) + view->pos().y(), 0, DLauncher::APPS_AREA_ENSURE_VISIBLE_MARGIN_Y);
    //updateCurrentVisibleCategory();
    if (category != m_currentCategory) {
        scrollToCategory(category);
    }
    // refershCurrentFloatTitle();
}

void FullScreenFrame::setblurboxChange()
{
    if (m_displayMode != GROUP_BY_CATEGORY)
        return;

    int  boxWidgetLen = sizeof(m_BoxWidget) / sizeof(m_BoxWidget[0]) - 1;
    m_BoxWidget[m_currentBox]->layout()->itemAt(0)->setAlignment(Qt::AlignHCenter);
    m_BoxWidget[m_currentBox]->layout()->update();
    if (m_currentBox > 0) {
        m_BoxWidget[m_currentBox - 1]->layout()->itemAt(0)->setAlignment(Qt::AlignRight);
        m_BoxWidget[m_currentBox - 1]->layout()->update();
    } else {
        m_BoxWidget[boxWidgetLen - 1]->layout()->itemAt(0)->setAlignment(Qt::AlignRight);
        m_BoxWidget[boxWidgetLen - 1]->layout()->update();
    }

    if (m_currentBox == boxWidgetLen) {
        m_BoxWidget[0]->layout()->itemAt(0)->setAlignment(Qt::AlignLeft);
        m_BoxWidget[ 0]->layout()->update();

    } else {
        m_BoxWidget[ m_currentBox + 1]->layout()->itemAt(0)->setAlignment(Qt::AlignLeft);
        m_BoxWidget[m_currentBox + 1]->layout()->update();
    }
}

void FullScreenFrame::refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums)
{
    if (m_displayMode != GROUP_BY_CATEGORY)
        return;

    QWidget *categoryTitle = this->categoryTitle(category);
    QWidget *categoryView = this->categoryBoxWidget(category);

    if (categoryView)
        categoryView->setVisible(appNums);
    if (categoryTitle)
        categoryTitle->setVisible(appNums);
}

void FullScreenFrame::updateDisplayMode(const int mode)
{
    if (m_displayMode == mode)
        return;

    m_displayMode = mode;

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

    bool isCategoryMode = m_displayMode == GROUP_BY_CATEGORY;
    m_pageCurrent = (m_displayMode == SEARCH) ? 0 : m_pageCurrent;

    for (int i = 0; i < m_appsManager->getPageCount(); i++) {
        if (m_displayMode == SEARCH) {
            m_pageAppsViewList[i]->setVisible(i == 0);
            m_floatBtnList[i]->setVisible(i == 0);
            m_floatBtnList[i]->setIcon((i == 0) ? m_iconViewActive : m_iconView);
        } else {
            m_pageAppsViewList[i]->setVisible(!isCategoryMode);
            m_floatBtnList[i]->setVisible(!isCategoryMode);
        }
    }

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
    m_navigationWidget->setVisible(isCategoryMode);

    m_pageAppsViewList[0]->setModel(m_displayMode == SEARCH ? m_searchResultModel : m_pageAppsModelList[0]);
    // choose nothing
    m_appItemDelegate->setCurrentIndex(QModelIndex());

    hideTips();

    if (m_displayMode == GROUP_BY_CATEGORY)
        scrollToCategory(m_currentCategory);
    else
        m_appsArea->horizontalScrollBar()->setValue(0);

    emit displayModeChanged(m_displayMode);
}

void FullScreenFrame::updateCurrentVisibleCategory()
{
    if (m_displayMode != GROUP_BY_CATEGORY)
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

    if (m_currentCategory == currentVisibleCategory)
        return;

    m_currentCategory = currentVisibleCategory;

    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::updatePlaceholderSize()
{
    const AppGridView *view = lastVisibleView();
    Q_ASSERT(view);

    m_viewListPlaceholder->setVisible(dockPosition() == DOCK_POS_BOTTOM);
}

void FullScreenFrame::updateDockPosition()
{
    // reset all spacing size
    m_topSpacing->setFixedHeight(30);
    m_bottomSpacing->setFixedHeight(0);

    const QRect dockGeometry = m_appsManager->dockGeometry();

    switch (m_appsManager->dockPosition()) {
    case DOCK_POS_TOP:
        m_topSpacing->setFixedHeight(30 + dockGeometry.height());
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(0);
        break;
    case DOCK_POS_BOTTOM:
        m_bottomSpacing->setFixedHeight(DLauncher::VIEWLIST_BOTTOM_MARGIN);
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(0);
        break;
    case DOCK_POS_LEFT:
        m_searchWidget->setLeftSpacing(dockGeometry.width());
        m_searchWidget->setRightSpacing(0);
        break;
    case DOCK_POS_RIGHT:
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(dockGeometry.width());
        break;
    default:
        break;
    }

    m_calcUtil->calculateAppLayout(m_appsArea->size() - QSize(LEFT_PADDING + RIGHT_PADDING, 0),
                                   m_appsManager->dockPosition());
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
        return m_internetModel;

    return nullptr;
}

void FullScreenFrame::nextTabWidget()
{
    switch (m_focusIndex) {
    case FirstItem: {
        m_searchWidget->categoryBtn()->clearFocus();
        AppGridView *pView = (m_displayMode == GROUP_BY_CATEGORY) ? m_internetView : m_pageAppsViewList[m_pageCurrent];
        m_appItemDelegate->setCurrentIndex(pView->indexAt(0));
        update();
        m_focusIndex = SearchEdit;
    }
    break;
    case SearchEdit: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        m_searchWidget->edit()->lineEdit()->setFocus();
        m_focusIndex = CategoryChangeBtn;
    }
    break;
    case CategoryChangeBtn: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        m_searchWidget->categoryBtn()->setFocus();
        m_focusIndex = FirstItem;
        m_focusIndex = (m_displayMode != GROUP_BY_CATEGORY) ? FirstItem : CategoryTital;
    }
    break;
    case CategoryTital: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        m_navigationWidget->setCurrentCategory(AppsListModel::Internet);
        m_navigationWidget->button(AppsListModel::Internet)->setFocus();
        m_focusIndex = FirstItem;
    }
    break;
    }
}

AppsListModel *FullScreenFrame::prevCategoryModel(const AppsListModel *currentModel)
{
    if (currentModel == m_internetModel)
        return m_othersModel;
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
    QSize boxSize;
    if (m_displayMode == ALL_APPS || m_displayMode == SEARCH) {
        const int appsContentWidth = (width() - LEFT_PADDING - RIGHT_PADDING);
        boxSize.setWidth(appsContentWidth);
        boxSize.setHeight(m_appsArea->height());
    } else {
        boxSize = m_calcUtil->getAppBoxSize();
    }

    m_appsHbox->setFixedHeight(m_appsArea->height());

    for (auto pageView : m_pageAppsViewList)
        pageView->setFixedSize(boxSize);

    m_internetView->setFixedSize(boxSize);
    m_musicView->setFixedSize(boxSize);
    m_videoView->setFixedSize(boxSize);
    m_graphicsView->setFixedSize(boxSize);
    m_gameView->setFixedSize(boxSize);
    m_officeView->setFixedSize(boxSize);
    m_readingView->setFixedSize(boxSize);
    m_developmentView->setFixedSize(boxSize);
    m_systemView->setFixedSize(boxSize);
    m_othersView->setFixedSize(boxSize);

    m_floatTitle->move(m_appsArea->pos().x() + LEFT_PADDING, m_appsArea->y() - m_floatTitle->height() + 10);

    if (m_displayMode == ALL_APPS || m_displayMode == SEARCH) {
        m_appsArea->horizontalScrollBar()->setValue(0);
    } else {
        scrollToCategory(m_currentCategory);
    }
}

void FullScreenFrame::searchTextChanged(const QString &keywords)
{
    if (keywords.isEmpty())
        updateDisplayMode(m_calcUtil->displayMode());
    else
        updateDisplayMode(SEARCH);

    m_appsManager->searchApp(keywords.trimmed());
}

