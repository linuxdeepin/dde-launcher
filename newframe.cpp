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

#include "newframe.h"
#include "global_util/util.h"

#include <DDBusSender>
#include <QHBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QScreen>
#include <QTimer>
#include <QDebug>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

#define DOCK_FASHION    0
#define DOCK_EFFICIENT  1

NewFrame::NewFrame(QWidget *parent)
    : DBlurEffectWidget(parent),
      m_dockInter(new DBusDock(this)),
      m_eventFilter(new SharedEventFilter(this)),
      m_windowHandle(this, this),
      m_wmHelper(DWindowManagerHelper::instance()),
      m_appsManager(AppsManager::instance()),
      m_appsView(new AppListView),
      m_appsModel(new AppsListModel(AppsListModel::All)),
      m_searchModel(new AppsListModel(AppsListModel::Search)),
      m_searchWidget(new SearchWidget),
      m_rightBar(new MiniFrameRightBar)
{
    m_windowHandle.setShadowRadius(60);
    m_windowHandle.setShadowOffset(QPoint(0, -1));
    m_windowHandle.setEnableBlurWindow(false);
    m_windowHandle.setTranslucentBackground(true);

    m_appsView->setModel(m_appsModel);
    m_appsView->setItemDelegate(new AppListDelegate);

    m_searchWidget->installEventFilter(m_eventFilter);

    QVBoxLayout *appsLayout = new QVBoxLayout;
    appsLayout->addSpacing(10);
    appsLayout->addWidget(m_searchWidget);
    appsLayout->addSpacing(10);
    appsLayout->addWidget(m_appsView);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addSpacing(10);
    mainLayout->addLayout(appsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_rightBar);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setMaskColor(DBlurEffectWidget::DarkColor);
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::ClickFocus);
    setFixedSize(580, 635);
    setLayout(mainLayout);
    setObjectName("MiniFrame");
    setStyleSheet(getQssFromFile(":/skin/qss/miniframe.qss"));

    installEventFilter(m_eventFilter);

    connect(m_rightBar, &MiniFrameRightBar::modeToggleBtnClicked, this, &NewFrame::onToggleFullScreen);
    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &NewFrame::onWMCompositeChanged);
    connect(m_searchWidget->edit(), &SearchLineEdit::textChanged, this, &NewFrame::searchText, Qt::QueuedConnection);

    connect(m_appsView, &QListView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_appsView, &QListView::clicked, this, &NewFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_appsView, &QListView::entered, m_appsView, &AppListView::setCurrentIndex);

    QTimer::singleShot(1, this, &NewFrame::onWMCompositeChanged);
}

NewFrame::~NewFrame()
{
    m_eventFilter->deleteLater();
}

void NewFrame::showLauncher()
{
    if (visible()) {
        return;
    }

    m_searchWidget->clearSearchContent();
    qApp->processEvents();

    adjustPosition();
    show();

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &NewFrame::adjustPosition, Qt::UniqueConnection);
}

void NewFrame::hideLauncher()
{
    if (!visible()) {
        return;
    }

    disconnect(m_dockInter, &DBusDock::FrontendRectChanged, this, &NewFrame::adjustPosition);

    hide();
}

bool NewFrame::visible()
{
    return isVisible();
}

void NewFrame::moveCurrentSelectApp(const int key)
{

}

void NewFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->setFocus(Qt::MouseFocusReason);
    m_searchWidget->edit()->setText(m_searchWidget->edit()->text() + ch);
}

void NewFrame::launchCurrentApp()
{

}

void NewFrame::uninstallApp(const QString &appKey)
{

}

bool NewFrame::windowDeactiveEvent()
{

}

void NewFrame::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton)
        hideLauncher();
}

void NewFrame::keyPressEvent(QKeyEvent *e)
{
    DBlurEffectWidget::keyPressEvent(e);

    switch (e->key()) {
    case Qt::Key_Escape:
        hideLauncher();
        break;
    default:
        break;
    }
}

void NewFrame::showEvent(QShowEvent *e)
{
    DBlurEffectWidget::showEvent(e);

    QTimer::singleShot(1, this, [this] () {
        raise();
        activateWindow();
        setFocus();
        emit visibleChanged(true);
    });
}

void NewFrame::hideEvent(QHideEvent *e)
{
    DBlurEffectWidget::hideEvent(e);

    QTimer::singleShot(1, this, [=] { emit visibleChanged(false); });
}

void NewFrame::enterEvent(QEvent *e)
{
    DBlurEffectWidget::enterEvent(e);

    raise();
    activateWindow();
    setFocus();
}

const QPoint NewFrame::scaledPosition(const QPoint &xpos)
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

void NewFrame::adjustPosition()
{
    const auto ratio = devicePixelRatioF();
    const int dockPos = m_dockInter->position();
    const QRect &r = m_dockInter->frontendRect();
    const QRect &dockRect = QRect(scaledPosition(r.topLeft()), r.size() / ratio);

    const int spacing = 0;
    const QSize s = size();
    QPoint p;

    // extra spacing for efficient mode
    if (m_dockInter->displayMode() == DOCK_EFFICIENT) {
        const QRect primaryRect = qApp->primaryScreen()->geometry();

        switch (dockPos) {
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
        default:
            Q_UNREACHABLE_IMPL();
        }
    } else {
        switch (dockPos) {
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
        default:
            Q_UNREACHABLE_IMPL();
        }
    }

    move(p);
}

void NewFrame::onToggleFullScreen()
{
    removeEventFilter(m_eventFilter);

    DDBusSender()
            .service("com.deepin.dde.daemon.Launcher")
            .interface("com.deepin.dde.daemon.Launcher")
            .path("/com/deepin/dde/daemon/Launcher")
            .property("Fullscreen")
            .set(true);
}

void NewFrame::onWMCompositeChanged()
{
    if (m_wmHelper->hasComposite()) {
        m_windowHandle.setWindowRadius(5);
        m_windowHandle.setBorderColor(QColor(255, 255, 255, .1 * 255));
    } else {
        m_windowHandle.setWindowRadius(0);
        m_windowHandle.setBorderColor(QColor("#2C3238"));
    }
}

void NewFrame::searchText(const QString &text)
{
    if (text.isEmpty()) {
        m_appsView->setModel(m_appsModel);
    } else {
        if (m_appsView->model() != m_searchModel) {
            m_appsView->setModel(m_searchModel);
        }

        m_appsManager->searchApp(text.trimmed());
    }
}
