#include "appgridview.h"
#include "global_util/constants.h"
#include "global_util/calculate_util.h"
#include "model/appslistmodel.h"

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

QPointer<AppsManager> AppGridView::m_appManager = nullptr;
QPointer<CalculateUtil> AppGridView::m_calcUtil = nullptr;

AppGridView::AppGridView(QWidget *parent) :
    QListView(parent),
    m_dropThresholdTimer(new QTimer(this))
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

    setStyleSheet("background-color:transparent;");

    // update item spacing
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, [this] { setSpacing(m_calcUtil->appItemSpacing()); });

#ifndef DISABLE_DRAG_ANIMATION
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppGridView::prepareDropSwap, Qt::QueuedConnection);
#else
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppListView::dropSwap);
#endif
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

void AppGridView::dropEvent(QDropEvent *e)
{
    e->accept();

    m_enableDropInside = true;
}

void AppGridView::mousePressEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::RightButton) {
        QPoint rightClickPoint = mapToGlobal(e->pos());

        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());
        if (clickedIndex.isValid())
            emit popupMenuRequested(rightClickPoint, clickedIndex);
    }

    if (e->buttons() == Qt::LeftButton && !m_lastFakeAni)
        m_dragStartPos = e->pos();

    QListView::mousePressEvent(e);
}

void AppGridView::dragEnterEvent(QDragEnterEvent *e)
{
    const QModelIndex index = indexAt(e->pos());

    if (model()->canDropMimeData(e->mimeData(), e->dropAction(), index.row(), index.column(), QModelIndex()))
        return e->accept();
}

void AppGridView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_ASSERT(m_containerBox);

    if (m_lastFakeAni)
        return;

    const QModelIndex dropIndex = QListView::indexAt(e->pos());
    if (dropIndex.isValid())
        m_dropToPos = dropIndex.row();


    m_dropThresholdTimer->stop();

    const QPoint pos = mapToGlobal(e->pos());
    const QRect containerRect = m_containerBox->geometry().marginsRemoved(QMargins(0, DLauncher::APP_DRAG_SCROLL_THRESHOLD,
                                                                                   0, DLauncher::APP_DRAG_SCROLL_THRESHOLD));

    /*if (containerRect.contains(pos))
        return */m_dropThresholdTimer->start();

    if (pos.y() < containerRect.top())
        emit requestScrollUp();
    else if (pos.y() > containerRect.bottom())
        emit requestScrollDown();
    else
        emit requestScrollStop();
}

void AppGridView::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();

    m_dropThresholdTimer->stop();
    emit requestScrollStop();
}

void AppGridView::mouseMoveEvent(QMouseEvent *e)
{
    // disable qlistview default drag
    setState(NoState);

    QListView::mouseMoveEvent(e);

    if (e->buttons() != Qt::LeftButton)
        return;

    if (qAbs(e->pos().x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
        qAbs(e->pos().y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD)
        startDrag(QListView::indexAt(e->pos()));
}

void AppGridView::mouseReleaseEvent(QMouseEvent *e)
{
    // request main frame hide when click invalid area
    if (e->button() != Qt::LeftButton)
        return;

    const QModelIndex index = QListView::indexAt(e->pos());
    if (!index.isValid())
        emit clicked(index);

    QListView::mouseReleaseEvent(e);
}

void AppGridView::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}

void AppGridView::startDrag(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    const QModelIndex &dragIndex = index;
    const QString appKey = index.data(AppsListModel::AppKeyRole).toString();
    const QPixmap pixmap = index.data(AppsListModel::AppIconRole).value<QPixmap>();

    QDrag *drag = new QDrag(this);
    drag->setMimeData(model()->mimeData(QModelIndexList() << dragIndex));
    drag->setPixmap(pixmap.scaled(DLauncher::APP_DRAG_ICON_SIZE, DLauncher::APP_DRAG_ICON_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    drag->setHotSpot(QPoint(DLauncher::APP_DRAG_ICON_SIZE, DLauncher::APP_DRAG_ICON_SIZE) / 2);

    // request remove current item.
    if (listModel->category() == AppsListModel::All)
    {
        m_dropToPos = index.row();
        listModel->setDragingIndex(index);
    }

    drag->exec(Qt::MoveAction);

    // disable animation when finally dropped
    m_dropThresholdTimer->stop();

    // disable auto scroll
    emit requestScrollStop();

    if (listModel->category() != AppsListModel::All)
        return;

    if (!m_lastFakeAni)
    {
        if (m_enableDropInside)
            listModel->dropSwap(m_dropToPos);
        else
            listModel->dropSwap(indexAt(m_dragStartPos).row());

        listModel->clearDragingIndex();
    }
    else
    {
        connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDragingIndex);
    }

    m_enableDropInside = false;
}

bool AppGridView::eventFilter(QObject *o, QEvent *e)
{
    if (o == viewport() && e->type() == QEvent::Paint)
        fitToContent();

    return false;
}

///
/// \brief AppListView::fitToContent change view size to fit viewport content
///
void AppGridView::fitToContent()
{
    if (width() == contentsRect().width() && height() == contentsSize().height())
        return;

    const int h = contentsSize().height();

    setFixedHeight(h < 0 ? 0 : h);
    setFixedWidth(contentsRect().width());
}

void AppGridView::prepareDropSwap()
{
    if (m_lastFakeAni || m_dropThresholdTimer->isActive())
        return;
    const QModelIndex dropIndex = indexAt(m_dropToPos);
    if (!dropIndex.isValid())
        return;
    const QModelIndex dragStartIndex = indexAt(m_dragStartPos);
    if (dragStartIndex == dropIndex)
        return;

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->clearDragingIndex();
    listModel->setDragingIndex(dragStartIndex);
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
    QPropertyAnimation *ani = new QPropertyAnimation(floatLabel, "pos", this);

    const QSize rectSize = index.data(AppsListModel::ItemSizeHintRole).toSize();
    const QPixmap iconPix = index.data(AppsListModel::AppIconRole).value<QPixmap>();

    QStyleOptionViewItem item;
    item.rect = QRect(QPoint(0, 0), rectSize);
    item.features |= QStyleOptionViewItem::HasDisplay;

    QPixmap pixmap(rectSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    itemDelegate()->paint(&painter, item, index);

    floatLabel->setFixedSize(rectSize);
    floatLabel->setPixmap(pixmap);
//  lastLabel->setStyleSheet("background-color:red;");
    floatLabel->show();

    ani->setStartValue(indexRect(index).topLeft());
    ani->setEndValue(indexRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft());
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(300);

    connect(ani, &QPropertyAnimation::finished, floatLabel, &QLabel::deleteLater);
    if (isLastAni)
    {
        m_lastFakeAni = ani;
        connect(ani, &QPropertyAnimation::finished, this, &AppGridView::dropSwap, Qt::QueuedConnection);
//        connect(ani, &QPropertyAnimation::finished, [this] {dropSwap(); m_lastFakeAni = nullptr;});
        connect(ani, &QPropertyAnimation::valueChanged, m_dropThresholdTimer, &QTimer::stop);
    }

    ani->start(QPropertyAnimation::DeleteWhenStopped);
}

///
/// \brief AppListView::dropSwap swap current item and drag out item
///
void AppGridView::dropSwap()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->dropSwap(m_dropToPos);
    m_lastFakeAni = nullptr;
}

const QRect AppGridView::indexRect(const QModelIndex &index) const
{
    return rectForIndex(index);
}
