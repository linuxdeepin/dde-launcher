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

MultiPagesView::MultiPagesView(AppsListModel::AppCategory categoryModel, QWidget *parent)
    : QWidget(parent)
    , m_pageCount(0)
    , m_pageIndex(0)
    , m_category(categoryModel)
    , m_appsManager(AppsManager::instance())
    , m_calcUtil(CalculateUtil::instance())
    , m_appListArea(new AppListArea)
    , m_viewBox(new DHBoxWidget)
    , m_pageLayout(new QHBoxLayout)
    , m_pageControl(new pageControl)
{
    // 滚动区域
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

    // 翻页按钮和动画
    pageSwitchAnimation = new QPropertyAnimation(m_appListArea->horizontalScrollBar(), "value");
    pageSwitchAnimation->setEasingCurve(QEasingCurve::OutQuad);

    connect(m_appListArea, &AppListArea::increaseIcon, this, [ = ] { m_calcUtil->increaseIconSize(); emit m_appsManager->layoutChanged(AppsListModel::All); });
    connect(m_appListArea, &AppListArea::decreaseIcon, this, [ = ] { m_calcUtil->decreaseIconSize(); emit m_appsManager->layoutChanged(AppsListModel::All); });
    connect(m_pageControl, &pageControl::onPageChanged, this, &MultiPagesView::showCurrentPage);
}

void MultiPagesView::updatePageCount(AppsListModel::AppCategory category)
{
    int pageCount = m_appsManager->getPageCount(category == AppsListModel::All ? m_category : category);
    if (pageCount < 1)
        pageCount = 1;

    if (pageCount == m_pageCount)
        return;

    if (pageCount > m_pageCount) {
        while (pageCount > m_pageCount) {
            AppsListModel *pModel = new AppsListModel(m_category);
            pModel->setPageIndex(m_pageCount);
            m_pageAppsModelList.push_back(pModel);

            AppGridView *pageView = new AppGridView;
            pageView->setModel(pModel);
            pageView->setItemDelegate(m_delegate);
            pageView->setContainerBox(m_appListArea);
            pageView->installEventFilter(this);
            m_appGridViewList.push_back(pageView);

            m_viewBox->layout()->insertWidget(m_pageCount, pageView);

            m_pageCount ++;

            emit connectViewEvent(pageView);
        }
    } else {
        while (pageCount < m_pageCount) {
            AppGridView *pageView = qobject_cast<AppGridView *>(m_viewBox->layout()->itemAt(m_pageCount - 1)->widget());
            m_viewBox->layout()->removeWidget(pageView);
            pageView->deleteLater();

            m_pageAppsModelList.removeLast();
            m_appGridViewList.removeLast();

            m_pageCount --;
        }
    }

    m_pageControl->setPageCount(m_pageCount > 1 ? pageCount : 0);
}

QModelIndex MultiPagesView::getAppItem(int index)
{
    return m_appGridViewList[m_pageIndex]->indexAt(index);
}

void MultiPagesView::setDataDelegate(QAbstractItemDelegate *delegate)
{
    m_delegate = delegate;
    InitUI();
}

void MultiPagesView::ShowPageView(AppsListModel::AppCategory category)
{
    int pageCount = m_appsManager->getPageCount(category);
    for (int i = 0; i < m_pageCount; i++) {
        m_appGridViewList[i]->setVisible(i < pageCount);
    }
    m_pageControl->setPageCount(pageCount > 1 ? pageCount : 0);
}

void MultiPagesView::setModel(AppsListModel::AppCategory category)
{
    for (int i = 0; i < m_pageCount; i++) {
        m_pageAppsModelList[i]->setCategory(category);
        m_appGridViewList[i]->setModel(m_pageAppsModelList[i]);
    }
}

void MultiPagesView::updatePosition()
{
    QSize boxSize;
    boxSize.setWidth(width());
    boxSize.setHeight(m_appListArea->height());
    m_viewBox->setFixedHeight(m_appListArea->height());
    if (m_category >= AppsListModel::Internet) {
        boxSize.setHeight(m_calcUtil->getAppBoxSize().height());
        auto temp = m_pageControl->pos();
        m_pageControl->move(temp.x() - 5, temp.y() - 5);
    }
    for (auto *pView : m_appGridViewList)
        pView->setFixedSize(boxSize);
}

void MultiPagesView::InitUI()
{
    m_viewBox->setAttribute(Qt::WA_TranslucentBackground);
    m_appListArea->setWidget(m_viewBox);

    m_pageLayout->addWidget(m_pageControl, 0, Qt::AlignCenter);

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setMargin(0);
    layoutMain->setSpacing(0);
    layoutMain->addWidget(m_appListArea);
    layoutMain->addLayout(m_pageLayout);

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

    m_pageControl->setCurrent(currentPage);
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

void MultiPagesView::wheelEvent(QWheelEvent *e)
{
    if (AppsListModel::All != m_category)
        return;

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
