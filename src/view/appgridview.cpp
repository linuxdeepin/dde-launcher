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

#include "appgridview.h"
#include "src/global_util/constants.h"
#include "src/global_util/calculate_util.h"
#include "src/global_util/util.h"
#include "src/model/appslistmodel.h"

#include <QDebug>
#include <QWheelEvent>
#include <QTimer>
#include <QDrag>
#include <QMimeData>
#include <QtGlobal>
#include <QDrag>
#include <QPropertyAnimation>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>

QPointer<AppsManager> AppGridView::m_appManager = nullptr;
QPointer<CalculateUtil> AppGridView::m_calcUtil = nullptr;

AppGridView::AppGridView(QWidget *parent)
    : QListView(parent)
    , m_dropThresholdTimer(new QTimer(this))
    , m_pDelegate(nullptr)
{
    if (!m_appManager)
        m_appManager = AppsManager::instance();
    if (!m_calcUtil)
        m_calcUtil = CalculateUtil::instance();

    m_dropThresholdTimer->setInterval(DLauncher::APP_DRAG_SWAP_THRESHOLD);
    m_dropThresholdTimer->setSingleShot(true);

    viewport()->installEventFilter(this);
    viewport()->setAcceptDrops(true);

    setUniformItemSizes(true);
    setMouseTracking(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    setWrapping(true);
    setFocusPolicy(Qt::NoFocus);
    setDragDropMode(QAbstractItemView::DragDrop);
    setMovement(QListView::Free);
    setFlow(QListView::LeftToRight);
    setLayoutMode(QListView::Batched);
    setResizeMode(QListView::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setViewportMargins(0, 0, 0, 0);

    // init origin size
    setFixedSize(qApp->primaryScreen()->size());

    viewport()->setAutoFillBackground(false);

    // update item spacing
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, [this] { setSpacing(m_calcUtil->appItemSpacing());
                                                                      setViewportMargins(m_calcUtil->appMarginLeft(), m_calcUtil->appMarginTop(), m_calcUtil->appMarginLeft(), 0);
                                                                    });

#ifndef DISABLE_DRAG_ANIMATION
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppGridView::prepareDropSwap);
#else
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppGridView::dropSwap);
#endif

    connect(m_appManager,&AppsManager::iconLoadFinished,this,[=]{
        QListView::update();
    });
}

const QModelIndex AppGridView::indexAt(const int index) const
{
    return model()->index(index, 0, QModelIndex());
}

///
/// \brief AppListView::indexYOffset return item Y offset of current view
/// \param index item index
/// \return pixel of Y offset
///
int AppGridView::indexYOffset(const QModelIndex &index) const
{
    return indexRect(index).y();
}

void AppGridView::setContainerBox(const QWidget *container)
{
    m_containerBox = container;
}

// FIXME(sbw):
// some white/black lines occurred in non-integer scale hidpi.
void AppGridView::updateItemHiDPIFixHook(const QModelIndex &index)
{
    const QRect &r = indexRect(index);
    update(r.marginsRemoved(QMargins(0, 0, 1, 1)));
//    update(r.marginsAdded(QMargins(-1, -1, -1, -1)));
//    QListView::update(index);
}

void AppGridView::dropEvent(QDropEvent *e)
{
    qDebug("AppGridView::dropEvent");
    e->accept();

    m_enableDropInside = true;

    // set the correct hover item.
    emit entered(QListView::indexAt(e->pos()));
}

void AppGridView::mousePressEvent(QMouseEvent *e)
{
    qDebug("AppGridView::mousePressEvent");
    if (e->button() == Qt::RightButton) {
        QPoint rightClickPoint = mapToGlobal(e->pos());

        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());
        if (clickedIndex.isValid() && !m_moveGridView)
            emit popupMenuRequested(rightClickPoint, clickedIndex);
    }

    if (e->buttons() == Qt::LeftButton && !m_lastFakeAni)
        m_dragStartPos = e->pos();


    if (m_pDelegate != nullptr)
        m_pDelegate->mousePress(e);

    QListView::mousePressEvent(e);
}

void AppGridView::setDelegate(DragPageDelegate *pDelegate)
{
   m_pDelegate = pDelegate;
}

void AppGridView::dragEnterEvent(QDragEnterEvent *e)
{
    qDebug("AppGridView::dragEnterEvent");
    const QModelIndex index = indexAt(e->pos());

    if (model()->canDropMimeData(e->mimeData(), e->dropAction(), index.row(), index.column(), QModelIndex())) {
        emit entered(QModelIndex());
        return e->accept();
    }
}

void AppGridView::dragMoveEvent(QDragMoveEvent *e)
{
    qDebug("AppGridView::dragMoveEvent");
    Q_ASSERT(m_containerBox);
    Q_UNUSED(e);
    m_dropThresholdTimer->stop();
    if (m_lastFakeAni)
        return;

    const QPoint pos = e->pos();
    const QRect containerRect = this->rect();

    const QModelIndex dropIndex = QListView::indexAt(pos);
    if (dropIndex.isValid()){
        m_dropToPos = dropIndex.row();
    } else if ( containerRect.contains(pos)) {
        AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
        if (listModel) {
            int lastRow = listModel->rowCount(QModelIndex()) - 1;

            QModelIndex lastIndex = listModel->index(lastRow);

            if (lastIndex.isValid()){
                QPoint lastPos = indexRect(lastIndex).center();
                if ( pos.x() > lastPos.x() && pos.y() > lastPos.y() )
                    m_dropToPos = lastIndex.row();
            }
        }
    }

    emit requestScrollStop();

    if (m_enableAnimation)
        m_dropThresholdTimer->start();
}

void AppGridView::dragOut(int pos)
{
    qDebug("AppGridView::dragOut");
    m_enableAnimation = false;
    m_dropToPos = pos;

    prepareDropSwap();
    dropSwap();
}

void AppGridView::dragIn(const QModelIndex &index)
{
    qDebug("AppGridView::dragIn");

    m_enableAnimation = false;
    m_dragStartPos = indexRect(index).center();
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->setDraggingIndex(index);
}

void AppGridView::flashDrag()
{
    startDrag(indexAt(0));
}

void AppGridView::dragLeaveEvent(QDragLeaveEvent *e)
{
    qDebug("AppGridView::dragLeaveEvent");
    e->accept();
    Q_ASSERT(m_containerBox);

    m_dropThresholdTimer->stop();

    const QPoint pos = m_containerBox->mapFromGlobal(QCursor::pos());

    int nSpace = m_calcUtil->appItemSpacing() + m_calcUtil->appMarginLeft();
    const QRect containerRect = m_containerBox->rect().marginsRemoved(QMargins(nSpace, DLauncher::APP_DRAG_SCROLL_THRESHOLD,
                                                                               nSpace, DLauncher::APP_DRAG_SCROLL_THRESHOLD));

    const QModelIndex dropStart = QListView::indexAt(m_dragStartPos);

    if (pos.x() > containerRect.right())
        emit requestScrollRight(dropStart);
    else if (pos.x() < containerRect.left())
        emit requestScrollLeft(dropStart);

    QListView ::dragLeaveEvent(e);
}

void AppGridView::mouseMoveEvent(QMouseEvent *e)
{
    //qDebug("AppGridView::mouseMoveEvent");
    e->accept();

    // disable qlistview default drag
    setState(NoState);

    const QModelIndex &idx = indexAt(e->pos());

    if (idx.isValid())
        emit entered(idx);

    if (e->buttons() != Qt::LeftButton)
        return;

    if (qAbs(e->x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
        qAbs(e->y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD) {
        m_moveGridView = true;
        return startDrag(QListView::indexAt(m_dragStartPos));
    }

    if (m_pDelegate)
        m_pDelegate->mouseMove(e);
}

void AppGridView::mouseReleaseEvent(QMouseEvent *e)
{
    // request main frame hide when click invalid area
    qDebug("AppGridView::mouseReleaseEvent");
    if (e->button() != Qt::LeftButton)
        return;

    m_moveGridView = false;
    if (m_pDelegate)
        m_pDelegate->mouseRelease(e);

    QListView::mouseReleaseEvent(e);
}

void AppGridView::NotifyDragEnd()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    if (listModel->category() != AppsListModel::All)
        return;

    m_dropThresholdTimer->stop();

    if (!m_lastFakeAni) {
        if (m_enableDropInside)
            listModel->dropSwap(m_dropToPos);
        else
            listModel->dropSwap(indexAt(m_dragStartPos).row());

        listModel->clearDraggingIndex();
    } else {
        connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDraggingIndex);
    }

    m_enableDropInside = false;
}

void AppGridView::startDrag(const QModelIndex &index)
{
    qDebug("AppGridView::startDrag");
    if (!index.isValid())
        return;
    m_moveGridView = false;
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    const QModelIndex &dragIndex = index;
    const qreal ratio = qApp->devicePixelRatio();
    QString appKey = index.data(AppsListModel::AppKeyRole).value<QString>();

//    if(appKey == "dde-trash")
//        return;

    QPixmap srcPix;
    if (appKey== "dde-calendar") {
        const  auto  s = m_calcUtil->appIconSize();
        const double  iconZoom =  s.width() /64.0;
        QStringList calIconList = m_calcUtil->calendarSelectIcon();

        auto calendar = new QWidget() ;
        calendar->setFixedSize(s);

        calendar->setAutoFillBackground(true);
        QPalette palette = calendar->palette();
        palette.setBrush(QPalette::Window,
                         QBrush(QPixmap(calIconList.at(0)).scaled(
                                    calendar->size(),
                                    Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation)));
        calendar->setPalette(palette);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->setSpacing(0);
        auto month = new QLabel();
        auto monthPix = loadSvg(calIconList.at(1), QSize(20,10)*iconZoom);
        month->setPixmap(monthPix.scaled(monthPix.width(),monthPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        month->setFixedHeight(monthPix.height());
        month->setAlignment(Qt::AlignCenter);
        month->setFixedWidth(s.width() - 5 * iconZoom);
        layout->addWidget(month, Qt::AlignVCenter);

        auto day = new QLabel();
        auto dayPix = loadSvg(calIconList.at(2), QSize(28,26)*iconZoom);
        day->setPixmap(dayPix.scaled(dayPix.width(),dayPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        day->setAlignment(Qt::AlignCenter);
        day->setFixedHeight(day->pixmap()->height());
        day->raise();
        layout->addWidget(day, Qt::AlignVCenter);

        auto week = new QLabel();
        auto weekPix = loadSvg(calIconList.at(3), QSize(14,6)*iconZoom);
        week->setPixmap(weekPix.scaled(weekPix.width(),weekPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        week->setFixedHeight(week->pixmap()->height());
        week->setAlignment(Qt::AlignCenter);
        week->setFixedWidth(s.width() + 5 * iconZoom);
        layout->addWidget(week, Qt::AlignVCenter);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 10 * iconZoom, 0, 10 * iconZoom);
        calendar->setLayout(layout);
        srcPix = calendar->grab(calendar->rect());
    }else{
         srcPix = index.data(AppsListModel::AppDragIconRole).value<QPixmap>();
    }

    srcPix = srcPix.scaled(m_calcUtil->appIconSize() * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    srcPix.setDevicePixelRatio(ratio);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(model()->mimeData(QModelIndexList() << dragIndex));
    drag->setPixmap(srcPix);
    drag->setHotSpot(srcPix.rect().center() / ratio);

    // request remove current item.
    if (listModel->category() == AppsListModel::All) {
        m_dropToPos = index.row();
        listModel->setDraggingIndex(index);
    }

    setState(DraggingState);
    drag->exec(Qt::MoveAction);

    // disable animation when finally dropped
    m_dropThresholdTimer->stop();

    //send to next page
    emit dragEnd();

    // disable auto scroll
    if (listModel->category() != AppsListModel::All)
        return;

    if (!m_lastFakeAni) {
        if (m_enableDropInside)
            listModel->dropSwap(m_dropToPos);
        else
            listModel->dropSwap(indexAt(m_dragStartPos).row());

        listModel->clearDraggingIndex();
    } else {
        connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDraggingIndex);
    }

    m_enableDropInside = false;
    qDebug("AppGridView::startDrag end");
}

bool AppGridView::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    Q_UNUSED(e);
    return false;
}

void AppGridView::enterEvent(QEvent *e)
{
    qDebug("AppGridView::enterEvent");
    QListView::enterEvent(e);

    // The coordinates of the entervent are incorrect after the menu is closed.
    emit entered(indexAt(mapFromGlobal(QCursor::pos())));
}

///
/// \brief AppListView::fitToContent change view size to fit viewport content
///
void AppGridView::fitToContent()
{
    const QSize size { contentsRect().width(), contentsSize().height() };

    if (size == rect().size()) return;

    setFixedSize(size);
}

void AppGridView::prepareDropSwap()
{
    if (m_lastFakeAni || m_dropThresholdTimer->isActive() || !m_enableAnimation)
        return;
    const QModelIndex dropIndex = indexAt(m_dropToPos);
    if (!dropIndex.isValid())
        return;
    const QModelIndex dragStartIndex = indexAt(m_dragStartPos);
    if (dragStartIndex == dropIndex)
        return;

    // startPos may be in the space between two icons.
    if (!dragStartIndex.isValid()) {
        m_dragStartPos = indexRect(dropIndex).center();
        return;
    }

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->clearDraggingIndex();
    listModel->setDraggingIndex(dragStartIndex);
    listModel->setDragDropIndex(dropIndex);

    const int startIndex = dragStartIndex.row();
    const bool moveToNext = startIndex <= m_dropToPos;
    const int start = moveToNext ? startIndex : m_dropToPos;
    const int end = !moveToNext ? startIndex : m_dropToPos;

    if (start == end)
        return;

    for (int i(start + moveToNext); i != end - !moveToNext; ++i)
        createFakeAnimation(i, moveToNext);
    // last animation
    createFakeAnimation(end - !moveToNext, moveToNext, true);

    m_dragStartPos = indexRect(dropIndex).center();
}

void AppGridView::createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni)
{
    const QModelIndex index(indexAt(pos));

    QLabel *floatLabel = new QLabel(this);
    QPropertyAnimation *ani = new QPropertyAnimation(floatLabel, "pos", floatLabel);

    const auto ratio = devicePixelRatioF();
    const QSize rectSize = index.data(AppsListModel::ItemSizeHintRole).toSize();

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

    ani->setStartValue(indexRect(index).topLeft());
    ani->setEndValue(indexRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft());
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(300);

    connect(ani, &QPropertyAnimation::finished, floatLabel, &QLabel::deleteLater);
    if (isLastAni) {
        m_lastFakeAni = ani;
        connect(ani, &QPropertyAnimation::finished, this, &AppGridView::dropSwap);
        connect(ani, &QPropertyAnimation::valueChanged, m_dropThresholdTimer, &QTimer::stop);
    }

    ani->start(QPropertyAnimation::DeleteWhenStopped);
}

///
/// \brief AppListView::dropSwap swap current item and drag out item
///
void AppGridView::dropSwap()
{
    qDebug("AppGridView::dropSwap");
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->dropSwap(m_dropToPos);
    m_lastFakeAni = nullptr;

    setState(NoState);
}

const QRect AppGridView::indexRect(const QModelIndex &index) const
{
    return rectForIndex(index);
}
