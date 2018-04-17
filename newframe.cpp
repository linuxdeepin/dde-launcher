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
#include <ddialog.h>

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

NewFrame::NewFrame(QWidget *parent)
    : DBlurEffectWidget(parent),
      m_dockInter(new DBusDock(this)),
      m_menuWorker(new MenuWorker),
      m_eventFilter(new SharedEventFilter(this)),
      m_windowHandle(this, this),
      m_wmHelper(DWindowManagerHelper::instance()),
      m_appsManager(AppsManager::instance()),
      m_appsView(new AppListView),
      m_appsModel(new AppsListModel(AppsListModel::All)),
      m_usedModel(new AppsListModel(AppsListModel::Used)),
      m_searchModel(new AppsListModel(AppsListModel::Search)),
      m_searchWidget(new SearchWidget),
      m_rightBar(new MiniFrameRightBar),
      m_switchBtn(new MiniFrameSwitchBtn),
      m_delayHideTimer(new QTimer)
{
    m_windowHandle.setShadowRadius(60);
    m_windowHandle.setShadowOffset(QPoint(0, -1));
    m_windowHandle.setEnableBlurWindow(false);
    m_windowHandle.setTranslucentBackground(true);

    m_appsView->setModel(m_usedModel);
    m_appsView->setItemDelegate(new AppListDelegate);

    m_searchWidget->installEventFilter(m_eventFilter);

    m_delayHideTimer->setInterval(200);
    m_delayHideTimer->setSingleShot(true);

    QVBoxLayout *appsLayout = new QVBoxLayout;
    appsLayout->addSpacing(10);
    appsLayout->addWidget(m_searchWidget);
    appsLayout->addSpacing(10);
    appsLayout->addWidget(m_appsView);
    appsLayout->addSpacing(10);
    appsLayout->addWidget(m_switchBtn);
    appsLayout->addSpacing(15);

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
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, this, static_cast<void (NewFrame::*)(const QModelIndex &)>(&NewFrame::uninstallApp));
    connect(m_menuWorker.get(), &MenuWorker::menuAccepted, m_delayHideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &NewFrame::hideLauncher);

    connect(m_appsView, &QListView::clicked, m_appsManager, &AppsManager::launchApp, Qt::QueuedConnection);
    connect(m_appsView, &QListView::clicked, this, &NewFrame::hideLauncher, Qt::QueuedConnection);
    connect(m_appsView, &QListView::entered, m_appsView, &AppListView::setCurrentIndex);
    connect(m_appsView, &AppListView::popupMenuRequested, m_menuWorker.get(), &MenuWorker::showMenuByAppItem);

    connect(m_delayHideTimer, &QTimer::timeout, this, &NewFrame::prepareHideLauncher);

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
    const QModelIndex currentIdx = m_appsView->currentIndex();
    QModelIndex targetIndex;

    const int row = currentIdx.row();

    switch (key) {
    case Qt::Key_Down:
        targetIndex = currentIdx.sibling(row + 1, 0);
        break;
    case Qt::Key_Up:
        targetIndex = currentIdx.sibling(row - 1, 0);
        break;
    }

    if (!currentIdx.isValid() || !targetIndex.isValid()) {
        targetIndex = m_appsView->model()->index(0, 0);
    }

    m_appsView->setCurrentIndex(targetIndex);
}

void NewFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->setFocus(Qt::MouseFocusReason);
    m_searchWidget->edit()->setText(m_searchWidget->edit()->text() + ch);
}

void NewFrame::launchCurrentApp()
{
    const QModelIndex currentIdx = m_appsView->currentIndex();

    if (currentIdx.isValid() && currentIdx.model() == m_appsView->model()) {
        m_appsManager->launchApp(currentIdx);
    } else {
        m_appsManager->launchApp(m_appsView->model()->index(0, 0));
    }

    hideLauncher();
}

void NewFrame::uninstallApp(const QString &appKey)
{
    uninstallApp(m_appsModel->indexAt(appKey));
}

void NewFrame::uninstallApp(const QModelIndex &context)
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
    QPixmap appIcon = context.data(AppsListModel::AppIconRole).value<QPixmap>();
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
    QTimer::singleShot(1, this, &NewFrame::hideLauncher);

    unInstallDialog.exec();
    UNINSTALL_DIALOG_SHOWN = false;
}

bool NewFrame::windowDeactiveEvent()
{
    if (isVisible() && !m_menuWorker->isMenuShown()) {
        m_delayHideTimer->start();
    }

    return false;
}

void NewFrame::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton) {
        hideLauncher();
    }
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

    m_delayHideTimer->stop();

    raise();
    activateWindow();
    setFocus();
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

void NewFrame::prepareHideLauncher()
{
    if (!visible()) {
        return;
    }

    if (geometry().contains(QCursor::pos())) {
        return activateWindow(); /* get focus back */
    }

    hideLauncher();
}
