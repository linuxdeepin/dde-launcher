/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     wangleiyong <wangleiyong_cm@deepin.com>
 *
 * Maintainer: wangleiyong <wangleiyong_cm@deepin.com>
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

#include "multipagesview.h"

#include <QHBoxLayout>

MultiPagesView::MultiPagesView(AppsListModel::AppCategory categoryModel, int space, QWidget *parent)
    : QWidget(parent)
    , m_pageCount(0)
    , m_pageIndex(0)
    , m_viewSpace(space)
    , m_appModel(categoryModel)
    , m_appsManager(AppsManager::instance())
    , m_calcUtil(CalculateUtil::instance())
    , m_appListArea(new AppListArea)
    , m_pHBoxLayout(new DHBoxWidget)
    , m_iconLayout(new QHBoxLayout)
{
    m_iconViewActive = QIcon(":/widgets/images/page_indicator_active.svg");
    m_iconView = QIcon(":/widgets/images/page_indicator.svg");

    m_appListArea->setObjectName("MultiPageBox");
    m_appListArea->viewport()->setAutoFillBackground(false);
    m_appListArea->setWidgetResizable(true);
    m_appListArea->setFocusPolicy(Qt::NoFocus);
    m_appListArea->setFrameStyle(QFrame::NoFrame);
    m_appListArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appListArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appListArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appListArea->viewport()->installEventFilter(this);
    m_appListArea->installEventFilter(this);

    pageSwitchAnimation = new QPropertyAnimation(m_appListArea->horizontalScrollBar(), "value");
    pageSwitchAnimation->setEasingCurve(QEasingCurve::OutQuad);

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &MultiPagesView::layoutChanged, Qt::QueuedConnection);
    connect(m_appListArea, &AppListArea::increaseIcon, this, [ = ] { m_calcUtil->increaseIconSize(); emit m_appsManager->layoutChanged(m_appModel); });
    connect(m_appListArea, &AppListArea::decreaseIcon, this, [ = ] { m_calcUtil->decreaseIconSize(); emit m_appsManager->layoutChanged(m_appModel); });
}

void MultiPagesView::UpdatePageCount(int pageCount)
{
    if (0 == pageCount || pageCount == m_pageCount)
        return;

    if (pageCount > m_pageCount) {
        while (pageCount > m_pageCount) {
            AppsListModel *pModel = new AppsListModel(m_appModel);
            pModel->setPageIndex(m_pageCount);
            m_pageAppsModelList.push_back(pModel);

            AppGridView *pView = new AppGridView;
            pView->setModel(pModel);
            pView->setItemDelegate(m_delegate);
            pView->setContainerBox(m_appListArea);
            pView->installEventFilter(this);
            m_appGridViewList.push_back(pView);

            DFloatingButton *floatBtn = new DFloatingButton(this);
            floatBtn->setIcon(m_iconView);
            floatBtn->setIconSize(QSize(20, 20));
            floatBtn->setFixedSize(QSize(20, 20));
            floatBtn->setBackgroundRole(DPalette::Button);
            connect(floatBtn, &DFloatingButton::clicked, this, &MultiPagesView::clickIconBtn);
            m_floatBtnList.push_back(floatBtn);

            m_pHBoxLayout->layout()->insertWidget(m_pageCount, pView);
            m_iconLayout->insertWidget(m_pageCount + 1, floatBtn);

            m_pageCount ++;
        }
    } else {
        while (pageCount < m_pageCount) {
            QLayoutItem *item = m_pHBoxLayout->layout()->itemAt(m_pageCount);
            m_pHBoxLayout->layout()->removeItem(item);
            item->widget()->setParent(nullptr);

            item = m_iconLayout->itemAt(pageCount);
            m_iconLayout->removeItem(item);
            item->widget()->setParent(nullptr);

            m_pageAppsModelList.removeLast();
            m_appGridViewList.removeLast();
            m_floatBtnList.removeLast();

            m_pageCount --;
        }
    }
    showCurrentPage(0);
}

QModelIndex MultiPagesView::getAppItem(int index)
{
    return m_appGridViewList[m_pageIndex]->indexAt(index);
}

void MultiPagesView::setDataDelegate(QAbstractItemDelegate *delegate)
{
    m_delegate = delegate;
    Init();
}

void MultiPagesView::Init()
{
    m_pHBoxLayout->layout()->setSpacing(m_viewSpace);

    m_iconLayout->setMargin(0);
    m_iconLayout->setSpacing(ICON_SPACE);
    m_iconLayout->addStretch();
    m_iconLayout->addStretch();

    QVBoxLayout *scrollVLayout = new QVBoxLayout;
    scrollVLayout->setMargin(0);
    scrollVLayout->setSpacing(0);
    scrollVLayout->addWidget(m_pHBoxLayout, 0, Qt::AlignTop);

    QFrame *m_contentFrame = new QFrame;
    m_contentFrame->setAttribute(Qt::WA_TranslucentBackground);
    m_contentFrame->setLayout(scrollVLayout);

    m_appListArea->setWidget(m_contentFrame);

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setMargin(0);
    layoutMain->setSpacing(0);
    layoutMain->addWidget(m_appListArea);
    layoutMain->addLayout(m_iconLayout);

    setLayout(layoutMain);
}

void MultiPagesView::showCurrentPage(int currentPage)
{
    m_pageIndex = currentPage;
    int endValue = m_appGridViewList[currentPage]->x();
    int startValue = m_appListArea->horizontalScrollBar()->value();
    pageSwitchAnimation->stop();
    pageSwitchAnimation->setStartValue(startValue);
    pageSwitchAnimation->setEndValue(endValue);
    pageSwitchAnimation->start();

    bool bVisible = m_pageCount > 1;

    for (int i = 0; i < m_pageCount; i++) {
        m_floatBtnList[i]->setIcon(m_iconView);
        m_floatBtnList[i]->setVisible(bVisible);
    }
    m_floatBtnList[m_pageIndex]->setIcon(m_iconViewActive);
}

QModelIndex MultiPagesView::selectApp(const int key)
{
    int page = m_pageIndex;
    int itemSelect = -1;
    if (Qt::Key_Left == key || Qt::Key_Up == key) {
        if (page - 1 >= 0) {
            -- page;
            itemSelect = m_calcUtil->appPageItemCount() - 1;
        }
    } else {
        if (page + 1 < m_pageCount) {
            ++ page;
            itemSelect = 0;
        }
    }
    if (page != m_pageIndex)
        showCurrentPage(page);

    return m_appGridViewList[m_pageIndex]->indexAt(itemSelect);
}

AppGridView *MultiPagesView::pageView(int pageIndex)
{
    if (pageIndex >= m_pageCount)
        return nullptr;

    return m_appGridViewList[pageIndex];
}

AppsListModel *MultiPagesView::pageModel(int pageIndex)
{
    if (pageIndex >= m_pageCount)
        return nullptr;

    return m_pageAppsModelList[pageIndex];
}

void MultiPagesView::clickIconBtn()
{
    for (int i = 0; i < m_pageCount; i++) {
        if (sender() == m_floatBtnList[i]) {
            showCurrentPage(i);
            break;
        }
    }
}

void MultiPagesView::layoutChanged()
{
    const int appsContentWidth = width();
    QSize boxSize;
    boxSize.setWidth(appsContentWidth);
    boxSize.setHeight(m_appListArea->height());
    m_pHBoxLayout->setFixedHeight(m_appListArea->height());
    for (auto *pView : m_appGridViewList)
        pView->setFixedSize(boxSize);

}

void MultiPagesView::wheelEvent(QWheelEvent *e)
{
    if (pageSwitchAnimation->state() == QPropertyAnimation::Running)
        return;

    int page = m_pageIndex;
    if (e->delta() > 0) {
        if (page - 1 >= 0)
            -- page;
    } else if (e->delta() < 0) {
        if (page + 1 < m_pageCount)
            ++ page;
    }

    if (page != m_pageIndex)
        showCurrentPage(page);
}

bool MultiPagesView::eventFilter(QObject *o, QEvent *e)
{
//    if (Qt::Key_Down == e->type())
//    {
//        return true;
//    }
    return false;
}
