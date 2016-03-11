#include "applistview.h"
#include "global_util/constants.h"
#include "model/appslistmodel.h"

#include <QDebug>
#include <QWheelEvent>
#include <QTimer>
#include <QDrag>
#include <QMimeData>

AppListView::AppListView(QWidget *parent) :
    QListView(parent)
{
    viewport()->installEventFilter(this);
    viewport()->setAcceptDrops(true);

    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setMovement(QListView::Free);
    setFlow(QListView::LeftToRight);
    setLayoutMode(QListView::Batched);
    setResizeMode(QListView::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setWrapping(true);
}

QSize AppListView::sizeHint() const
{
    return QListView::sizeHint();
}

void AppListView::dropEvent(QDropEvent *e)
{
//    for (QModelIndex &index : selectedIndexes())
//        model()->removeRow(index.row());
    qDebug() << "drop: " << e->dropAction() << e->mimeData();
}

void AppListView::mousePressEvent(QMouseEvent *e)
{
    m_dragStartPos = e->pos();
    qDebug() << "mousePressing!";
    if (e->button() == Qt::RightButton) {
        qDebug() << "AppListView mousePressEvent:" << e->pos();
    }
    QListView::mousePressEvent(e);
}

void AppListView::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
}

void AppListView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_UNUSED(e);
}

void AppListView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_UNUSED(e);
}

void AppListView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() != Qt::LeftButton)
        return;

    if (qAbs(e->pos().x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
        qAbs(e->pos().y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD)
        startDrag(Qt::MoveAction);
}

void AppListView::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);
}

void AppListView::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}

void AppListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QListView::currentChanged(current, previous);
}

bool AppListView::eventFilter(QObject *o, QEvent *e)
{
    if (o == viewport() && e->type() == QEvent::Paint)
        fitToContent();

    return false;
}

void AppListView::fitToContent()
{
    if (width() == contentsRect().width() && height() == contentsSize().height())
        return;

    setFixedWidth(contentsRect().width());
    setFixedHeight(contentsSize().height());
}
