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

    m_multiPagesView(new MultiPagesView()),
    m_multiPagesInternetView(new MultiPagesView(AppsListModel::Internet)),
    m_multiPagesChatView(new MultiPagesView(AppsListModel::Chat)),
    m_multiPagesMusicView(new MultiPagesView(AppsListModel::Music)),
    m_multiPagesVideoView(new MultiPagesView(AppsListModel::Video)),
    m_multiPagesGraphicsView(new MultiPagesView(AppsListModel::Graphics)),
    m_multiPagesGameView(new MultiPagesView(AppsListModel::Game)),
    m_multiPagesOfficeView(new MultiPagesView(AppsListModel::Office)),
    m_multiPagesReadingView(new MultiPagesView(AppsListModel::Reading)),
    m_multiPagesDevelopmentView(new MultiPagesView(AppsListModel::Development)),
    m_multiPageSystemView(new MultiPagesView(AppsListModel::System)),
    m_multiPagesOthersView(new MultiPagesView(AppsListModel::Others)),

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
    //m_currentCategory = AppsListModel::Internet;

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

    connect(m_multiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
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

void FullScreenFrame::scrollToCategory(const AppsListModel::AppCategory &category, int nNext)
{
    AppsListModel::AppCategory tempMode = category;
    if (tempMode < AppsListModel::Internet || tempMode > AppsListModel::Others)
        tempMode = AppsListModel::Internet ;

    if (category != m_currentCategory || category == 0) {
        setCategoryIndex(tempMode, nNext);
    }

    QWidget *dest = categoryBoxWidget(tempMode);

    if (!dest) return;

    m_focusIndex = CategoryTital;
    m_currentCategory = tempMode;
    m_navigationWidget->button(m_currentCategory)->installEventFilter(m_eventFilter);
    m_currentBox = m_currentCategory - 4;

    const int  temp = (qApp->primaryScreen()->geometry().size().width() / 2 -  LEFT_PADDING * 2 - 20) / 2 ;

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
    m_focusIndex = CategoryTital;
    m_currentCategory =  AppsListModel::AppCategory(m_currentBox + 4);
    setCategoryIndex(m_currentCategory);
    m_navigationWidget->button(m_currentCategory)->installEventFilter(m_eventFilter);
    const int  temp = (qApp->primaryScreen()->geometry().size().width() / 2 -  LEFT_PADDING * 2 - 20) / 2 ;
    m_scrollDest = dest;

    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(m_appsArea->horizontalScrollBar()->value());
    m_scrollAnimation->setEndValue(dest->x() - temp);
    m_scrollAnimation->start();

    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::setCategoryIndex(AppsListModel::AppCategory &category, int nNext)
{
    bool isScrollLeft = true;
    if (nNext > 0) isScrollLeft = true;
    else if (nNext == 0 && category < m_currentCategory)  isScrollLeft = false;
    else  if (nNext < 0) isScrollLeft = false;


    if (category < AppsListModel::Internet) category = AppsListModel::Internet;

    int categoryCount = m_calcUtil->appCategoryCount();
    for (int type = category, i = 0; i < categoryCount; i++, isScrollLeft ? type++ : type--) {
        if (nNext != 0) {
            if (type < AppsListModel::Internet && !isScrollLeft)
                category = AppsListModel::Others;
            else if (type > AppsListModel::Others) {
                category = AppsListModel::Internet ;
            }
        }

        if (m_appsManager->appNums((AppsListModel::AppCategory)type)) {
            category = (AppsListModel::AppCategory)type;
            break;
        }
    }

    AppsListModel::AppCategory tempMode = category;
    auto *dest = categoryBoxWidget(tempMode);
    if (!dest) return;

    BlurBoxWidget *leftBlurBox = nullptr;
    for (int type = tempMode - 1, i = 0; i < categoryCount; i++, type--) {
        if (type < AppsListModel::Internet) type = AppsListModel::Others;
        leftBlurBox = categoryBoxWidget((AppsListModel::AppCategory)type);
        if (m_appsManager->appNums((AppsListModel::AppCategory)type)) {
            m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX, leftBlurBox);
            leftBlurBox->setMaskVisible(true);
            leftBlurBox->layout()->itemAt(0)->setAlignment(Qt::AlignRight);
            leftBlurBox->layout()->update();
            break;
        }
    }

    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + 1, dest);
    dest->setMaskVisible(false);
    dest->layout()->itemAt(0)->setAlignment(Qt::AlignHCenter);
    dest->layout()->update();

    BlurBoxWidget *rightBlurBox = nullptr;
    for (int type = tempMode + 1, i = 0; i < categoryCount; i++, type++) {
        if (type > AppsListModel::Others) type = AppsListModel::Internet;
        rightBlurBox = categoryBoxWidget((AppsListModel::AppCategory)type);
        if (m_appsManager->appNums((AppsListModel::AppCategory)type)) {
            m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + 2, rightBlurBox);
            rightBlurBox->setMaskVisible(true);
            rightBlurBox->layout()->itemAt(0)->setAlignment(Qt::AlignLeft);
            rightBlurBox->layout()->update();
            break;
        }
    }
    m_appsHbox->adjustSize();
    if (isScrollLeft) {
        if (leftBlurBox != nullptr)
            m_appsArea->horizontalScrollBar()->setValue(leftBlurBox->x());
    } else {
        if (rightBlurBox != nullptr)
            m_appsArea->horizontalScrollBar()->setValue(rightBlurBox->x());
    }

}

void FullScreenFrame::addViewEvent(AppGridView *pView)
{
    connect(pView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(pView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(pView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(pView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, pView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
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

    m_multiPagesView->setAccessibleName("all");
    m_multiPagesView->setDataDelegate(m_appItemDelegate);
    m_multiPagesView->updatePageCount(m_appsManager->getPageCount(AppsListModel::All));
    m_multiPagesView->installEventFilter(this);

    m_multiPagesInternetView->setAccessibleName("internet");
    m_multiPagesInternetView->setDataDelegate(m_appItemDelegate);
    m_multiPagesInternetView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Internet));
    m_internetBoxWidget->setCategory(AppsListModel::Internet);
    m_multiPagesChatView->setAccessibleName("chat");
    m_multiPagesChatView->setDataDelegate(m_appItemDelegate);
    m_multiPagesChatView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Chat));
    m_chatBoxWidget->setCategory(AppsListModel::Chat);
    m_multiPagesMusicView->setAccessibleName("music");
    m_musicBoxWidget->setCategory(AppsListModel::Music);
    m_multiPagesMusicView->setDataDelegate(m_appItemDelegate);
    m_multiPagesMusicView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Music));
    m_multiPagesVideoView->setAccessibleName("video");
    m_videoBoxWidget->setCategory(AppsListModel::Video);
    m_multiPagesVideoView->setDataDelegate(m_appItemDelegate);
    m_multiPagesVideoView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Video));
    m_multiPagesGraphicsView->setAccessibleName("graphics");
    m_graphicsBoxWidget->setCategory(AppsListModel::Graphics);
    m_multiPagesGraphicsView->setDataDelegate(m_appItemDelegate);
    m_multiPagesGraphicsView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Graphics));
    m_multiPagesGameView->setAccessibleName("game");
    m_gameBoxWidget->setCategory(AppsListModel::Game);
    m_multiPagesGameView->setDataDelegate(m_appItemDelegate);
    m_multiPagesGameView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Game));
    m_multiPagesOfficeView->setAccessibleName("office");
    m_officeBoxWidget->setCategory(AppsListModel::Office);
    m_multiPagesOfficeView->setDataDelegate(m_appItemDelegate);
    m_multiPagesOfficeView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Office));
    m_multiPagesReadingView->setAccessibleName("reading");
    m_readingBoxWidget->setCategory(AppsListModel::Reading);
    m_multiPagesReadingView->setDataDelegate(m_appItemDelegate);
    m_multiPagesReadingView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Reading));
    m_multiPagesDevelopmentView->setAccessibleName("development");
    m_developmentBoxWidget->setCategory(AppsListModel::Development);
    m_multiPagesDevelopmentView->setDataDelegate(m_appItemDelegate);
    m_multiPagesDevelopmentView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Development));
    m_multiPageSystemView->setAccessibleName("system");
    m_systemBoxWidget->setCategory(AppsListModel::System);
    m_multiPageSystemView->setDataDelegate(m_appItemDelegate);
    m_multiPageSystemView->updatePageCount(m_appsManager->getPageCount(AppsListModel::System));
    m_multiPagesOthersView->setAccessibleName("others");
    m_othersBoxWidget->setCategory(AppsListModel::Others);
    m_multiPagesOthersView->setDataDelegate(m_appItemDelegate);
    m_multiPagesOthersView->updatePageCount(m_appsManager->getPageCount(AppsListModel::Others));

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

    m_appsHbox->layout()->addWidget(m_multiPagesView);

    auto titleStretch  = m_calcUtil->getAppBoxSize().width() / 2;
    m_internetBoxWidget->layoutAddWidget(m_internetTitle,  titleStretch, Qt::AlignHCenter);
    m_internetBoxWidget->layout()->addWidget(m_multiPagesInternetView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Internet, m_internetBoxWidget);

    m_chatBoxWidget->layoutAddWidget(m_chatTitle, titleStretch, Qt::AlignHCenter);
    m_chatBoxWidget->layout()->addWidget(m_multiPagesChatView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Chat, m_chatBoxWidget);

    m_musicBoxWidget->layoutAddWidget(m_musicTitle, titleStretch, Qt::AlignHCenter);
    m_musicBoxWidget->layout()->addWidget(m_multiPagesMusicView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Music, m_musicBoxWidget);

    m_videoBoxWidget->layoutAddWidget(m_videoTitle, titleStretch, Qt::AlignHCenter);
    m_videoBoxWidget->layout()->addWidget(m_multiPagesVideoView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Video, m_videoBoxWidget);

    m_graphicsBoxWidget->layoutAddWidget(m_graphicsTitle, titleStretch, Qt::AlignHCenter);
    m_graphicsBoxWidget->layout()->addWidget(m_multiPagesGraphicsView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Graphics, m_graphicsBoxWidget);

    m_gameBoxWidget->layoutAddWidget(m_gameTitle, titleStretch, Qt::AlignHCenter);
    m_gameBoxWidget->layout()->addWidget(m_multiPagesGameView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Game, m_gameBoxWidget);

    m_officeBoxWidget->layoutAddWidget(m_officeTitle, titleStretch, Qt::AlignHCenter);
    m_officeBoxWidget->layout()->addWidget(m_multiPagesOfficeView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Office, m_officeBoxWidget);

    m_readingBoxWidget->layoutAddWidget(m_readingTitle, titleStretch, Qt::AlignHCenter);
    m_readingBoxWidget->layout()->addWidget(m_multiPagesReadingView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Reading, m_readingBoxWidget);

    m_developmentBoxWidget->layoutAddWidget(m_developmentTitle, titleStretch, Qt::AlignHCenter);
    m_developmentBoxWidget->layout()->addWidget(m_multiPagesDevelopmentView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Development, m_developmentBoxWidget);

    m_systemBoxWidget->layoutAddWidget(m_systemTitle, titleStretch, Qt::AlignHCenter);
    m_systemBoxWidget->layout()->addWidget(m_multiPageSystemView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::System, m_systemBoxWidget);

    m_othersBoxWidget->layoutAddWidget(m_othersTitle, titleStretch, Qt::AlignHCenter);
    m_othersBoxWidget->layout()->addWidget(m_multiPagesOthersView);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Others, m_othersBoxWidget);

    m_appsHbox->layout()->addWidget(m_viewListPlaceholder);
    m_appsHbox->layout()->addStretch();
    m_appsHbox->layout()->setSpacing(DLauncher::APPHBOX_SPACING);
    m_appsHbox->layout()->setContentsMargins(0, DLauncher::APPS_AREA_TOP_MARGIN, 0, 0);

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

MultiPagesView *FullScreenFrame::getCategoryGridViewList(const AppsListModel::AppCategory category)
{
    MultiPagesView *view = nullptr;

    switch (category) {
    case AppsListModel::Internet:       view = m_multiPagesInternetView;      break;
    case AppsListModel::Chat:           view = m_multiPagesChatView;          break;
    case AppsListModel::Music:          view = m_multiPagesMusicView;         break;
    case AppsListModel::Video:          view = m_multiPagesVideoView;         break;
    case AppsListModel::Graphics:       view = m_multiPagesGraphicsView;      break;
    case AppsListModel::Game:           view = m_multiPagesGameView;          break;
    case AppsListModel::Office:         view = m_multiPagesOfficeView;        break;
    case AppsListModel::Reading:        view = m_multiPagesReadingView;       break;
    case AppsListModel::Development:    view = m_multiPagesDevelopmentView;   break;
    case AppsListModel::System:         view = m_multiPageSystemView;        break;
    case AppsListModel::Others:         view = m_multiPagesOthersView;        break;
//    case AppsListModel::All:            view = m_pageAppsViewList[m_pageCurrent];   break;
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

void FullScreenFrame::initConnection()
{
    connect(qApp, &QApplication::primaryScreenChanged, this, &FullScreenFrame::updateGeometry);
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &FullScreenFrame::updateGeometry);
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &FullScreenFrame::layoutChanged, Qt::QueuedConnection);
    //connect(m_scrollAnimation, &QPropertyAnimation::valueChanged, this, &FullScreenFrame::ensureScrollToDest);
    connect(m_scrollAnimation, &QPropertyAnimation::finished, this, &FullScreenFrame::refershCurrentFloatTitle, Qt::QueuedConnection);
    connect(m_navigationWidget, &NavigationWidget::scrollToCategory, this, &FullScreenFrame::scrollToCategory);

    connect(this, &FullScreenFrame::currentVisibleCategoryChanged, m_navigationWidget, &NavigationWidget::setCurrentCategory);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, m_navigationWidget, &NavigationWidget::refershCategoryVisible);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, this, &FullScreenFrame::refershCategoryVisible);
    connect(this, &FullScreenFrame::displayModeChanged, this, &FullScreenFrame::checkCategoryVisible);
    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &FullScreenFrame::searchTextChanged);
    connect(m_delayHideTimer, &QTimer::timeout, this, &FullScreenFrame::hide, Qt::QueuedConnection);


    connect(m_clearCacheTimer, &QTimer::timeout, m_appsManager, &AppsManager::clearCache);

    // auto scroll when drag to app list box border
//    connect(m_pageAppsViewList[0], &AppGridView::requestScrollStop, m_autoScrollTimer, &QTimer::stop);
//    connect(m_autoScrollTimer, &QTimer::timeout, [this] {
//        m_appsArea->horizontalScrollBar()->setValue(m_appsArea->horizontalScrollBar()->value() + m_autoScrollStep);
//    });
//    connect(m_pageAppsViewList[0], &AppGridView::requestScrollUp, [this] {
//        m_autoScrollStep = -DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
//        if (!m_autoScrollTimer->isActive())
//            m_autoScrollTimer->start();
//    });
//    connect(m_pageAppsViewList[0], &AppGridView::requestScrollDown, [this] {
//        m_autoScrollStep = DLauncher::APPS_AREA_AUTO_SCROLL_STEP;
//        if (!m_autoScrollTimer->isActive())
//            m_autoScrollTimer->start();
//    });

//    for (int i=0; i<m_appsManager->getPageCount(); i++) {
//        connect(m_multiPagesView->pageView(i), &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
//        connect(m_multiPagesView->pageView(i), &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
//        connect(m_multiPagesView->pageView(i), &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
//        connect(m_multiPagesView->pageView(i), &AppGridView::clicked, this, &FullScreenFrame::hide);
//        connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, m_multiPagesView->pageView(i), static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
//    }

    for (int i = 0; i < CATEGORY_MAX; i++) {
        for (int j = 0; j < m_appsManager->getPageCount(AppsListModel::AppCategory(i + 4)); j++) {
            connect(getCategoryGridViewList(AppsListModel::AppCategory(i + 4))->pageView(j), &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
            connect(getCategoryGridViewList(AppsListModel::AppCategory(i + 4))->pageView(j), &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
            connect(getCategoryGridViewList(AppsListModel::AppCategory(i + 4))->pageView(j), &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
            connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, getCategoryGridViewList(AppsListModel::AppCategory(i + 4))->pageView(j), static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
        }
    }

    connect(m_internetBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_chatBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_musicBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_videoBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_graphicsBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_gameBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_officeBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_readingBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_developmentBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_systemBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);
    connect(m_othersBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::scrollToCategory);

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
    connect(m_appsManager, &AppsManager::dataChanged, [this] {reflashPageView();});
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
        nextTabWidget(key);
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

    if (m_focusIndex == CategoryTital) {
        switch (key) {
        case Qt::Key_Backtab:
        case Qt::Key_Left: {
            int nextCategory = m_currentCategory - 1;
            if (nextCategory < AppsListModel::Internet) nextCategory = AppsListModel::Others;
            scrollToCategory(AppsListModel::AppCategory(nextCategory), -1);
            return;
        }
        case Qt::Key_Right: {
            int nextCategory = m_currentCategory + 1;
            if (nextCategory > AppsListModel::Others) nextCategory = AppsListModel::Internet;
            scrollToCategory(AppsListModel::AppCategory(nextCategory), 1);
            return;
        }
        case Qt::Key_Down:  m_focusIndex = FirstItem;  break;
        default:;
        }
    }
    const QModelIndex currentIndex = m_appItemDelegate->currentIndex();
    // move operation should be start from a valid location, if not, just init it.
    if (!currentIndex.isValid()) {
        if (m_currentCategory < AppsListModel::Internet)
            m_currentCategory = AppsListModel::Internet;

        if (m_displayMode == GROUP_BY_CATEGORY)
            m_appItemDelegate->setCurrentIndex(getCategoryGridViewList(m_currentCategory)->getAppItem(0));
        else
            m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(0));

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
    case Qt::Key_Right:    index = currentIndex.sibling(currentIndex.row() + 1, 0);        break;
    case Qt::Key_Up:        index = currentIndex.sibling(currentIndex.row() - column, 0);   break;
    case Qt::Key_Down:   index = currentIndex.sibling(currentIndex.row() + column, 0);   break;
    default:;
    }

    //to next page
    if (m_displayMode == ALL_APPS && !index.isValid()) {
        index = m_multiPagesView->selectApp(key);
        index = index.isValid() ? index : currentIndex;
    }

    // now, we need to check and fix if destination is invalid.
    do {
        if (index.isValid())
            break;

        // the column number of destination, when moving up/down, columns shouldn't be changed.
        //const int realColumn = currentIndex.row() % column;

        // const AppsListModel *model = static_cast<const AppsListModel *>(currentIndex.model());

        if (key == Qt::Key_Down || key == Qt::Key_Right) {
            int currentIndex = m_appsManager->getPageIndex(m_currentCategory);
            if (m_appsManager->getPageCount(m_currentCategory) != (currentIndex + 1)) {
                getCategoryGridViewList(m_currentCategory)->showCurrentPage(++currentIndex);
                auto index = getCategoryGridViewList(m_currentCategory)->getAppItem(0);
                if (!index.isValid()) return;
                m_appItemDelegate->setCurrentIndex(index);
                update();
                return;
            }
        } else if (key == Qt::Key_Up || key == Qt::Key_Left) {
            int currentIndex = m_appsManager->getPageIndex(m_currentCategory);
            if (0 < currentIndex) {
                getCategoryGridViewList(m_currentCategory)->showCurrentPage(--currentIndex);
                auto index = getCategoryGridViewList(m_currentCategory)->getAppItem(0);
                if (!index.isValid()) return;
                m_appItemDelegate->setCurrentIndex(index);
                update();
                return;
            }
        }
    } while (false);

    // valid verify and UI adjustment.
    const QModelIndex selectedIndex = index.isValid() ? index : currentIndex;
    m_appItemDelegate->setCurrentIndex(selectedIndex);
    //ensureItemVisible(selectedIndex);
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
    case ALL_APPS:            m_appsManager->launchApp(m_multiPagesView->getAppItem(0));     break;
//    case ALL_APPS:            m_appsManager->launchApp(m_pageAppsViewList[m_pageCurrent]->indexAt(0));     break;
    case GROUP_BY_CATEGORY:   m_appsManager->launchApp(getCategoryGridViewList(m_currentCategory)->getAppItem(0));    break;
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
    for (int i = 0; i < m_appsManager->getPageCount(AppsListModel::All); i++) {
        uninstallApp(m_multiPagesView->pageModel(i)->indexAt(appKey));
    }

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

void FullScreenFrame::clickToCategory(const QModelIndex &index)
{
    qDebug() << "modeValue" <<  index.data(AppsListModel::AppCategoryRole).value<AppsListModel::AppCategory>();
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
    MultiPagesView *view = nullptr;
    const AppsListModel::AppCategory category = index.data(AppsListModel::AppCategoryRole).value<AppsListModel::AppCategory>();

//   if (m_displayMode == SEARCH || m_displayMode == ALL_APPS)
//        view = m_pageAppsViewList[m_pageCurrent];
//    else
    if (m_displayMode == GROUP_BY_CATEGORY)
        view = getCategoryGridViewList(category);

    if (!view)
        return;

    // m_appsArea->ensureVisible(0, view->indexYOffset(index) + view->pos().y(), 0, DLauncher::APPS_AREA_ENSURE_VISIBLE_MARGIN_Y);
    updateCurrentVisibleCategory();
    if (category != m_currentCategory) {
        scrollToCategory(category);
    }
}

void FullScreenFrame::reflashPageView()
{
    m_multiPagesView->updatePageCount(m_appsManager->getPageCount(AppsListModel::All));
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

    m_multiPagesView->setVisible(!isCategoryMode);

    m_internetTitle->setVisible(isCategoryMode);
    m_multiPagesInternetView->setVisible(isCategoryMode);
    m_internetBoxWidget->setVisible(isCategoryMode);
    m_chatTitle->setVisible(isCategoryMode);
    m_multiPagesChatView->setVisible(isCategoryMode);
    m_chatBoxWidget->setVisible(isCategoryMode);
    m_musicTitle->setVisible(isCategoryMode);
    m_multiPagesMusicView->setVisible(isCategoryMode);
    m_musicBoxWidget->setVisible(isCategoryMode);
    m_videoTitle->setVisible(isCategoryMode);
    m_multiPagesVideoView->setVisible(isCategoryMode);
    m_videoBoxWidget->setVisible(isCategoryMode);
    m_graphicsTitle->setVisible(isCategoryMode);
    m_multiPagesGraphicsView->setVisible(isCategoryMode);
    m_graphicsBoxWidget->setVisible(isCategoryMode);
    m_gameTitle->setVisible(isCategoryMode);
    m_multiPagesGameView->setVisible(isCategoryMode);
    m_gameBoxWidget->setVisible(isCategoryMode);
    m_officeTitle->setVisible(isCategoryMode);
    m_multiPagesOfficeView->setVisible(isCategoryMode);
    m_officeBoxWidget->setVisible(isCategoryMode);
    m_readingTitle->setVisible(isCategoryMode);
    m_multiPagesReadingView->setVisible(isCategoryMode);
    m_readingBoxWidget->setVisible(isCategoryMode);
    m_developmentTitle->setVisible(isCategoryMode);
    m_multiPagesDevelopmentView->setVisible(isCategoryMode);
    m_developmentBoxWidget->setVisible(isCategoryMode);
    m_systemTitle->setVisible(isCategoryMode);
    m_multiPageSystemView->setVisible(isCategoryMode);
    m_systemBoxWidget->setVisible(isCategoryMode);
    m_othersTitle->setVisible(isCategoryMode);
    m_multiPagesOthersView->setVisible(isCategoryMode);
    m_othersBoxWidget->setVisible(isCategoryMode);

    m_viewListPlaceholder->setVisible(isCategoryMode);
    m_navigationWidget->setVisible(isCategoryMode);

    m_multiPagesView->setSearchModel(m_searchResultModel, m_displayMode == SEARCH);
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
    if (!m_multiPagesInternetView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Internet;
    else if (!m_multiPagesChatView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Chat;
    else if (!m_multiPagesMusicView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Music;
    else if (!m_multiPagesVideoView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Video;
    else if (!m_multiPagesGraphicsView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Graphics;
    else if (!m_multiPagesGameView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Game;
    else if (!m_multiPagesOfficeView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Office;
    else if (!m_multiPagesReadingView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Reading;
    else if (!m_multiPagesDevelopmentView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Development;
    else if (!m_multiPageSystemView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::System;
    else if (!m_multiPagesOthersView->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Others;

    if (m_currentCategory == currentVisibleCategory)
        return;

    m_currentCategory = currentVisibleCategory;

    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::updatePlaceholderSize()
{
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
    AppsListModel *nextModel = m_internetModel;
    AppsListModel::AppCategory categoty = currentModel->category();

    if (categoty < AppsListModel::Internet)
        nextModel =  m_internetModel;
    else if (categoty == AppsListModel::Internet)
        nextModel =   m_chatModel;
    else if (categoty == AppsListModel::Chat)
        nextModel =   m_musicModel;
    else if (categoty == AppsListModel::Music)
        nextModel =   m_videoModel;
    else if (categoty == AppsListModel::Video)
        nextModel =   m_graphicsModel;
    else if (categoty == AppsListModel::Graphics)
        nextModel =   m_gameModel;
    else if (categoty == AppsListModel::Game)
        nextModel =   m_officeModel;
    else if (categoty == AppsListModel::Office)
        nextModel =   m_readingModel;
    else  if (categoty == AppsListModel::Reading)
        nextModel =   m_developmentModel;
    else if (categoty == AppsListModel::Development)
        nextModel =   m_systemModel;
    else if (categoty == AppsListModel::System)
        nextModel =   m_othersModel;
    else if (categoty == AppsListModel::Others)
        nextModel =   m_internetModel;
    else {
        nextModel = m_internetModel;
    }
    if (!m_appsManager->appNums(nextModel->category())) {
        nextModel = nextCategoryModel(nextModel);
    }
    return nextModel;
}

void FullScreenFrame::nextTabWidget(int key)
{
    if (Qt::Key_Backtab == key) {
        m_focusIndex--;
        if (m_displayMode == GROUP_BY_CATEGORY) {
            if (m_focusIndex < FirstItem) m_focusIndex = CategoryTital;
        } else {
            if (m_focusIndex < FirstItem) m_focusIndex = CategoryChangeBtn;
        }
    } else if (Qt::Key_Tab == key) {
        m_focusIndex++;
        if (m_displayMode == GROUP_BY_CATEGORY) {
            if (m_focusIndex > CategoryTital) m_focusIndex = FirstItem;
        } else {
            if (m_focusIndex > CategoryChangeBtn) m_focusIndex = FirstItem;
        }
    }

    switch (m_focusIndex) {
    case FirstItem: {
        m_searchWidget->categoryBtn()->clearFocus();
        if (m_currentCategory < AppsListModel::Internet)
            m_currentCategory = AppsListModel::Internet;

//        AppGridView *pView = (m_displayMode == GROUP_BY_CATEGORY) ? categoryView(m_currentCategory) : m_pageAppsViewList[m_pageCurrent];
//        m_appItemDelegate->setCurrentIndex(pView->indexAt(0));
        if (m_displayMode == GROUP_BY_CATEGORY)
            m_appItemDelegate->setCurrentIndex(getCategoryGridViewList(m_currentCategory)->getAppItem(0));
        else
            m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(0));

        update();
        m_navigationWidget->setCancelCurrentCategory(m_currentCategory);
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
        m_navigationWidget->setCancelCurrentCategory(m_currentCategory);

    }
    break;
    case CategoryTital: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        if (m_currentCategory < AppsListModel::Internet) m_currentCategory = AppsListModel::Internet;
        m_navigationWidget->setCurrentCategory(m_currentCategory);
        m_navigationWidget->button(m_currentCategory)->setFocus();
    }
    break;
    }
}

AppsListModel *FullScreenFrame::prevCategoryModel(const AppsListModel *currentModel)
{
    AppsListModel *prevModel = m_othersModel;
    AppsListModel::AppCategory categoty = currentModel->category();

    if (categoty < AppsListModel::Internet)
        prevModel = m_othersModel;
    else if (categoty == AppsListModel::Others)
        prevModel =  m_internetModel;
    else  if (categoty == AppsListModel::Music)
        prevModel =  m_chatModel;
    else  if (categoty == AppsListModel::Video)
        prevModel =  m_musicModel;
    else if (categoty == AppsListModel::Graphics)
        prevModel =  m_videoModel;
    else  if (categoty == AppsListModel::Game)
        prevModel =  m_graphicsModel;
    else if (categoty == AppsListModel::Office)
        prevModel =  m_gameModel;
    else  if (categoty == AppsListModel::Reading)
        prevModel =  m_officeModel;
    else if (categoty == AppsListModel::Development)
        prevModel =  m_readingModel;
    else if (categoty == AppsListModel::System)
        prevModel =  m_developmentModel;
    else if (categoty == AppsListModel::Others)
        prevModel =  m_systemModel;
    else {
        prevModel = m_othersModel;
    }
    if (!m_appsManager->appNums(prevModel->category())) {
        prevModel = prevCategoryModel(prevModel);
    }
    return prevModel;
}

void FullScreenFrame::layoutChanged()
{
    QSize boxSize;
    if (m_displayMode == ALL_APPS || m_displayMode == SEARCH) {
        const int appsContentWidth = (width() - LEFT_PADDING - RIGHT_PADDING);
        boxSize.setWidth(appsContentWidth);
        boxSize.setHeight(m_appsArea->height() - m_topSpacing->height());
        m_multiPagesView->setFixedSize(boxSize);
    } else {
        boxSize = m_calcUtil->getAppBoxSize();
    }

    m_appsHbox->setFixedHeight(m_appsArea->height());

    m_internetBoxWidget->setMaskSize(boxSize);
    m_chatBoxWidget->setMaskSize(boxSize);
    m_musicBoxWidget->setMaskSize(boxSize);
    m_videoBoxWidget->setMaskSize(boxSize);
    m_graphicsBoxWidget->setMaskSize(boxSize);
    m_gameBoxWidget->setMaskSize(boxSize);
    m_officeBoxWidget->setMaskSize(boxSize);
    m_readingBoxWidget->setMaskSize(boxSize);
    m_developmentBoxWidget->setMaskSize(boxSize);
    m_systemBoxWidget->setMaskSize(boxSize);
    m_othersBoxWidget->setMaskSize(boxSize);

    for (int i = 0; i < CATEGORY_MAX; i++) {
        for (int j = 0; j < m_appsManager->getPageCount(AppsListModel::AppCategory(i + 4)); j++) {
            getCategoryGridViewList(AppsListModel::AppCategory(i + 4))->pageView(j)->setFixedHeight(boxSize.width());
        }
    }

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

