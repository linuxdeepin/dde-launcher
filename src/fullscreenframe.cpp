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
#include <DSearchEdit>
#include <DFloatingButton>

#include <ddialog.h>
#include <QScroller>

#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

#include "sharedeventfilter.h"

#define FIRST_APP_INDEX 0

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

    m_internetBoxWidget(new BlurBoxWidget(AppsListModel::Internet, const_cast<char *>("Internet"))),
    m_chatBoxWidget(new BlurBoxWidget(AppsListModel::Chat, const_cast<char *>("Chat"))),
    m_musicBoxWidget(new BlurBoxWidget(AppsListModel::Music, const_cast<char *>("Music"))),
    m_videoBoxWidget(new BlurBoxWidget(AppsListModel::Video, const_cast<char *>("Video"))),
    m_graphicsBoxWidget(new BlurBoxWidget(AppsListModel::Graphics, const_cast<char *>("Graphics"))),
    m_gameBoxWidget(new BlurBoxWidget(AppsListModel::Game, const_cast<char *>("Game"))),
    m_officeBoxWidget(new BlurBoxWidget(AppsListModel::Office, const_cast<char *>("Office"))),
    m_readingBoxWidget(new BlurBoxWidget(AppsListModel::Reading, const_cast<char *>("Reading"))),
    m_developmentBoxWidget(new BlurBoxWidget(AppsListModel::Development, const_cast<char *>("Development"))),
    m_systemBoxWidget(new BlurBoxWidget(AppsListModel::System, const_cast<char *>("System"))),
    m_othersBoxWidget(new BlurBoxWidget(AppsListModel::Others, const_cast<char *>("Others"))),

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
    m_topSpacing(new QFrame)
    , m_bottomSpacing(new QFrame)
    , m_contentFrame(new QFrame)
    , m_displayInter(new DBusDisplay(this))
{
    m_focusIndex = 0;
    m_padding = 200;
    //m_currentCategory = AppsListModel::Internet;
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
    m_mouse_press = false;
    m_mouse_press_time = 0;
    m_appsAreaHScrollBarValue = 0;
    setAttribute(Qt::WA_InputMethodEnabled, true);
    m_nextFocusIndex = Applist;
    m_currentFocusIndex = m_nextFocusIndex;
    m_appNum = m_appsManager->GetAllAppNum();

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
    for (int i = 0; i < CATEGORY_MAX; i++)
        connect(getCategoryGridViewList(AppsListModel::AppCategory(i + 4)), &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);

    initUI();
    initConnection();

    updateDisplayMode(m_calcUtil->displayMode());
    updateDockPosition();
    setFixedSize(QSize(m_displayInter->primaryRect().width, m_displayInter->primaryRect().height)/qApp->primaryScreen()->devicePixelRatio());

//    QScroller::grabGesture(m_appsArea->viewport(), QScroller::TouchGesture);
//    QScroller *scroller = QScroller::scroller(m_appsArea->viewport());
//    QScrollerProperties sp;
//    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
//    scroller->setScrollerProperties(sp);
}

FullScreenFrame::~FullScreenFrame()
{
    QScroller *scroller = QScroller::scroller(m_appsArea->viewport());
    if (scroller) {
        scroller->stop();
    }
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

    m_currentCategory = tempMode;
    m_navigationWidget->button(m_currentCategory)->installEventFilter(m_eventFilter);
    m_currentBox = m_currentCategory - 4;

    const int  temp = (qApp->primaryScreen()->geometry().size().width() / 2 -  m_padding * 2 - 20) / 2 ;

    m_scrollDest = dest;
    int endValue = dest->x() - temp;
    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(m_appsArea->horizontalScrollBar()->value());
    m_scrollAnimation->setEndValue(endValue);
    m_scrollAnimation->start();

    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::scrollToBlurBoxWidget(BlurBoxWidget *category, int nNext)
{
    QWidget *dest = category;

    if (!dest)
        return;

    m_currentCategory =  AppsListModel::AppCategory(m_currentBox + 4);

    setCategoryIndex(m_currentCategory, nNext);

    m_navigationWidget->button(m_currentCategory)->installEventFilter(m_eventFilter);
    const int  temp = (qApp->primaryScreen()->geometry().size().width() / 2 -  m_padding * 2 - 20) / 2 ;
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
            } else {
                category = AppsListModel::AppCategory(type);
            }
        }

        if (m_appsManager->appNums(AppsListModel::AppCategory(category))) {
            category = AppsListModel::AppCategory(category);
            break;
        }
    }

    AppsListModel::AppCategory tempMode = category;
    auto *dest = categoryBoxWidget(tempMode);
    if (!dest) return;

    BlurBoxWidget *leftBlurBox = nullptr;
    for (int type = tempMode - 1, i = 0; i < categoryCount; i++, type--) {
        if (type < AppsListModel::Internet) type = AppsListModel::Others;
        leftBlurBox = categoryBoxWidget(AppsListModel::AppCategory(type));
        if (m_appsManager->appNums(AppsListModel::AppCategory(type))) {
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
        rightBlurBox = categoryBoxWidget(AppsListModel::AppCategory(type));
        if (m_appsManager->appNums(AppsListModel::AppCategory(type))) {
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
        //        updateDockPosition();
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
            m_searchWidget->edit()->lineEdit()->setText(clipboardText);
            m_searchWidget->edit()->lineEdit()->setFocus();
            m_focusIndex = SearchEdit;
        }
    }
}

void FullScreenFrame::showEvent(QShowEvent *e)
{
    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();
    //updateCurrentVisibleCategory();
    // TODO: Do we need this in showEvent ???
    //XcbMisc::instance()->set_deepin_override(winId());
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
        m_searchWidget->raise();
        emit visibleChanged(true);
    });

    m_clearCacheTimer->stop();
}

void FullScreenFrame::hideEvent(QHideEvent *e)
{
    BoxFrame::hideEvent(e);

    QTimer::singleShot(1, this, [ = ] { emit visibleChanged(false); });

    m_clearCacheTimer->start();
}

void FullScreenFrame::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || m_displayMode != GROUP_BY_CATEGORY)
        return;
    m_mouse_press = true;
    m_appsAreaHScrollBarValue = m_appsArea->horizontalScrollBar()->value();
    m_mouse_press_time =  QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_mousePos = e->pos();
}

void FullScreenFrame::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_mouse_press  || e->button() == Qt::RightButton) {
        return;
    }

    int move_diff = e->pos().x() - m_mousePos.x();
    int limit_move = m_calcUtil->getAppBoxSize().width() -  DLauncher::MOUSE_MOVE_TO_NEXT;
    if (move_diff  > limit_move)  move_diff = limit_move;
    if (move_diff < -limit_move) move_diff = -limit_move;
    m_appsArea->horizontalScrollBar()->setValue(m_appsAreaHScrollBarValue - move_diff);
}

void FullScreenFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || m_displayMode != GROUP_BY_CATEGORY)
        return;

    if (m_mousePos == e->pos()) {
        hide();
    } else {
        qint64 mouse_release_time =  QDateTime::currentDateTime().toMSecsSinceEpoch();
        int move_diff   = e->pos().x() - m_mousePos.x();
        //快速滑动
        if (mouse_release_time - m_mouse_press_time <= DLauncher::MOUSE_PRESS_TIME_DIFF) {
            if (move_diff > 0) {
                //快速长距离滑动
                if (move_diff > DLauncher::MOUSE_MOVE_TO_NEXT * 2) {
                    int targetCategory = prevCategoryModel(prevCategoryModel(m_currentCategory));
                    scrollToCategory(AppsListModel::AppCategory(targetCategory), -1);
                } else {
                    scrollToCategory(prevCategoryModel(m_currentCategory), -1);
                }
            } else {
                if (move_diff < -DLauncher::MOUSE_MOVE_TO_NEXT * 2) {
                    int targetCategory = nextCategoryModel(nextCategoryModel(m_currentCategory));
                    scrollToCategory(AppsListModel::AppCategory(targetCategory), 1);
                } else {
                    scrollToCategory(nextCategoryModel(m_currentCategory), 1);
                }
            }
        } else {

            if (move_diff > DLauncher::MOUSE_MOVE_TO_NEXT) {
                scrollToCategory(prevCategoryModel(m_currentCategory), -1);
            } else if (move_diff < -DLauncher::MOUSE_MOVE_TO_NEXT) {
                scrollToCategory(nextCategoryModel(m_currentCategory), 1);
            } else {
                scrollToCategory((m_currentCategory), move_diff > 0 ? 1 : -1);
            }
        }
    }
    m_mouse_press = false;
}

void FullScreenFrame::wheelEvent(QWheelEvent *e)
{

    if (m_displayMode == GROUP_BY_CATEGORY) {
        if (m_scrollAnimation->state() == m_scrollAnimation->Running) return;
        int nNext = 0;
        static int  wheelTime = 0;
        if (e->angleDelta().y() < 0) {
            wheelTime++;
            nNext = 1;
        } else {
            wheelTime--;
            nNext = -1;
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

            scrollToBlurBoxWidget(m_BoxWidget[m_currentBox], nNext);
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
    if (o == m_searchWidget->edit()->lineEdit() && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyPress = static_cast<QKeyEvent *>(e);
        if (keyPress->key() == Qt::Key_Left || keyPress->key() == Qt::Key_Right) {
            QKeyEvent *event = new QKeyEvent(keyPress->type(), keyPress->key(), keyPress->modifiers());
            qApp->postEvent(this, event);
            return true;
        }
    } else if ((o == m_appsArea->viewport() && e->type() == QEvent::Wheel)
               || (o == m_appsArea && e->type() == QEvent::Scroll)) {

    } else if (o == m_appsArea->viewport() && e->type() == QEvent::Resize) {
        updateDockPosition();
        updatePlaceholderSize();
    }
    return false;
}

void FullScreenFrame::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->setText(e->commitString());
        m_searchWidget->edit()->lineEdit()->setFocus();
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
    default:;
    }
    return QWidget::inputMethodQuery(prop);
}

void FullScreenFrame::enterEvent(QEvent* event)
{
    updateFrameCursor();
    BoxFrame::enterEvent(event);
}

void FullScreenFrame::initUI()
{
    m_searchWidget->showToggle();

    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(500, 50);
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setStyleSheet("color:rgba(238, 238, 238, .6);"
                               //                               "background-color:red;"
                               "font-size:22px;");

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
    m_multiPagesView->updatePageCount(AppsListModel::All);
    m_multiPagesView->installEventFilter(this);

    m_internetBoxWidget->setDataDelegate(m_appItemDelegate);
    m_chatBoxWidget->setDataDelegate(m_appItemDelegate);
    m_musicBoxWidget->setDataDelegate(m_appItemDelegate);
    m_videoBoxWidget->setDataDelegate(m_appItemDelegate);
    m_graphicsBoxWidget->setDataDelegate(m_appItemDelegate);
    m_gameBoxWidget->setDataDelegate(m_appItemDelegate);
    m_officeBoxWidget->setDataDelegate(m_appItemDelegate);
    m_readingBoxWidget->setDataDelegate(m_appItemDelegate);
    m_developmentBoxWidget->setDataDelegate(m_appItemDelegate);
    m_systemBoxWidget->setDataDelegate(m_appItemDelegate);
    m_othersBoxWidget->setDataDelegate(m_appItemDelegate);

    m_appsHbox->layout()->addWidget(m_multiPagesView);

    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Internet, m_internetBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Chat, m_chatBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Music, m_musicBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Video, m_videoBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Graphics, m_graphicsBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Game, m_gameBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Office, m_officeBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Reading, m_readingBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::Development, m_developmentBoxWidget);
    m_appsHbox->layout()->insertWidget(DLauncher::APPS_AREA_CATEGORY_INDEX + AppsListModel::System, m_systemBoxWidget);
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
    QSize screen = m_calcUtil->getScreenSize();
    m_padding = screen.width() / 10;
    scrollHLayout->setContentsMargins(m_padding, 0, m_padding, 0);
    scrollHLayout->setSpacing(0);
    scrollHLayout->addWidget(m_appsHbox, 0, Qt::AlignTop);
    m_pHBoxLayout = scrollHLayout;

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
}

MultiPagesView *FullScreenFrame::getCategoryGridViewList(const AppsListModel::AppCategory category)
{
    MultiPagesView *view = nullptr;

    switch (category) {
    case AppsListModel::Internet:       view = m_internetBoxWidget->getMultiPagesView();      break;
    case AppsListModel::Chat:           view = m_chatBoxWidget->getMultiPagesView();          break;
    case AppsListModel::Music:          view = m_musicBoxWidget->getMultiPagesView();         break;
    case AppsListModel::Video:          view = m_videoBoxWidget->getMultiPagesView();         break;
    case AppsListModel::Graphics:       view = m_graphicsBoxWidget->getMultiPagesView();      break;
    case AppsListModel::Game:           view = m_gameBoxWidget->getMultiPagesView();          break;
    case AppsListModel::Office:         view = m_officeBoxWidget->getMultiPagesView();        break;
    case AppsListModel::Reading:        view = m_readingBoxWidget->getMultiPagesView();       break;
    case AppsListModel::Development:    view = m_developmentBoxWidget->getMultiPagesView();   break;
    case AppsListModel::System:         view = m_systemBoxWidget->getMultiPagesView();        break;
    case AppsListModel::Others:         view = m_othersBoxWidget->getMultiPagesView();        break;
    //    case AppsListModel::All:            view = m_pageAppsViewList[m_pageCurrent];   break;
    default: view = m_internetBoxWidget->getMultiPagesView();
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
    default: view = m_internetBoxWidget;
    }

    return view;
}

void FullScreenFrame::initConnection()
{
    connect(qApp, &QApplication::primaryScreenChanged, this, &FullScreenFrame::updateGeometry);
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &FullScreenFrame::updateGeometry);
    connect(m_displayInter, &DBusDisplay::PrimaryChanged, this, &FullScreenFrame::updateGeometry, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &FullScreenFrame::updateGeometry, Qt::QueuedConnection);

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &FullScreenFrame::layoutChanged, Qt::QueuedConnection);
    //connect(m_scrollAnimation, &QPropertyAnimation::valueChanged, this, &FullScreenFrame::ensureScrollToDest);
    connect(m_navigationWidget, &NavigationWidget::scrollToCategory, this, &FullScreenFrame::scrollToCategory);

    connect(this, &FullScreenFrame::currentVisibleCategoryChanged, m_navigationWidget, &NavigationWidget::setCurrentCategory);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, m_navigationWidget, &NavigationWidget::refershCategoryVisible);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, this, &FullScreenFrame::refershCategoryVisible);
    connect(this, &FullScreenFrame::displayModeChanged, this, &FullScreenFrame::checkCategoryVisible);
    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &FullScreenFrame::searchTextChanged);
    connect(m_delayHideTimer, &QTimer::timeout, this, &FullScreenFrame::hide, Qt::QueuedConnection);
    connect(m_clearCacheTimer, &QTimer::timeout, m_appsManager, &AppsManager::clearCache);

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
    connect(m_appsManager, &AppsManager::dataChanged, this, &FullScreenFrame::reflashPageView);

    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::ScreenHeightChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::ScreenWidthChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::PrimaryChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
}

void FullScreenFrame::showLauncher()
{
    if (m_firstStart && qApp->primaryScreen()->devicePixelRatio() != 1) {
        show();
        hide();
        m_firstStart = false;
    }

    m_focusIndex = 1;
    m_appItemDelegate->setCurrentIndex(QModelIndex());
    m_searchWidget->categoryBtn()->clearFocus();
    show();
    setFixedSize(QSize(m_displayInter->primaryRect().width, m_displayInter->primaryRect().height)/qApp->primaryScreen()->devicePixelRatio());
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
    qDebug()<<rect;

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
    //Prevent the application from being added or uninstalled. Each time you go to get the total number
    m_appNum = m_appsManager->GetAllAppNum();
    if (Qt::Key_Tab == key || Qt::Key_Backtab == key) {
        nextTabWidget(key);
        return;
    }

    if (Qt::Key_Undo == key) {
        auto  oldStr =  m_searchWidget->edit()->lineEdit()->text();
        m_searchWidget->edit()->lineEdit()->undo();
        if (!oldStr.isEmpty() && oldStr == m_searchWidget->edit()->lineEdit()->text()) {
            m_searchWidget->edit()->lineEdit()->clear();
        }
        return;
    }

    if (Qt::Key_Space == key) {
        if (m_searchWidget->categoryBtn()->hasFocus() || m_focusIndex == CategoryChangeBtn) {
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
            scrollToCategory(prevCategoryModel(m_currentCategory), -1);
            return;
        }
        case Qt::Key_Right: {
            scrollToCategory(nextCategoryModel(m_currentCategory), 1);
            return;
        }
        case Qt::Key_Down: {
            m_focusIndex = FirstItem;
        } break;
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
    // When it is in the first one, another operation will let you select the last app
    case Qt::Key_Left:      index = currentIndex.sibling((currentIndex.row() == FIRST_APP_INDEX ? m_appNum : currentIndex.row()) - 1, 0);       break;
    case Qt::Key_Tab:
    //When it is at the end, another operation will let you select the first app
    case Qt::Key_Right:     index = currentIndex.sibling( (currentIndex.row() + 1) == m_appNum ? FIRST_APP_INDEX : currentIndex.row() + 1, 0);        break;
    case Qt::Key_Up:
        if( currentIndex.row() - column < 0 && currentIndex.row() > FIRST_APP_INDEX ) {
            index = currentIndex.sibling(FIRST_APP_INDEX, 0);
        } else if( currentIndex.row() == FIRST_APP_INDEX ) {
            index = currentIndex.sibling(m_appNum - 1, 0);
        } else {
            index = currentIndex.sibling(currentIndex.row() - column, 0);
        }
        break;
    case Qt::Key_Down:
        if( currentIndex.row() + column > m_appNum - 1 && currentIndex.row() < m_appNum - 1 ) {
            index = currentIndex.sibling(m_appNum - 1, 0);
        } else if(currentIndex.row() == m_appNum - 1) {
            index = currentIndex.sibling(FIRST_APP_INDEX, 0);
        } else {
            index = currentIndex.sibling(currentIndex.row() + column, 0);
        }
        break;
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
        int nAppIndex = 0;

        if (key == Qt::Key_Down || key == Qt::Key_Right) {
            int currentIndex = getCategoryGridViewList(m_currentCategory)->currentPage();
            if (m_appsManager->getPageCount(m_currentCategory) != (currentIndex + 1))
                getCategoryGridViewList(m_currentCategory)->showCurrentPage(++currentIndex);
            else
                getCategoryGridViewList(m_currentCategory)->showCurrentPage(0);

            nAppIndex = 0;
        } else if (key == Qt::Key_Up || key == Qt::Key_Left) {
            int currentIndex = getCategoryGridViewList(m_currentCategory)->currentPage();
            if (0 < currentIndex)
                getCategoryGridViewList(m_currentCategory)->showCurrentPage(--currentIndex);
            else
                getCategoryGridViewList(m_currentCategory)->showCurrentPage(m_appsManager->getPageCount(m_currentCategory) - 1);

            currentIndex = getCategoryGridViewList(m_currentCategory)->currentPage();
            nAppIndex = getCategoryGridViewList(m_currentCategory)->pageModel(currentIndex)->rowCount(QModelIndex()) - 1;
        }
        index = getCategoryGridViewList(m_currentCategory)->getAppItem(nAppIndex);

    } while (false);

    // valid verify and UI adjustment.
    const QModelIndex selectedIndex = index.isValid() ? index : currentIndex;
    m_appItemDelegate->setCurrentIndex(selectedIndex);
    //    ensureItemVisible(selectedIndex);
    update();
}

void FullScreenFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->lineEdit()->setFocus(Qt::MouseFocusReason);

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

    if (m_currentFocusIndex == Category) {
        emit m_searchWidget->categoryBtn()->clicked();
    }
    if (m_searchWidget->categoryBtn()->hasFocus() ||  m_focusIndex == CategoryChangeBtn) {
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
     }else {
        int currentPage =  categoryBoxWidget(m_currentCategory)->getMultiPagesView()->currentPage();
        uninstallApp(categoryBoxWidget(m_currentCategory)->getMultiPagesView()->pageModel(currentPage)->indexAt(appKey));
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
    unInstallDialog.setTitle(QString(tr("Are you sure you want to uninstall?")));
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

    unInstallDialog.show();
    unInstallDialog.moveToCenter();
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

    updateCurrentVisibleCategory();
    if (category != m_currentCategory) {
        scrollToCategory(category);
    }
}


void FullScreenFrame::reflashPageView(AppsListModel::AppCategory category)
{
    if (AppsListModel::Search == category) {
        m_multiPagesView->ShowPageView((AppsListModel::AppCategory)m_displayMode);
    } else {
        m_multiPagesView->updatePageCount(category);
    }
}

void FullScreenFrame::primaryScreenChanged()
{
    setFixedSize(qApp->primaryScreen()->geometry().size());
    updateDockPosition();
}

void FullScreenFrame::refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums)
{
    if (m_displayMode != GROUP_BY_CATEGORY)
        return;

    QWidget *categoryView = this->categoryBoxWidget(category);

    if (categoryView)
        categoryView->setVisible(appNums);
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
    AppsListModel::AppCategory category = (m_displayMode == SEARCH) ? AppsListModel::Search : AppsListModel::All;
    m_multiPagesView->setModel(category);

    m_internetBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Internet));
    m_chatBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Chat));
    m_musicBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Music));
    m_videoBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Video));
    m_graphicsBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Graphics));
    m_gameBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Game));
    m_officeBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Office));
    m_readingBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Reading));
    m_developmentBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Development));
    m_systemBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::System));
    m_othersBoxWidget->setVisible(isCategoryMode && m_appsManager->appNums(AppsListModel::Others));

    m_viewListPlaceholder->setVisible(isCategoryMode);
    m_navigationWidget->setVisible(isCategoryMode);

    // choose nothing
    m_appItemDelegate->setCurrentIndex(QModelIndex());

    hideTips();

    if (m_displayMode == GROUP_BY_CATEGORY)
        QTimer::singleShot(200, this, [ = ] {
        scrollToCategory(m_currentCategory);
    });

    else
        m_appsArea->horizontalScrollBar()->setValue(0);

    emit displayModeChanged(m_displayMode);
}

void FullScreenFrame::updateCurrentVisibleCategory()
{
    if (m_displayMode != GROUP_BY_CATEGORY)
        return;

    AppsListModel::AppCategory currentVisibleCategory;
    if (!m_internetBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Internet;
    else if (!m_chatBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Chat;
    else if (!m_musicBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Music;
    else if (!m_videoBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Video;
    else if (!m_graphicsBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Graphics;
    else if (!m_gameBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Game;
    else if (!m_officeBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Office;
    else if (!m_readingBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Reading;
    else if (!m_developmentBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::Development;
    else if (!m_systemBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
        currentVisibleCategory = AppsListModel::System;
    else if (!m_othersBoxWidget->getMultiPagesView()->visibleRegion().isEmpty())
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

    const QRect dockGeometry = m_appsManager->dockGeometry();

    int bottomMargin = (m_displayMode == GROUP_BY_CATEGORY) ? m_calcUtil->getScreenSize().height() *0.064815 : 15;

    m_topSpacing->setFixedHeight(30);
    m_bottomSpacing->setFixedHeight(bottomMargin);

    switch (m_appsManager->dockPosition()) {
    case DOCK_POS_TOP:
        m_topSpacing->setFixedHeight(30 + dockGeometry.height());
        bottomMargin = m_topSpacing->height() + 20;
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(0);
        break;
    case DOCK_POS_BOTTOM:
        bottomMargin += dockGeometry.height();
        m_bottomSpacing->setFixedHeight(bottomMargin);
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

    int padding = m_calcUtil->getScreenSize().width() / 10;
    if (m_pHBoxLayout && m_padding != padding) {
        m_padding = padding;
        m_pHBoxLayout->setContentsMargins(m_padding, 0, m_padding, 0);
    }

    m_calcUtil->calculateAppLayout(m_appsArea->size() - QSize(m_padding * 2, bottomMargin),
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

AppsListModel::AppCategory FullScreenFrame::nextCategoryModel(const AppsListModel::AppCategory category)
{
    int nextCategory = category + 1;
    if (nextCategory > AppsListModel::Others)
        nextCategory = AppsListModel::Internet;

    if (!m_appsManager->appNums(AppsListModel::AppCategory(nextCategory))) {
        nextCategory = nextCategoryModel(AppsListModel::AppCategory(nextCategory));
    }

    return static_cast<AppsListModel::AppCategory>(nextCategory);
}

AppsListModel::AppCategory FullScreenFrame::prevCategoryModel(const AppsListModel::AppCategory category)
{
    int nextCategory = category - 1;
    if (nextCategory < AppsListModel::Internet)
        nextCategory = AppsListModel::Others;

    if (!m_appsManager->appNums(AppsListModel::AppCategory(nextCategory))) {
        nextCategory = prevCategoryModel(AppsListModel::AppCategory(nextCategory));
    }

    return (AppsListModel::AppCategory)nextCategory;
}

void FullScreenFrame::layoutChanged()
{
    QSize boxSize;

    QPixmap pixmap = cachePixmap();
    if (m_displayMode == ALL_APPS || m_displayMode == SEARCH) {
        const int appsContentWidth = (m_appsArea->width() - m_padding * 2);
        boxSize.setWidth(appsContentWidth);
        boxSize.setHeight(m_appsArea->height() - m_topSpacing->height());
        m_multiPagesView->setFixedSize(boxSize);
        m_multiPagesView->updatePosition();
    } else {
        boxSize = m_calcUtil->getAppBoxSize();
    }

//    qreal scale = qApp->primaryScreen()->devicePixelRatio();
//    m_searchWidget->setFixedHeight(m_calcUtil->getScreenSize().height()*0.043*scale);
//    m_navigationWidget->setFixedHeight(m_calcUtil->getScreenSize().height()*0.083*scale);
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
            getCategoryGridViewList(AppsListModel::AppCategory(i + 4))->pageView(j)->setFixedHeight(boxSize.height());
        }
        getCategoryGridViewList(AppsListModel::AppCategory(i + 4))->updatePosition();
    }

    if (m_displayMode == ALL_APPS || m_displayMode == SEARCH) {
        m_appsArea->horizontalScrollBar()->setValue(0);
    } else {
        scrollToCategory(m_currentCategory);
    }
}

void FullScreenFrame::searchTextChanged(const QString &keywords)
{
    QString tmpKeywords = keywords;
    //删除搜索字符串中的空格
    tmpKeywords.remove(QChar(' '));
    if (tmpKeywords.isEmpty())
        updateDisplayMode(m_calcUtil->displayMode());
    else
        updateDisplayMode(SEARCH);

    if (m_searchWidget->edit()->lineEdit()->text().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->clearFocus();
    }

    m_appsManager->searchApp(keywords.trimmed());
}

void FullScreenFrame::updateFrameCursor()
{
    static QCursor *lastArrowCursor = nullptr;
    static QString  lastCursorTheme;
    int lastCursorSize = 0;
    QGSettings gsetting("com.deepin.xsettings", "/com/deepin/xsettings/");
    QString theme = gsetting.get("gtk-cursor-theme-name").toString();
    int cursorSize = gsetting.get("gtk-cursor-theme-size").toInt();
    if (theme != lastCursorTheme || cursorSize != lastCursorSize)
    {
        QCursor *cursor = loadQCursorFromX11Cursor(theme.toStdString().c_str(), "left_ptr", cursorSize);
        lastCursorTheme = theme;
        lastCursorSize = cursorSize;
        setCursor(*cursor);
        if (lastArrowCursor != nullptr)
            delete lastArrowCursor;

        lastArrowCursor = cursor;
    }
}

