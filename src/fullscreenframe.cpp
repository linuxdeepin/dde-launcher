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
#include <QProcess>
#include <DWindowManagerHelper>

#include <ddialog.h>
#include <QScroller>

#define     SIDES_SPACE_SCALE   0.10

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
    m_clearCacheTimer(new QTimer(this)),
    m_navigationWidget(new NavigationWidget(this)),
    m_searchWidget(new SearchWidget(this)),
    m_contentFrame(new QFrame(this)),
    m_appsIconBox(new DHBoxWidget(m_contentFrame)),
    m_appsItemBox(new DHBoxWidget(m_contentFrame)),
    m_appsItemSeizeBox(new MaskQWidget),
    m_tipsLabel(new QLabel(this)),
    m_appItemDelegate(new AppItemDelegate),
    m_multiPagesView(new MultiPagesView()),

    m_internetBoxWidget(new BlurBoxWidget(AppsListModel::Internet, const_cast<char *>("Internet"),m_appsItemBox)),
    m_chatBoxWidget(new BlurBoxWidget(AppsListModel::Chat, const_cast<char *>("Chat"),m_appsItemBox)),
    m_musicBoxWidget(new BlurBoxWidget(AppsListModel::Music, const_cast<char *>("Music"),m_appsItemBox)),
    m_videoBoxWidget(new BlurBoxWidget(AppsListModel::Video, const_cast<char *>("Video"),m_appsItemBox)),
    m_graphicsBoxWidget(new BlurBoxWidget(AppsListModel::Graphics, const_cast<char *>("Graphics"),m_appsItemBox)),
    m_gameBoxWidget(new BlurBoxWidget(AppsListModel::Game, const_cast<char *>("Games"),m_appsItemBox)),
    m_officeBoxWidget(new BlurBoxWidget(AppsListModel::Office, const_cast<char *>("Office"),m_appsItemBox)),
    m_readingBoxWidget(new BlurBoxWidget(AppsListModel::Reading, const_cast<char *>("Reading"),m_appsItemBox)),
    m_developmentBoxWidget(new BlurBoxWidget(AppsListModel::Development, const_cast<char *>("Development"),m_appsItemBox)),
    m_systemBoxWidget(new BlurBoxWidget(AppsListModel::System, const_cast<char *>("System"),m_appsItemBox)),
    m_othersBoxWidget(new BlurBoxWidget(AppsListModel::Others, const_cast<char *>("Other"),m_appsItemBox)),
    m_topSpacing(new QFrame),
    m_bottomSpacing(new QFrame),
    m_animationGroup(new ScrollParallelAnimationGroup(this)),
    m_displayInter(new DBusDisplay(this))
{
    // accessible.h 中使用
    setAccessibleName("FullScrreenFrame");
    m_topSpacing->setAccessibleName("topspacing");
    m_bottomSpacing->setAccessibleName("BottomSpacing");
    m_appsItemBox->setAccessibleName("apphbox");
    m_navigationWidget->setAccessibleName("navigationWidget");
    m_searchWidget->setAccessibleName("searchWidget");
    m_tipsLabel->setAccessibleName("tipsLabel");
    m_appItemDelegate->setObjectName("appItemDelegate");
    m_internetBoxWidget->setAccessibleName("internetBoxWidget");
    m_chatBoxWidget->setAccessibleName("chatBoxWidget");
    m_musicBoxWidget->setAccessibleName("musicBoxWidget");
    m_videoBoxWidget->setAccessibleName("videoBoxWidget");
    m_graphicsBoxWidget->setAccessibleName("graphicsBoxWidget");
    m_gameBoxWidget->setAccessibleName("gameBoxWidget");
    m_officeBoxWidget->setAccessibleName("officeBoxWidget");
    m_readingBoxWidget->setAccessibleName("readingBoxWidget");
    m_developmentBoxWidget->setAccessibleName("developmentBoxWidget");
    m_systemBoxWidget->setAccessibleName("systemBoxWidget");
    m_othersBoxWidget->setAccessibleName("othersBoxWidget");
    m_searchWidget->categoryBtn()->setAccessibleName("search_categoryBtn");
    m_contentFrame->setAccessibleName("ContentFrame");
    m_searchWidget->edit()->setAccessibleName("FullScreenSearchEdit");

    m_focusIndex = 0;
    m_currentCategory = AppsListModel::Internet;
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
    m_mouse_press = false;
    m_mouse_press_time = 0;
    setAttribute(Qt::WA_InputMethodEnabled, true);

    QPalette palette;
    QColor colorButton(255, 255, 255, 0.15 * 255);
    QColor colorText(255, 255, 255);
    palette.setColor(QPalette::Button, colorButton);
    palette.setColor(QPalette::Text,colorText);
    palette.setColor(QPalette::ButtonText,colorText);

    m_searchWidget->edit()->lineEdit()->setPalette(palette);
    m_searchWidget->categoryBtn()->setPalette(palette);
    m_searchWidget->toggleModeBtn()->setPalette(palette);

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

    setObjectName("LauncherFrame");

    installEventFilter(m_eventFilter);

    connect(m_multiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    for (int i = AppsListModel::Internet; i <= AppsListModel::Others; i++) {
        AppsListModel::AppCategory appCategory = AppsListModel::AppCategory(i);
        MultiPagesView * multiPagesView = getCategoryGridViewList(appCategory);
        connect(multiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    }

    for (int i = 0; i < 5; i++) {
        ScrollWidgetAgent *widgetAgent = new ScrollWidgetAgent(this);
        widgetAgent->setControlWidget(m_appsItemBox);
        connect(widgetAgent, &ScrollWidgetAgent::scrollBlurBoxWidget, this, &FullScreenFrame::scrollBlurBoxWidget);
        m_widgetAgentList.append(widgetAgent);
        m_animationGroup->addAnimation(widgetAgent->animation());
    }

    initUI();
    initConnection();

    updateDisplayMode(m_calcUtil->displayMode());
}

FullScreenFrame::~FullScreenFrame()
{
    while (m_widgetAgentList.count() > 0) {
        ScrollWidgetAgent *widgetAgent = m_widgetAgentList.takeAt(0);
        delete widgetAgent;
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

void FullScreenFrame::scrollToCategory(const AppsListModel::AppCategory oldCategory, const AppsListModel::AppCategory newCategory)
{
    m_searchWidget->clearSearchContent();
    if (m_animationGroup->state() == m_animationGroup->Running) return;

    int spaceCount = nearestCategory(oldCategory, newCategory);

    if (spaceCount == 0) {
        scrollCurrent();
    }  else if (spaceCount == -1) {
        scrollPrev();
    } else if (spaceCount == 1) {
        scrollNext();
    } else if (spaceCount < -1) {
        hideCategoryBoxWidget();
        m_currentCategory = nextCategoryType(newCategory);
        showCategoryBoxWidget(m_currentCategory);
        scrollPrev();
    } else if (spaceCount > 1) {
        hideCategoryBoxWidget();
        m_currentCategory = prevCategoryType(newCategory);
        showCategoryBoxWidget(m_currentCategory);
        scrollNext();
    }
}

void FullScreenFrame::blurBoxWidgetMaskClick(const AppsListModel::AppCategory appCategory)
{
    if (m_mouse_press) {
        m_mouse_press = false;
    }

    if (m_animationGroup->state() == m_animationGroup->Running) {
        return;
    }

    if (appCategory == m_currentCategory) {
        hide();
        return;
    }

    scrollToCategory(m_currentCategory, appCategory);
}

void FullScreenFrame::scrollPrev()
{
    int categoryCount = m_appsManager->getVisibleCategoryCount();

    if (categoryCount <= 1) {
        return;
    }

    if (categoryCount == 2) {
        for (int i = Pos_LL; i <= Pos_RR; i++ ) {
            PosType posType = PosType(i);
            ScrollWidgetAgent *widgetAgent = getScrollWidgetAgent(posType);

            if (posType == Pos_RR) {
                widgetAgent->setScrollToType(Pos_L);
            } else if (posType == Pos_R) {
                widgetAgent->setScrollToType(Pos_LL);
            } else if (posType == Pos_M) {
                widgetAgent->setScrollToType(Pos_RR);
            } else if (posType == Pos_L) {
                widgetAgent->setScrollToType(Pos_R);
            } else if (posType == Pos_LL) {
                widgetAgent->setScrollToType(Pos_M);
            }
        }
    } else {
        for (int i = Pos_LL; i <= Pos_RR; i++ ) {
            PosType posType = PosType(i);
            ScrollWidgetAgent *widgetAgent = getScrollWidgetAgent(posType);

            if (posType == Pos_LL) {
                widgetAgent->setScrollToType(Pos_L);
            } else if (posType == Pos_L) {
                widgetAgent->setScrollToType(Pos_M);
            } else if (posType == Pos_M) {
                widgetAgent->setScrollToType(Pos_R);
            } else if (posType == Pos_R) {
                widgetAgent->setScrollToType(Pos_RR);
            } else if (posType == Pos_RR) {
                widgetAgent->setScrollToType(Pos_LL);
            }
        }
    }

    m_animationGroup->setScrollType(Scroll_Prev);
    m_animationGroup->start();
}

void FullScreenFrame::scrollNext()
{
    int categoryCount = m_appsManager->getVisibleCategoryCount();

    if (categoryCount <= 1) {
        return;
    }

    if (categoryCount == 2) {
        for (int i = Pos_LL; i <= Pos_RR; i++ ) {
            PosType posType = PosType(i);
            ScrollWidgetAgent *widgetAgent = getScrollWidgetAgent(posType);

            if (posType == Pos_LL) {
                widgetAgent->setScrollToType(Pos_R);
            } else if (posType == Pos_L) {
                widgetAgent->setScrollToType(Pos_RR);
            } else if (posType == Pos_M) {
                widgetAgent->setScrollToType(Pos_LL);
            } else if (posType == Pos_R) {
                widgetAgent->setScrollToType(Pos_L);
            } else if (posType == Pos_RR) {
                widgetAgent->setScrollToType(Pos_M);
            }
        }
    } else {
        for (int i = Pos_LL; i <= Pos_RR; i++ ) {
            PosType posType = PosType(i);
            ScrollWidgetAgent *widgetAgent = getScrollWidgetAgent(posType);

            if (posType == Pos_LL) {
                widgetAgent->setScrollToType(Pos_RR);
            } else if (posType == Pos_L) {
                widgetAgent->setScrollToType(Pos_LL);
            } else if (posType == Pos_M) {
                widgetAgent->setScrollToType(Pos_L);
            } else if (posType == Pos_R) {
                widgetAgent->setScrollToType(Pos_M);
            } else if (posType == Pos_RR) {
                widgetAgent->setScrollToType(Pos_R);
            }
        }
    }

    m_animationGroup->setScrollType(Scroll_Next);
    m_animationGroup->start();
}

void FullScreenFrame::scrollCurrent()
{
    for (int i = Pos_LL; i <= Pos_RR; i++ ) {
        PosType posType = PosType(i);
        ScrollWidgetAgent *widgetAgent = getScrollWidgetAgent(posType);
        widgetAgent->setScrollToType(posType);
    }

    m_animationGroup->setScrollType(Scroll_Current);
    m_animationGroup->start();
}

void FullScreenFrame::addViewEvent(AppGridView *pView)
{
    connect(pView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(pView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(pView, &AppGridView::clicked, m_appsManager, &AppsManager::launchApp);
    connect(pView, &AppGridView::clicked, this, &FullScreenFrame::hide);
    connect(pView, &AppGridView::requestMouseRelease,this,  [ = ]() {
            m_mouse_press = false;
    });
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, pView, static_cast<void (AppGridView::*)(const QModelIndex &)>(&AppGridView::update));
}

void FullScreenFrame::scrollBlurBoxWidget(ScrollWidgetAgent * widgetAgent)
{
    if (!widgetAgent) {
        return;
    }

    if (m_animationGroup->currentScrollType() == Scroll_Current) {
        return;
    }

    if (widgetAgent->posType() != Pos_M) {
       return;
    }

    BlurBoxWidget * blurBoxWidget = widgetAgent->blurBoxWidget();
    if (!blurBoxWidget) {
        return;
    }

    int categoryCount = m_appsManager->getVisibleCategoryCount();

    if (categoryCount <= 1) {
        return;
    }

    ScrollWidgetAgent *pos_LL_WidgetAgent = getScrollWidgetAgent(Pos_LL);
    ScrollWidgetAgent *pos_L_WidgetAgent = getScrollWidgetAgent(Pos_L);
    ScrollWidgetAgent *pos_M_WidgetAgent = getScrollWidgetAgent(Pos_M);
    ScrollWidgetAgent *pos_R_WidgetAgent = getScrollWidgetAgent(Pos_R);
    ScrollWidgetAgent *pos_RR_WidgetAgent = getScrollWidgetAgent(Pos_RR);

    if (categoryCount == 2) {
        if (m_animationGroup->currentScrollType() == Scroll_Next &&
            pos_R_WidgetAgent->pos().x() <= m_contentFrame->width() / 2 &&
            !pos_RR_WidgetAgent->blurBoxWidget()) {

            pos_LL_WidgetAgent->setVisible(false);
            pos_LL_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_LL_WidgetAgent->setPos(pos_RR_WidgetAgent->pos() + QPoint(m_calcUtil->getAppBoxSize().width() + DLauncher::APPHBOX_SPACING, 0));

            pos_L_WidgetAgent->setVisible(false);
            pos_L_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_L_WidgetAgent->setPos(pos_LL_WidgetAgent->pos() + QPoint(m_calcUtil->getAppBoxSize().width() + DLauncher::APPHBOX_SPACING, 0));

            BlurBoxWidget * blurBoxWidget = getCategoryBoxWidgetByPostType(Pos_RR,m_currentCategory);
            pos_RR_WidgetAgent->setBlurBoxWidget(blurBoxWidget);
            pos_RR_WidgetAgent->setVisible(true);
        } else if (m_animationGroup->currentScrollType() == Scroll_Prev &&
                   pos_L_WidgetAgent->pos().x() + m_calcUtil->getAppBoxSize().width() >= m_contentFrame->width() / 2 &&
                   !pos_LL_WidgetAgent->blurBoxWidget()) {

            pos_RR_WidgetAgent->setVisible(false);
            pos_RR_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_RR_WidgetAgent->setPos(pos_LL_WidgetAgent->pos() - QPoint(m_calcUtil->getAppBoxSize().width() + DLauncher::APPHBOX_SPACING, 0));

            pos_R_WidgetAgent->setVisible(false);
            pos_R_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_R_WidgetAgent->setPos(pos_RR_WidgetAgent->pos() - QPoint(m_calcUtil->getAppBoxSize().width() + DLauncher::APPHBOX_SPACING, 0));

            BlurBoxWidget * blurBoxWidget = getCategoryBoxWidgetByPostType(Pos_LL,m_currentCategory);
            pos_LL_WidgetAgent->setBlurBoxWidget(blurBoxWidget);
            pos_LL_WidgetAgent->setVisible(true);
        }  else if (m_animationGroup->currentScrollType() == Scroll_Next &&
            pos_RR_WidgetAgent->pos().x() <= m_contentFrame->width() / 2 &&
            pos_M_WidgetAgent->blurBoxWidget()) {
            pos_LL_WidgetAgent->setPosType(Pos_R);
            pos_L_WidgetAgent->setPosType(Pos_RR);
            pos_M_WidgetAgent->setPosType(Pos_LL);
            pos_M_WidgetAgent->setVisible(false);
            pos_M_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_R_WidgetAgent->setPosType(Pos_L);
            pos_RR_WidgetAgent->setPosType(Pos_M);
            scrollToCategoryFinish();
        } else if (m_animationGroup->currentScrollType() == Scroll_Prev &&
                   pos_LL_WidgetAgent->pos().x() + m_calcUtil->getAppBoxSize().width() >= m_contentFrame->width() / 2 &&
                   pos_M_WidgetAgent->blurBoxWidget()) {
            pos_RR_WidgetAgent->setPosType(Pos_L);
            pos_R_WidgetAgent->setPosType(Pos_LL);
            pos_M_WidgetAgent->setPosType(Pos_RR);
            pos_M_WidgetAgent->setVisible(false);
            pos_M_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_L_WidgetAgent->setPosType(Pos_R);
            pos_LL_WidgetAgent->setPosType(Pos_M);
            scrollToCategoryFinish();
        }
    } else {
        if (m_animationGroup->currentScrollType() == Scroll_Next &&
            pos_R_WidgetAgent->pos().x() <= m_contentFrame->width() / 2 &&
            !pos_RR_WidgetAgent->blurBoxWidget()) {

            pos_LL_WidgetAgent->setPosType(Pos_RR);
            pos_LL_WidgetAgent->setVisible(false);
            pos_LL_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_LL_WidgetAgent->setPos(pos_RR_WidgetAgent->pos() + QPoint(m_calcUtil->getAppBoxSize().width() + DLauncher::APPHBOX_SPACING, 0));

            pos_L_WidgetAgent->setPosType(Pos_LL);
            pos_L_WidgetAgent->setVisible(false);
            pos_L_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_M_WidgetAgent->setPosType(Pos_L);
            pos_R_WidgetAgent->setPosType(Pos_M);
            pos_RR_WidgetAgent->setPosType(Pos_R);

            BlurBoxWidget * blurBoxWidget = getCategoryBoxWidgetByPostType(Pos_RR,m_currentCategory);
            pos_RR_WidgetAgent->setBlurBoxWidget(blurBoxWidget);
            pos_RR_WidgetAgent->setVisible(true);

            m_mouse_press_pos = QCursor::pos();
            scrollToCategoryFinish();
        } else if (m_animationGroup->currentScrollType() == Scroll_Prev &&
                   pos_L_WidgetAgent->pos().x() + m_calcUtil->getAppBoxSize().width() >= m_contentFrame->width() / 2 &&
                   !pos_LL_WidgetAgent->blurBoxWidget()) {
            pos_RR_WidgetAgent->setPosType(Pos_LL);
            pos_RR_WidgetAgent->setVisible(false);
            pos_RR_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_RR_WidgetAgent->setPos(pos_LL_WidgetAgent->pos() - QPoint(m_calcUtil->getAppBoxSize().width() + DLauncher::APPHBOX_SPACING, 0));

            pos_R_WidgetAgent->setPosType(Pos_RR);
            pos_R_WidgetAgent->setVisible(false);
            pos_R_WidgetAgent->setBlurBoxWidget(nullptr);
            pos_M_WidgetAgent->setPosType(Pos_R);
            pos_L_WidgetAgent->setPosType(Pos_M);
            pos_LL_WidgetAgent->setPosType(Pos_L);

            BlurBoxWidget * blurBoxWidget = getCategoryBoxWidgetByPostType(Pos_LL,m_currentCategory);
            pos_LL_WidgetAgent->setBlurBoxWidget(blurBoxWidget);
            pos_LL_WidgetAgent->setVisible(true);

            m_mouse_press_pos = QCursor::pos();
            scrollToCategoryFinish();
        }
    }
}

void FullScreenFrame::showTips(const QString &tips)
{
    if (m_displayMode != SEARCH)
        return;

    m_tipsLabel->setText(tips);
    QPalette palette;
    QColor color(255, 255, 255, 0.5 * 255);
    QFont font;
    font.setPointSize(30);
    palette.setColor(QPalette::WindowText,color);
    m_tipsLabel->setPalette(palette);
    m_tipsLabel->setFont(font);

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
    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();
    XcbMisc::instance()->set_deepin_override(winId());
    // To make sure the window is placed at the right position.
    updateGeometry();
    updateBackground();

    if (!m_appsManager->isVaild()) {
        m_appsManager->refreshAllList();
    }

    updateDockPosition();

    QFrame::showEvent(e);

    QTimer::singleShot(0, this, [this]() {
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

//    m_clearCacheTimer->start();
}

void FullScreenFrame::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
       return;
    m_searchWidget->clearSearchContent();
    m_mouse_press = true;
    m_mouse_press_time =  QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_mouse_move_pos = e->pos();
    m_mouse_press_pos = e->pos();
}

void FullScreenFrame::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_mouse_press  || e->button() == Qt::RightButton) {
        return;
    }

    if (m_animationGroup->state() == m_animationGroup->Running) {
        return;
    }

    int categoryCount = m_appsManager->getVisibleCategoryCount();

    if (categoryCount <= 2) {
        return;
    }

    qint64 mouse_release_time =  QDateTime::currentDateTime().toMSecsSinceEpoch();

    int move_diff = e->pos().x() - m_mouse_move_pos.x();
    int moved_diff = e->pos().x() - m_mouse_press_pos.x();

    foreach (ScrollWidgetAgent * agent, m_widgetAgentList) {
        QPoint p(agent->pos());
        p.setX(p.x() + move_diff);
        agent->setPos(p);
    }

    if (mouse_release_time - m_mouse_press_time > DLauncher::MOUSE_PRESS_TIME_DIFF) {
        if (moved_diff < 0) {
            m_animationGroup->setScrollType(Scroll_Next);
        } else {
            m_animationGroup->setScrollType(Scroll_Prev);
        }
    }

    m_mouse_move_pos = e->pos();
}

void FullScreenFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !m_mouse_press)
        return;

    int diff_x = qAbs(e->pos().x() - m_mouse_press_pos.x());
    int diff_y = qAbs(e->pos().y() - m_mouse_press_pos.y());
    // 小范围位置变化，当作没有变化，针对触摸屏
    if ((e->source() == Qt::MouseEventSynthesizedByQt && diff_x < DLauncher::TOUCH_DIFF_THRESH && diff_y < DLauncher::TOUCH_DIFF_THRESH)
            || (e->source() != Qt::MouseEventSynthesizedByQt && e->pos() == m_mouse_press_pos )) {
        hide();
    } else if (m_displayMode == GROUP_BY_CATEGORY){
        qint64 mouse_release_time =  QDateTime::currentDateTime().toMSecsSinceEpoch();
        int move_diff   = e->pos().x() - m_mouse_press_pos.x();
        //快速滑动
        if (mouse_release_time - m_mouse_press_time <= DLauncher::MOUSE_PRESS_TIME_DIFF &&
            abs(move_diff) > DLauncher::MOUSE_MOVE_TO_NEXT) {

            if (move_diff > 0) {
                AppsListModel::AppCategory targetCategory = prevCategoryType(m_currentCategory);
                scrollToCategory(m_currentCategory, targetCategory);
            } else {
                AppsListModel::AppCategory targetCategory = nextCategoryType(m_currentCategory);
                scrollToCategory(m_currentCategory, targetCategory);
            }
        } else {
            scrollToCategory(m_currentCategory, m_currentCategory);
        }
    }
    m_mouse_press = false;
}

void FullScreenFrame::wheelEvent(QWheelEvent *e)
{
    if (m_displayMode == GROUP_BY_CATEGORY) {
        if (m_animationGroup->state() == m_animationGroup->Running) return;
        static int  wheelTime = 0;
        if (e->angleDelta().y() < 0 ||  e-> angleDelta().x() < 0) {
            wheelTime++;
        } else {
            wheelTime--;
        }

        if (wheelTime >= DLauncher::WHOOLTIME_TO_SCROOL || wheelTime <= -DLauncher::WHOOLTIME_TO_SCROOL) {
            if (wheelTime > 0) {
                scrollNext();
            } else {
                scrollPrev();
            }

            wheelTime = 0;
        }
        return;
    }

}

bool FullScreenFrame::eventFilter(QObject *o, QEvent *e)
{   
    QKeyEvent *keyPress = static_cast<QKeyEvent *>(e);
    if (m_displayMode == GROUP_BY_CATEGORY && e->type() == QEvent::KeyPress && keyPress->key() == Qt::Key_Tab) {
        foreach (QAbstractButton * button, m_navigationWidget->buttonGroup()->buttons()) {
            if (button == o ) {
                m_focusIndex = CategoryTital;
                nextTabWidget(Qt::Key_Tab);
                return true;
            }
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

    m_clearCacheTimer->setSingleShot(true);
    m_clearCacheTimer->setInterval(DLauncher::CLEAR_CACHE_TIMER * 1000);

    m_searchWidget->categoryBtn()->installEventFilter(m_eventFilter);
    m_searchWidget->edit()->lineEdit()->installEventFilter(this);
    m_searchWidget->installEventFilter(m_eventFilter);
    m_appItemDelegate->installEventFilter(m_eventFilter);

    m_multiPagesView->setAccessibleName("allAppPagesView");
    m_multiPagesView->setDataDelegate(m_appItemDelegate);
    m_multiPagesView->updatePageCount(AppsListModel::All);
    m_multiPagesView->installEventFilter(this);

    foreach (QAbstractButton * button, m_navigationWidget->buttonGroup()->buttons()) {
        if (button) {
            button->installEventFilter(m_eventFilter);
        }
    }



    m_internetBoxWidget->setVisible(false);
    m_chatBoxWidget->setVisible(false);
    m_musicBoxWidget->setVisible(false);
    m_videoBoxWidget->setVisible(false);
    m_graphicsBoxWidget->setVisible(false);
    m_gameBoxWidget->setVisible(false);
    m_officeBoxWidget->setVisible(false);
    m_readingBoxWidget->setVisible(false);
    m_developmentBoxWidget->setVisible(false);
    m_systemBoxWidget->setVisible(false);
    m_othersBoxWidget->setVisible(false);

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

    //m_appsItemBox->installEventFilter(this);
    m_appsItemBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_appsItemBox->layout()->setMargin(0);
    m_appsItemBox->layout()->setSpacing(0);
    m_appsItemBox->layout()->addWidget(m_appsItemSeizeBox);
    m_appsItemSeizeBox->lower();

    //自由排序模式，设置大小调整方式为固定方式
    m_appsIconBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_appsIconBox->layout()->setSpacing(0);
    m_appsIconBox->layout()->addWidget(m_multiPagesView, 0, Qt::AlignCenter);
    m_pHBoxLayout = m_appsIconBox->layout();

    //启动时默认按屏幕大小设置自由排序widget的大小
    int padding = m_calcUtil->getScreenSize().width() * SIDES_SPACE_SCALE;
    QSize boxSize;
    const int appsContentWidth = (m_calcUtil->getScreenSize().width() - padding * 2);
    const int appsContentHeight = (m_calcUtil->getScreenSize().height() - DLauncher::APPS_AREA_TOP_MARGIN);
    boxSize.setWidth(appsContentWidth);
    boxSize.setHeight(appsContentHeight);
    m_appsIconBox->setFixedSize(boxSize);

    QVBoxLayout *scrollVLayout = new QVBoxLayout;
    scrollVLayout->setContentsMargins(0, DLauncher::APPS_AREA_TOP_MARGIN, 0, 0);
    scrollVLayout->setSpacing(0);
    scrollVLayout->addWidget(m_appsIconBox,0,Qt::AlignCenter);
    scrollVLayout->addWidget(m_appsItemBox,0,Qt::AlignCenter);

    m_contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentFrame->setFrameStyle(QFrame::NoFrame);
    m_contentFrame->setLayout(scrollVLayout);
    m_contentFrame->installEventFilter(this);

    m_navigationWidget->setFixedHeight(m_calcUtil->instance()->navigationHeight());

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_topSpacing);
    m_mainLayout->addWidget(m_searchWidget);
    m_mainLayout->addWidget(m_navigationWidget, 0, Qt::AlignHCenter);
    m_mainLayout->addWidget(m_contentFrame);
    m_mainLayout->addWidget(m_bottomSpacing);

    setLayout(m_mainLayout);
}

MultiPagesView *FullScreenFrame::getCategoryGridViewList(const AppsListModel::AppCategory category)
{
    BlurBoxWidget * widget = getCategoryBoxWidget(category);

    return widget->getMultiPagesView();
}

BlurBoxWidget *FullScreenFrame::getCategoryBoxWidget(const AppsListModel::AppCategory category) const
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

void FullScreenFrame::checkCurrentCategoryVisible()
{
    AppsListModel::AppCategory tmpCategory = m_currentCategory;
    //判断当前分类是否允许显示，如果应用当前分类应用个数小于等于0，则当前分类不允许显示，显示其他分类
    if (m_appsManager->appNums(tmpCategory) <= 0) {
        //使用前一分类作为当前分类，若前一分类循环变成others等后面的分类，则使用当前分类的后一分类作为当前分类
        tmpCategory =  prevCategoryType(m_currentCategory);

        if (tmpCategory > m_currentCategory) {
            tmpCategory = nextCategoryType(m_currentCategory);
        }
    }

    m_currentCategory = tmpCategory;
    emit currentVisibleCategoryChanged(m_currentCategory);
}

void FullScreenFrame::showCategoryBoxWidget(AppsListModel::AppCategory appCategory)
{
    for (int i = Pos_LL; i <= Pos_RR; i++) {
        PosType posType =  PosType(i);

        ScrollWidgetAgent *widgetAgent = getScrollWidgetAgent(posType);
        widgetAgent->setPosType(posType);

        QPoint p = widgetAgent->getScrollWidgetAgentPos(posType);
        widgetAgent->setPos(p);

        if (posType >= Pos_L && posType <= Pos_R) {
            BlurBoxWidget * blurBoxWidget = getCategoryBoxWidgetByPostType(posType, appCategory);
            widgetAgent->setBlurBoxWidget(blurBoxWidget);
            widgetAgent->setVisible(true);
        }
    }
}

void FullScreenFrame::hideCategoryBoxWidget()
{
    for (int i = 0; i < m_widgetAgentList.count(); i++) {
        ScrollWidgetAgent *widgetAgent = m_widgetAgentList.at(i);
        widgetAgent->setVisible(false);
        widgetAgent->setBlurBoxWidget(nullptr);
    }
}

void FullScreenFrame::scrollToCategoryFinish()
{
    int categoryCount = m_appsManager->getVisibleCategoryCount();

    if (categoryCount <= 1) {
        return;
    } else if (categoryCount == 2) {
        if (m_animationGroup->currentScrollType() == Scroll_Prev) {
            m_currentIndex = (m_currentIndex - 2);
            m_currentIndex = m_currentIndex >= 0 ? m_currentIndex : m_widgetAgentList.count() + m_currentIndex;
        } else if (m_animationGroup->currentScrollType() == Scroll_Next) {
            m_currentIndex = (m_currentIndex + 2) % m_widgetAgentList.count();
        }
    } else {
        if (m_animationGroup->currentScrollType() == Scroll_Prev) {
            m_currentIndex = (m_currentIndex - 1);
            m_currentIndex = m_currentIndex >= 0 ? m_currentIndex : m_widgetAgentList.count() + m_currentIndex;
        } else if (m_animationGroup->currentScrollType() == Scroll_Next) {
            m_currentIndex = (m_currentIndex + 1) % m_widgetAgentList.count();
        }
    }

    m_animationGroup->setScrollType(Scroll_Current);

    ScrollWidgetAgent * widgetAgent = getScrollWidgetAgent(Pos_M);
    m_currentCategory = widgetAgent->blurBoxWidget()->category();

    emit currentVisibleCategoryChanged(m_currentCategory);
}

ScrollWidgetAgent *FullScreenFrame::getScrollWidgetAgent(PosType type)
{
    ScrollWidgetAgent *widgetAgent = nullptr;
    int index;

    switch (type) {
    case Pos_LL: {
        index = (m_currentIndex - 2) % m_widgetAgentList.count();
        index = index >= 0 ? index : m_widgetAgentList.count() + index;
        widgetAgent = m_widgetAgentList.at(index);
        break;
    }
    case Pos_L: {
        index = (m_currentIndex - 1);
        index = index >= 0 ? index : m_widgetAgentList.count() + index;
        widgetAgent = m_widgetAgentList.at(index);
        break;
    }
    case Pos_M: {
        widgetAgent = m_widgetAgentList.at(m_currentIndex);
        break;
    }
    case Pos_R: {
        index = (m_currentIndex + 1) % m_widgetAgentList.count();
        widgetAgent = m_widgetAgentList.at(index);
        break;
    }
    case Pos_RR:
        index = (m_currentIndex + 2) % m_widgetAgentList.count();
        widgetAgent = m_widgetAgentList.at(index);
        break;
    case Pos_None: break;
    }

    return widgetAgent;
}

BlurBoxWidget *FullScreenFrame::getCategoryBoxWidgetByPostType(PosType posType, AppsListModel::AppCategory appCategory)
{
    BlurBoxWidget *blurBoxWidget = nullptr;
    int categoryCount = m_appsManager->getVisibleCategoryCount();

    switch (posType) {
    case Pos_LL: {
        if (categoryCount >= 3) {
            blurBoxWidget = getCategoryBoxWidget(prevCategoryType(prevCategoryType(appCategory)));
        } else if (categoryCount == 2) {
            blurBoxWidget = getCategoryBoxWidget(prevCategoryType(appCategory));
        }
        break;
    }
    case Pos_L: {
        if (categoryCount >= 3) {
            blurBoxWidget = getCategoryBoxWidget(prevCategoryType(appCategory));
        }
        break;
    }
    case Pos_M: {
        if (categoryCount >= 1) {
            blurBoxWidget = getCategoryBoxWidget(appCategory);
        }
        break;
    }
    case Pos_R: {
        if (categoryCount >= 3) {
            blurBoxWidget = getCategoryBoxWidget(nextCategoryType(appCategory));
        }
        break;
    }
    case Pos_RR:
        if (categoryCount >= 3) {
            blurBoxWidget = getCategoryBoxWidget(nextCategoryType(nextCategoryType(appCategory)));
        } else if (categoryCount == 2) {
            blurBoxWidget = getCategoryBoxWidget(nextCategoryType(appCategory));
        }
        break;
    case Pos_None: break;
    }

    return blurBoxWidget;
}

int FullScreenFrame::nearestCategory(const AppsListModel::AppCategory oldCategory, const AppsListModel::AppCategory newCategory)
{
    int prevCount = 0;
    AppsListModel::AppCategory tmpCategory = oldCategory;
    while (tmpCategory != newCategory) {
        prevCount--;
        tmpCategory = prevCategoryType(tmpCategory);
    }

    tmpCategory = oldCategory;
    int nextCount = 0;
    while (tmpCategory != newCategory) {
        nextCount++;
        tmpCategory = nextCategoryType(tmpCategory);
    }

    if (prevCount != 0 || nextCount != 0) {
        if (abs(prevCount) < abs(nextCount)) {
            return prevCount;
        } else if (abs(prevCount) > abs(nextCount)) {
            return nextCount;
        } else if (oldCategory < newCategory) {
            return nextCount;
        } else {
            return prevCount;
        }
    } else{
        return 0;
    }
}

void FullScreenFrame::initConnection()
{
//    connect(qApp, &QApplication::primaryScreenChanged, this, &FullScreenFrame::updateGeometry);
//    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &FullScreenFrame::updateGeometry);
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &FullScreenFrame::layoutChanged, Qt::QueuedConnection);

    connect(m_navigationWidget, &NavigationWidget::scrollToCategory, this, &FullScreenFrame::scrollToCategory);

    connect(this, &FullScreenFrame::currentVisibleCategoryChanged, m_navigationWidget, &NavigationWidget::setCurrentCategory);
    connect(this, &FullScreenFrame::categoryAppNumsChanged, m_navigationWidget, &NavigationWidget::refershCategoryVisible);
    connect(this, &FullScreenFrame::displayModeChanged, this, &FullScreenFrame::categoryListChanged);

    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &FullScreenFrame::searchTextChanged);
    connect(m_delayHideTimer, &QTimer::timeout, this, &FullScreenFrame::hide, Qt::QueuedConnection);
    connect(m_clearCacheTimer, &QTimer::timeout, m_appsManager, &AppsManager::clearCache);

    connect(m_internetBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_chatBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_musicBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_videoBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_graphicsBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_gameBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_officeBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_readingBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_developmentBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_systemBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);
    connect(m_othersBoxWidget, &BlurBoxWidget::maskClick, this, &FullScreenFrame::blurBoxWidgetMaskClick);

    connect(this, &BoxFrame::backgroundImageChanged, m_internetBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_chatBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_musicBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_videoBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_graphicsBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_gameBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_officeBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_readingBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_developmentBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_systemBoxWidget, &BlurBoxWidget::updateBackgroundImage);
    connect(this, &BoxFrame::backgroundImageChanged, m_othersBoxWidget, &BlurBoxWidget::updateBackgroundImage);

    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &FullScreenFrame::hideLauncher);
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (FullScreenFrame::*)(const QModelIndex &)>(&FullScreenFrame::uninstallApp));
    connect(m_searchWidget, &SearchWidget::toggleMode, [this] {
        updateDisplayMode(m_calcUtil->displayMode() == GROUP_BY_CATEGORY ? ALL_APPS : GROUP_BY_CATEGORY);
    });

    connect(m_appsManager, &AppsManager::categoryListChanged, this, &FullScreenFrame::categoryListChanged);
    connect(m_appsManager, &AppsManager::requestTips, this, &FullScreenFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &FullScreenFrame::hideTips);
    connect(m_appsManager, &AppsManager::IconSizeChanged, this, &FullScreenFrame::updateDockPosition);
    connect(m_appsManager, &AppsManager::dataChanged, this, &FullScreenFrame::reflashPageView);

    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::ScreenHeightChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::ScreenWidthChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::PrimaryChanged, this, &FullScreenFrame::primaryScreenChanged, Qt::QueuedConnection);
}

void FullScreenFrame::showLauncher()
{
    m_focusIndex = 1;
    m_appItemDelegate->setCurrentIndex(QModelIndex());
    m_searchWidget->categoryBtn()->clearFocus();
    m_searchWidget->edit()->clearEdit();
    m_searchWidget->edit()->clear();
    updateDisplayMode(m_calcUtil->displayMode());
    m_searchWidget->edit()->lineEdit()->clearFocus();
    setFixedSize(m_appsManager->currentScreen()->geometry().size());
    show();
    connect(m_appsManager, &AppsManager::dockGeometryChanged, this, &FullScreenFrame::hideLauncher);
}

void FullScreenFrame::hideLauncher()
{
    if (!isVisible()) {
        return;
    }
    disconnect(m_appsManager, &AppsManager::dockGeometryChanged, this, &FullScreenFrame::hideLauncher);
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
        if (!oldStr.isEmpty() && oldStr == m_searchWidget->edit()->lineEdit()->text()) {
            m_searchWidget->edit()->lineEdit()->clear();
        }
        return;
    }

    if (Qt::Key_Space == key) {
        if (m_searchWidget->categoryBtn()->hasFocus()) {
            m_searchWidget->categoryBtn()->click();
        }
        return;
    }

    if (m_focusIndex == CategoryTital) {
        switch (key) {
        case Qt::Key_Backtab:
        case Qt::Key_Left: {
            scrollPrev();
            return;
        }
        case Qt::Key_Right: {
            scrollNext();
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
    case Qt::Key_Left:      index = currentIndex.sibling(currentIndex.row() - 1, 0); break;
    case Qt::Key_Right:    index = currentIndex.sibling(currentIndex.row() + 1, 0); break;
    case Qt::Key_Up:        index = currentIndex.sibling(currentIndex.row() - column, 0); break;
    case Qt::Key_Down:   index = currentIndex.sibling(currentIndex.row() + column, 0); break;
    default:;
    }

    //to next page
    if (m_displayMode != GROUP_BY_CATEGORY && !index.isValid()) {
        index = m_multiPagesView->selectApp(key);
        index = index.isValid() ? index : currentIndex;
    }

    // now, we need to check and fix if destination is invalid.
    do {
        if (index.isValid())
            break;

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
     }else {
        int currentPage =  getCategoryBoxWidget(m_currentCategory)->getMultiPagesView()->currentPage();
        uninstallApp(getCategoryBoxWidget(m_currentCategory)->getMultiPagesView()->pageModel(currentPage)->indexAt(appKey));
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

void FullScreenFrame::reflashPageView(AppsListModel::AppCategory category)
{
    if (AppsListModel::Search == category) {
        m_multiPagesView->ShowPageView(AppsListModel::AppCategory(m_displayMode));
    } else {
        m_multiPagesView->updatePageCount(category);
    }

    if (m_calcUtil->displayMode() == GROUP_BY_CATEGORY) {
        for (int i = AppsListModel::Internet; i <= AppsListModel::Others; i++) {
            getCategoryGridViewList(AppsListModel::AppCategory(i))->updatePageCount(AppsListModel::AppCategory(i));
        }
    }

    if (m_displayMode == GROUP_BY_CATEGORY && AppsListModel::Search == category) {
        checkCurrentCategoryVisible();
        hideCategoryBoxWidget();
        showCategoryBoxWidget(m_currentCategory);
    }
}

void FullScreenFrame::primaryScreenChanged()
{
    updateBackground();
    updateBlurBackground();
    setFixedSize(m_appsManager->currentScreen()->size());
    updateDockPosition();
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

    m_internetBoxWidget->setVisible(false);
    m_chatBoxWidget->setVisible(false);
    m_musicBoxWidget->setVisible(false);
    m_videoBoxWidget->setVisible(false);
    m_graphicsBoxWidget->setVisible(false);
    m_gameBoxWidget->setVisible(false);
    m_officeBoxWidget->setVisible(false);
    m_readingBoxWidget->setVisible(false);
    m_developmentBoxWidget->setVisible(false);
    m_systemBoxWidget->setVisible(false);
    m_othersBoxWidget->setVisible(false);

    if (m_displayMode == GROUP_BY_CATEGORY) {
        //隐藏自由模式显示
        m_appsIconBox->setVisible(false);
        AppsListModel::AppCategory category = (m_displayMode == SEARCH) ? AppsListModel::Search : AppsListModel::All;
        m_multiPagesView->setModel(category);
        //再显示分类模式
        m_navigationWidget->setVisible(true);
        m_appsItemBox->setVisible(true);
    } else {
        //先刷新后端数据
        AppsListModel::AppCategory category = (m_displayMode == SEARCH) ? AppsListModel::Search : AppsListModel::All;
        m_multiPagesView->setModel(category);

        //设置自由模式wdiget大小
        QSize boxSize;
        int padding = m_calcUtil->getScreenSize().width() * SIDES_SPACE_SCALE;
        const int appsContentWidth = (m_contentFrame->width() - padding * 2);
        const int appsContentHeight = (m_contentFrame->height() - DLauncher::APPS_AREA_TOP_MARGIN);
        boxSize.setWidth(appsContentWidth);
        boxSize.setHeight(appsContentHeight);

        //隐藏分类模式显示
        m_appsIconBox->setFixedSize(boxSize);
        m_multiPagesView->setFixedSize(boxSize);
        m_multiPagesView->updatePosition();

        m_navigationWidget->setVisible(false);
        m_appsItemBox->setVisible(false);

        //再显示自由显示模式
        m_appsIconBox->setVisible(true);
    }

    m_appItemDelegate->setCurrentIndex(QModelIndex());

    hideTips();

    emit displayModeChanged(m_displayMode);
}

void FullScreenFrame::updateDockPosition()
{
    const QRect dockGeometry = m_appsManager->dockGeometry();

    int bottomMargin = (m_displayMode == GROUP_BY_CATEGORY) ? m_calcUtil->getScreenSize().height() *0.064815 : 20;

    m_navigationWidget->updateSize();

    m_topSpacing->setFixedHeight(30);
    m_bottomSpacing->setFixedHeight(bottomMargin);

    switch (m_appsManager->dockPosition()) {
    case DOCK_POS_TOP:
        m_topSpacing->setFixedHeight(30 + dockGeometry.height());
        bottomMargin = m_topSpacing->height() + DLauncher::APPS_AREA_TOP_MARGIN;
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

    int padding = m_calcUtil->getScreenSize().width() * SIDES_SPACE_SCALE;

    if (m_displayMode == ALL_APPS || m_displayMode == SEARCH) {
        m_calcUtil->calculateAppLayout(m_contentFrame->size() - QSize(padding * 2, DLauncher::APPS_AREA_TOP_MARGIN), m_displayMode);
    } else {
        m_calcUtil->calculateAppLayout(m_contentFrame->size() - QSize(0, DLauncher::APPS_AREA_TOP_MARGIN + 12), m_displayMode);
    }
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

AppsListModel::AppCategory FullScreenFrame::nextCategoryType(const AppsListModel::AppCategory category)
{
    AppsListModel::AppCategory nextCategory = AppsListModel::AppCategory(category + 1);
    if (nextCategory > AppsListModel::Others)
        nextCategory = AppsListModel::Internet;

    if (m_appsManager->appNums(nextCategory) <= 0) {
        nextCategory = nextCategoryType(nextCategory);
    }

    return nextCategory;
}

AppsListModel::AppCategory FullScreenFrame::prevCategoryType(const AppsListModel::AppCategory category)
{
    AppsListModel::AppCategory prevCategory = AppsListModel::AppCategory(category - 1);
    if (prevCategory < AppsListModel::Internet)
        prevCategory = AppsListModel::Others;

    if (m_appsManager->appNums(prevCategory) <= 0) {
        prevCategory = prevCategoryType(prevCategory);
    }

    return prevCategory;
}

const QScreen *FullScreenFrame::currentScreen()
{
    return m_appsManager->currentScreen();
}

void FullScreenFrame::layoutChanged()
{
    QSize boxSize;
    int padding = m_calcUtil->getScreenSize().width() * SIDES_SPACE_SCALE;

    QPixmap pixmap = cachePixmap();
    if (m_displayMode == ALL_APPS || m_displayMode == SEARCH) {
        const int appsContentWidth = (m_contentFrame->width() - padding * 2);
        const int appsContentHeight = (m_contentFrame->height() - DLauncher::APPS_AREA_TOP_MARGIN);
        boxSize.setWidth(appsContentWidth);
        boxSize.setHeight(appsContentHeight);

        m_appsIconBox->setFixedSize(boxSize);
        m_multiPagesView->setFixedSize(boxSize);
        m_multiPagesView->updatePosition();
    } else {
        const int appsContentWidth = (m_contentFrame->width());
        const int appsContentHeight = (m_contentFrame->height() - DLauncher::APPS_AREA_TOP_MARGIN);
        boxSize.setWidth(appsContentWidth);
        boxSize.setHeight(appsContentHeight);

        m_appsItemSeizeBox->setFixedSize(boxSize);
        m_appsItemBox->setFixedSize(boxSize);

        boxSize.setWidth(m_calcUtil->getAppBoxSize().width());
        boxSize.setHeight(boxSize.height());

        for (int i = AppsListModel::Internet; i <= AppsListModel::Others; i++) {
            AppsListModel::AppCategory appCategory = AppsListModel::AppCategory(i);
            BlurBoxWidget * boxWidget = getCategoryBoxWidget(appCategory);
            getCategoryGridViewList(AppsListModel::AppCategory(i))->updatePageCount(AppsListModel::AppCategory(i));
            boxWidget->setFixedSize(boxSize);
        }

        checkCurrentCategoryVisible();
        showCategoryBoxWidget(m_currentCategory);
    }
}

void FullScreenFrame::searchTextChanged(const QString &keywords)
{
    m_appsManager->searchApp(keywords.trimmed());

    if (!m_searchWidget->edit()->lineEdit()->hasFocus()) return;

    if (keywords.isEmpty())
        updateDisplayMode(m_calcUtil->displayMode());
    else
        updateDisplayMode(SEARCH);

    if (m_searchWidget->edit()->lineEdit()->text().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->clearFocus();
    }
}

