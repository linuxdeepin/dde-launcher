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

#include "applistview.h"
#include "global_util/constants.h"
#include "delegate/applistdelegate.h"
#include "model/appslistmodel.h"

#include <QStyleOptionViewItem>
#include <QPropertyAnimation>
#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPainter>
#include <QTimer>
#include <QLabel>
#include <QDebug>
#include <QDrag>

AppListView::AppListView(QWidget *parent)
    : QListView(parent)
    , m_dropThresholdTimer(new QTimer(this))
    , m_scrollAni(new QPropertyAnimation(verticalScrollBar(), "value"))
    , m_opacityEffect(new QGraphicsOpacityEffect(this))
    , m_wmHelper(DWindowManagerHelper::instance())
{
    m_scrollAni->setEasingCurve(QEasingCurve::OutQuint);
    m_scrollAni->setDuration(800);

    horizontalScrollBar()->setEnabled(false);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(SingleSelection);
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
    setFixedHeight(510);

    // support drag and drop.
    setDragDropMode(QAbstractItemView::DragDrop);
    setMovement(QListView::Free);
    setDragEnabled(true);

    // init opacity effect.
    m_opacityEffect->setOpacity(1);
    setGraphicsEffect(m_opacityEffect);

    // init drop threshold timer.
    m_dropThresholdTimer->setInterval(DLauncher::APP_DRAG_SWAP_THRESHOLD);
    m_dropThresholdTimer->setSingleShot(true);

#ifndef DISABLE_DRAG_ANIMATION
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppListView::prepareDropSwap, Qt::QueuedConnection);
#else
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppListView::dropSwap);
#endif
}

const QModelIndex AppListView::indexAt(const int index) const
{
    return model()->index(index, 0, QModelIndex());
}

void AppListView::wheelEvent(QWheelEvent *e)
{
    // only support all programs rolling.
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel || listModel->category() != AppsListModel::All)
        return;

    if (e->pixelDelta().y() == 0) {
        QWheelEvent ve(e->pos(), e->globalPos(), e->pixelDelta(),
                       e->angleDelta(), e->delta() * 16,
                       Qt::Vertical, e->buttons(), e->modifiers());
        QListView::wheelEvent(&ve);
    } else {
        int offset = -e->delta();

        m_scrollAni->stop();
        m_scrollAni->setStartValue(verticalScrollBar()->value());
        m_scrollAni->setEndValue(verticalScrollBar()->value() + offset * m_speedTime);
        m_scrollAni->start();
    }
}

void AppListView::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();

    setState(NoState);

    const QModelIndex &index = indexAt(e->pos());
    const QPoint pos = e->pos();

    if (index.isValid() && !m_enableDropInside)
        Q_EMIT entered(index);

    if (e->buttons() != Qt::LeftButton)
        return;

    if (qAbs(pos.x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
        qAbs(pos.y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD) {
        m_dragStartPos = e->pos();
        m_dragStartRow = index.row();
        return startDrag(index);
    }
}

void AppListView::mousePressEvent(QMouseEvent *e)
{
    QListView::mousePressEvent(e);

    const QModelIndex &index = indexAt(e->pos());
    if (!index.isValid())
        e->ignore();

    if (e->buttons() == Qt::RightButton) {
        const QPoint rightClickPoint = mapToGlobal(e->pos());
        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());

        if (clickedIndex.isValid())
            emit popupMenuRequested(rightClickPoint, clickedIndex);
    }

    if (e->buttons() == Qt::LeftButton) {
        m_dragStartPos = e->pos();
        m_dragStartRow = indexAt(e->pos()).row();
    }
}

void AppListView::dragEnterEvent(QDragEnterEvent *e)
{
    const QModelIndex index = indexAt(e->pos());

    if (model()->canDropMimeData(e->mimeData(), e->dropAction(), index.row(), index.column(), QModelIndex())) {
        // to enable transparent.
        m_opacityEffect->setOpacity(0.5);

        return e->accept();
    }
}

void AppListView::dragMoveEvent(QDragMoveEvent *e)
{
    if (m_lastFakeAni)
        return;

    const QModelIndex dropIndex = QListView::indexAt(e->pos());
    if (dropIndex.isValid())
        m_dropToRow = dropIndex.row();

    m_dropThresholdTimer->stop();

    const QPoint pos = e->pos();
    const QRect rect = this->rect();

    if (pos.y() < DRAG_SCROLL_THRESHOLD) {
        Q_EMIT requestScrollUp();
    } else if (pos.y() > rect.height() - DRAG_SCROLL_THRESHOLD) {
        Q_EMIT requestScrollDown();
    } else {
        Q_EMIT requestScrollStop();

        // rolling without swapping.
        m_dropThresholdTimer->start();
    }

    // drag move does not allow to have selected effect.
    Q_EMIT entered(QModelIndex());
}

void AppListView::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();

    // drag leave will also restore opacity.
    m_opacityEffect->setOpacity(1);
    m_dropThresholdTimer->stop();

    Q_EMIT requestScrollStop();
}

void AppListView::dropEvent(QDropEvent *e)
{
    e->accept();

    // restore opacity.
    m_opacityEffect->setOpacity(1);
    m_enableDropInside = true;
}

void AppListView::startDrag(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel || listModel->category() != AppsListModel::All)
        return;

    const QModelIndex &dragIndex = index;
    const auto ratio = devicePixelRatioF();
    const QSize rectSize = QSize(320, 51);

    QPixmap dropPixmap;
    QPixmap sourcePixmap(rectSize * ratio);
    sourcePixmap.fill(Qt::transparent);
    sourcePixmap.setDevicePixelRatio(ratio);

    QPainter painter(&sourcePixmap);
    QStyleOptionViewItem item;
    item.rect = QRect(QPoint(0, 0), rectSize);
    item.features |= QStyleOptionViewItem::Alternate;

    itemDelegate()->paint(&painter, item, index);
    dropPixmap = sourcePixmap;

    // wm support transparent to draw a shadow effect.
    if (m_wmHelper->hasComposite()) {
        QColor shadowColor("#2CA7F8");
        shadowColor.setAlpha(180);
        dropPixmap = AppListDelegate::dropShadow(sourcePixmap, 50, shadowColor, QPoint(0, 8));
        dropPixmap.setDevicePixelRatio(ratio);
    }

    QDrag *drag = new QDrag(this);
    drag->setMimeData(model()->mimeData(QModelIndexList() << dragIndex));
    drag->setPixmap(dropPixmap);
    drag->setHotSpot(dropPixmap.rect().center() / dropPixmap.devicePixelRatioF());

    // request remove current item.
    if (listModel->category() == AppsListModel::All) {
        m_dropToRow = index.row();
        listModel->setDraggingIndex(index);
    }

    setState(DraggingState);
    drag->exec(Qt::MoveAction);

    // disable animation when finally dropped
    m_dropThresholdTimer->stop();

    // disable auto scroll
    Q_EMIT requestScrollStop();

    if (listModel->category() != AppsListModel::All)
        return;

    if (!m_lastFakeAni) {
        if (m_enableDropInside)
            listModel->dropSwap(m_dropToRow);
        else
            listModel->dropSwap(m_dragStartRow);

        listModel->clearDraggingIndex();
    } else {
        connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDraggingIndex);
    }

    m_enableDropInside = false;
}

void AppListView::prepareDropSwap()
{
    if (m_lastFakeAni || m_dropThresholdTimer->isActive())
        return;

    const QModelIndex dropIndex = indexAt(m_dropToRow);
    if (!dropIndex.isValid())
        return;

    const QModelIndex dragStartIndex = indexAt(m_dragStartRow);
    if (dropIndex == dragStartIndex)
        return;

    if (!dragStartIndex.isValid()) {
        m_dragStartRow = dropIndex.row();
        return;
    }

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->clearDraggingIndex();
    listModel->setDraggingIndex(dragStartIndex);
    listModel->setDragDropIndex(dropIndex);

    const int startIndex = dragStartIndex.row();
    const bool moveToNext = startIndex <= m_dropToRow;
    const int start = moveToNext ? startIndex : m_dropToRow;
    const int end = !moveToNext ? startIndex : m_dropToRow;

    if (start == end)
        return;

    for (int i = start + moveToNext; i != end - !moveToNext; ++i) {
        createFakeAnimation(i, moveToNext);
    }

    // last animation.
    createFakeAnimation(end - !moveToNext, moveToNext, true);

    m_dragStartRow = dropIndex.row();
}

void AppListView::createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni)
{
    const QModelIndex index(indexAt(pos));

    QLabel *floatLabel = new QLabel(this);
    QPropertyAnimation *animation = new QPropertyAnimation(floatLabel, "pos", floatLabel);

    const auto ratio = devicePixelRatioF();
    const QSize rectSize(320, 51);

    QPixmap pixmap(rectSize * ratio);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(ratio);

    QStyleOptionViewItem item;
    item.rect = QRect(QPoint(0, 0), rectSize);
    item.features |= QStyleOptionViewItem::HasDisplay;

    QPainter painter(&pixmap);
    itemDelegate()->paint(&painter, item, index);

    floatLabel->setFixedSize(rectSize);
    floatLabel->setPixmap(pixmap);
    floatLabel->show();

    animation->setStartValue(visualRect(index).topLeft());
    animation->setEndValue(visualRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft());
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setDuration(200);

    connect(animation, &QPropertyAnimation::finished, floatLabel, &QLabel::deleteLater);

    if (isLastAni) {
        m_lastFakeAni = animation;
        connect(animation, &QPropertyAnimation::finished, this, &AppListView::dropSwap, Qt::QueuedConnection);
        connect(animation, &QPropertyAnimation::valueChanged, m_dropThresholdTimer, &QTimer::stop);
    }

    animation->start();
}

void AppListView::dropSwap()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());

    if (!listModel)
        return;

    listModel->dropSwap(m_dropToRow);

    m_lastFakeAni = nullptr;
    m_dragStartRow = m_dropToRow;

    setState(NoState);
}
