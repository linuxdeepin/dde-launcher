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
#include "constants.h"
#include "calculate_util.h"
#include "util.h"
#include "appslistmodel.h"
#include "fullscreenframe.h"

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

bool AppGridView::m_longPressed = false;
Gesture *AppGridView::m_gestureInter = nullptr;

/**
 * @brief AppGridView::AppGridView
 * 全屏模式下 应用网格视图，主要处理全屏图标的拖拽事件及分组切换的动画效果
 * @param parent
 */
AppGridView::AppGridView(QWidget *parent)
    : QListView(parent)
    , m_mousePress(false)
    , m_dropThresholdTimer(new QTimer(this))
{
    m_pDelegate = nullptr;

    if (!m_gestureInter) {
        m_gestureInter = new Gesture("com.deepin.daemon.Gesture"
                                     , "/com/deepin/daemon/Gesture"
                                     , QDBusConnection::systemBus()
                                     , nullptr);
    }

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
    setDropIndicatorShown(false);// 解决拖拽释放前有小黑点出现的问题
    setMovement(QListView::Free);
    setFlow(QListView::LeftToRight);
    setLayoutMode(QListView::Batched);
    setResizeMode(QListView::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setViewportMargins(0, 0, 0, 0);
    viewport()->setAutoFillBackground(false);

    // update item spacing
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &AppGridView::setAppItemSpacing);

#ifndef DISABLE_DRAG_ANIMATION
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppGridView::prepareDropSwap);
#else
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppGridView::dropSwap);
#endif

    // 根据后端延迟触屏信号控制是否可进行图标拖动，收到延迟触屏信号可拖动，没有收到延迟触屏信号、点击松开就不可拖动
    connect(m_gestureInter, &Gesture::TouchSinglePressTimeout, m_gestureInter, [] {
        m_longPressed = true;
    }, Qt::UniqueConnection);
    connect(m_gestureInter, &Gesture::TouchUpOrCancel, m_gestureInter, [] {
        m_longPressed = false;
    }, Qt::UniqueConnection);
}

const QModelIndex AppGridView::indexAt(const int index) const
{
    return model()->index(index, 0, QModelIndex());
}

/**
 * @brief AppGridView::indexYOffset
 * @param index item index
 * @return item Y offset of current view
 */
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

void AppGridView::setDelegate(DragPageDelegate *pDelegate)
{
    m_pDelegate = pDelegate;
}

DragPageDelegate *AppGridView::getDelegate()
{
    return m_pDelegate;
}

void AppGridView::dropEvent(QDropEvent *e)
{
    e->accept();

    if (m_dropThresholdTimer->isActive())
        m_dropThresholdTimer->stop();

    // 解决快速拖动应用，没有动画效果的问题，当拖拽小于200毫秒时，禁止拖动
    if (m_dragLastTime.elapsed() < 200)
        return;

    m_enableDropInside = true;

    // set the correct hover item.
    emit entered(QListView::indexAt(e->pos()));
}

void AppGridView::mousePressEvent(QMouseEvent *e)
{
    // 识别出鼠标左键按下还是触摸按下
    if (e->source() != Qt::MouseEventSynthesizedByQt && e->button() == Qt::LeftButton)
        m_mousePress = true;

    if (e->button() == Qt::RightButton) {
        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());
        if (clickedIndex.isValid() && !m_moveGridView) {
            QPoint rightClickPoint = QCursor::pos();
            // 触控屏右键
            if (e->source() == Qt::MouseEventSynthesizedByQt) {
                AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
                QPoint indexPoint = mapToGlobal(indexRect(clickedIndex).center());

                if (listModel->category() == 0) {
                    rightClickPoint.setX(indexPoint.x() + indexRect(clickedIndex).width() - 58 * m_calcUtil->getScreenScaleX());
                } else if (listModel->category() == 2) {
                    rightClickPoint.setX(indexPoint.x() + indexRect(clickedIndex).width()  - 90 * m_calcUtil->getScreenScaleX());
                } else {
                    rightClickPoint.setX(indexPoint.x() + indexRect(clickedIndex).width() - 3 * m_calcUtil->getScreenScaleX());
                }
            }

            // 鼠标左键点击和触摸按住应用的情况同时存在则不触发右键菜单，因为右键菜单exec()会导致全屏模式下的触摸失效,
            // 但如果鼠标左键点击的时刻刚好等于触摸按住应用然后触发该信号但右键菜单还没有出来的时刻，触摸依旧会失效
            if ((!m_mousePress || !m_longPressed))
                emit popupMenuRequested(rightClickPoint, clickedIndex);
        }
    }

    if (e->buttons() == Qt::LeftButton && !m_lastFakeAni) {
        m_dragStartPos = e->pos();

        // 记录动画的终点位置
        setDropAndLastPos(appIconRect(indexAt(e->pos())).topLeft());
    }

    if (m_pDelegate)
        m_pDelegate->mousePress(e);

    QListView::mousePressEvent(e);
}

void AppGridView::dragEnterEvent(QDragEnterEvent *e)
{
    m_dragLastTime.start();

    const QModelIndex index = indexAt(e->pos());

    if (model()->canDropMimeData(e->mimeData(), e->dropAction(), index.row(), index.column(), QModelIndex())) {
        emit entered(QModelIndex());
        return e->accept();
    }
}

void AppGridView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_UNUSED(e);
    m_dropThresholdTimer->stop();

    if (m_lastFakeAni)
        return;

    const QPoint pos = e->pos();
    const QRect containerRect = this->rect();

    const QModelIndex dropIndex = QListView::indexAt(pos);
    if (dropIndex.isValid()) {
        m_dropToPos = dropIndex.row();
    } else if (containerRect.contains(pos)) {
        AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
        if (listModel) {
            int lastRow = listModel->rowCount(QModelIndex()) - 1;

            QModelIndex lastIndex = listModel->index(lastRow);

            if (lastIndex.isValid()) {
                QPoint lastPos = indexRect(lastIndex).center();
                if (pos.x() > lastPos.x() && pos.y() > lastPos.y())
                    m_dropToPos = lastIndex.row();
            }
        }
    }

    // 分页切换后隐藏label过渡效果
    emit requestScrollStop();

    // 释放前执行app交换动画
    if (m_enableAnimation)
        m_dropThresholdTimer->start();
}

void AppGridView::dragOut(int pos)
{
    m_enableAnimation = false;
    m_dropToPos = pos;

    prepareDropSwap();
    dropSwap();
}

void AppGridView::dragIn(const QModelIndex &index)
{
    m_enableAnimation = false;
    m_dragStartPos = indexRect(index).center();
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->setDraggingIndex(index);
}

void AppGridView::setDropAndLastPos(const QPoint& itemPos)
{
    m_dropPoint = itemPos;
}

void AppGridView::flashDrag()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    int dragDropRow = listModel->dragDropIndex().row();

    startDrag(indexAt(dragDropRow));
}

/**
 * @brief AppGridView::dragLeaveEvent 离开listview时触发分页
 * @param e 拖动离开事件指针对象
 */
void AppGridView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_ASSERT(m_containerBox);

    m_dropThresholdTimer->stop();

    QRect containerRect;
    // QCursor::pos()获取的是绝对坐标（在多屏的情况下，(0,0)原点是从最左侧的屏幕开始计算的)，
    // 因此当启动器ui在最右侧的屏幕时容易出现向左滑动触发翻页，结果响应的是向右翻页，因此统一使用
    // QScrollArea来计算相对该控件的全局坐标
    QPoint pos = m_containerBox->mapFromGlobal(QCursor::pos());

    int padding = m_calcUtil->getScreenSize().width() * DLauncher::SIDES_SPACE_SCALE;
    QMargins margin(padding, DLauncher::APP_DRAG_SCROLL_THRESHOLD,
                    padding, DLauncher::APP_DRAG_SCROLL_THRESHOLD);

    containerRect = this->contentsRect().marginsRemoved(margin);

    const QModelIndex dropStart = QListView::indexAt(m_dragStartPos);

    if (pos.x() > containerRect.right())
        emit requestScrollRight(dropStart);
    else if (pos.x() < containerRect.left())
        emit requestScrollLeft(dropStart);

    QListView ::dragLeaveEvent(e);
}

void AppGridView::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();

    setState(NoState);

    const QModelIndex &idx = indexAt(e->pos());

    // 鼠标在app上划过时实现选中效果
    if (idx.isValid())
        emit entered(idx);

    if (e->buttons() != Qt::LeftButton)
        return;

    // 当点击的位置在图标上就不把消息往下传(listview 滚动效果)
    if (!idx.isValid() && m_pDelegate && !m_longPressed)
        m_pDelegate->mouseMove(e);

    // 如果是触屏事件转换而来并且没有收到后端的延时触屏消息，不进行拖拽
    if (e->source() == Qt::MouseEventSynthesizedByQt && !m_longPressed)
        return;

    if (qAbs(e->x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
            qAbs(e->y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD) {
        m_moveGridView = true;

        // 开始拖拽后,导致fullscreenframe只收到mousePress事件,收不到mouseRelease事件,需要处理一下异常
        if (idx.isValid())
            emit requestMouseRelease();

        return startDrag(QListView::indexAt(m_dragStartPos));
    }
}

void AppGridView::mouseReleaseEvent(QMouseEvent *e)
{
    m_mousePress = false;

    // request main frame hide when click invalid area
    if (e->button() != Qt::LeftButton)
        return;

    m_moveGridView = false;
    if (m_pDelegate)
        m_pDelegate->mouseRelease(e);

    int diff_x = qAbs(e->pos().x() - m_dragStartPos.x());
    int diff_y = qAbs(e->pos().y() - m_dragStartPos.y());

    // 小范围位置变化，当作没有变化，针对触摸屏
    if (diff_x < DLauncher::TOUCH_DIFF_THRESH && diff_y < DLauncher::TOUCH_DIFF_THRESH)
        QListView::mouseReleaseEvent(e);
}

QPixmap AppGridView::creatSrcPix(const QModelIndex &index, const QString &appKey)
{
    QPixmap srcPix;

    if (appKey == "dde-calendar") {
        const  auto  s = m_calcUtil->appIconSize();
        const double  iconZoom =  s.width() / 64.0;
        QStringList calIconList = m_calcUtil->calendarSelectIcon();

        auto calendar = new QWidget(this) ;
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
        auto monthPix = loadSvg(calIconList.at(1), QSize(20, 10) * iconZoom);
        month->setPixmap(monthPix.scaled(monthPix.width(), monthPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        month->setFixedHeight(monthPix.height());
        month->setAlignment(Qt::AlignCenter);
        month->setFixedWidth(s.width() - 5 * iconZoom);
        layout->addWidget(month, Qt::AlignVCenter);

        auto day = new QLabel();
        auto dayPix = loadSvg(calIconList.at(2), QSize(28, 26) * iconZoom);
        day->setPixmap(dayPix.scaled(dayPix.width(), dayPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        day->setAlignment(Qt::AlignCenter);
        day->setFixedHeight(day->pixmap()->height());
        day->raise();
        layout->addWidget(day, Qt::AlignVCenter);

        auto week = new QLabel();
        auto weekPix = loadSvg(calIconList.at(3), QSize(14, 6) * iconZoom);
        week->setPixmap(weekPix.scaled(weekPix.width(), weekPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        week->setFixedHeight(week->pixmap()->height());
        week->setAlignment(Qt::AlignCenter);
        week->setFixedWidth(s.width() + 5 * iconZoom);
        layout->addWidget(week, Qt::AlignVCenter);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 10 * iconZoom, 0, 10 * iconZoom);
        calendar->setLayout(layout);
        srcPix = calendar->grab(calendar->rect());
        calendar->deleteLater();
    } else {
        srcPix = index.data(AppsListModel::AppDragIconRole).value<QPixmap>();
    }

    return srcPix;
}

/**
 * @brief AppGridView::appIconRect 计算应用图标的矩形位置，该接口中的浮点数据
 * 是从视图代理paint接口中摘过来的matlab模拟的数据
 * @param index 应用图标所在的模型索引
 * @return 应用图标对应的矩形大小
 */
QRect AppGridView::appIconRect(const QModelIndex &index)
{
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).toSize();

    const static double x1 = 0.26418192;
    const static double x2 = -0.38890932;

    const int w = indexRect(index).width();
    const int h = indexRect(index).height();
    const int sub = qAbs((w - h) / 2);

    QRect ibr;
    if (w == h)
         ibr = indexRect(index);
    else if (w > h)
          ibr = indexRect(index) - QMargins(sub, 0, sub, 0);
    else
          ibr = indexRect(index) - QMargins(0, 0, 0, sub * 2);

    const double result = x1 * ibr.width() + x2 * iconSize.width();
    int margin = result > 0 ? result * 0.71 : 1;
    QRect br = ibr.marginsRemoved(QMargins(margin, 1, margin, margin * 2));

    int iconTopMargin = 6;

    const int iconLeftMargins = (br.width() - iconSize.width()) / 2;

    return QRect(br.topLeft() + QPoint(iconLeftMargins, iconTopMargin - 2), iconSize);
}

/**
 * @brief AppGridView::startDrag 处理listview中app的拖动操作
 * @param index 被拖动app的对应的模型索引
 */
void AppGridView::startDrag(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    m_moveGridView = false;
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    const QModelIndex &dragIndex = index;
    const qreal ratio = qApp->devicePixelRatio();
    QString appKey = index.data(AppsListModel::AppKeyRole).value<QString>();

    QPixmap srcPix = creatSrcPix(index, appKey);

    srcPix = srcPix.scaled(m_calcUtil->appIconSize() * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    srcPix.setDevicePixelRatio(ratio);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(model()->mimeData(QModelIndexList() << dragIndex));
    drag->setPixmap(srcPix);
    drag->setHotSpot(srcPix.rect().center() / ratio);

    // 给被拖动item添加鼠标释放的动画效果,避开在龙芯设备上从隐藏到显示的突兀问题
    // 获取全屏指针对象
    FullScreenFrame *fullscreenFrame = nullptr;
    if (m_calcUtil->displayMode() == GROUP_BY_CATEGORY) {
        fullscreenFrame = qobject_cast<FullScreenFrame*>(
                    this->parentWidget()->parentWidget()->parentWidget()->parentWidget()
                    ->parentWidget()->parentWidget()->parentWidget()->parentWidget());

        // 解决全屏分类模式，搜索模式下拖动应用父对象为空导致出现空页面问题的情况
        if (!fullscreenFrame) {
            fullscreenFrame = qobject_cast<FullScreenFrame*>(
                        this->parentWidget()->parentWidget()->parentWidget()->parentWidget()
                        ->parentWidget()->parentWidget()->parentWidget());
        }
    } else {
        fullscreenFrame = qobject_cast<FullScreenFrame*>(
                    this->parentWidget()->parentWidget()->parentWidget()->parentWidget()
                    ->parentWidget()->parentWidget()->parentWidget());
    }

    Q_ASSERT(fullscreenFrame);

    QLabel *pixLabel = new QLabel(fullscreenFrame);
    pixLabel->setPixmap(srcPix);
    pixLabel->setFixedSize(srcPix.size());
    pixLabel->move(srcPix.rect().center() / ratio);
    pixLabel->hide();

    QPropertyAnimation *posAni = new QPropertyAnimation(pixLabel, "pos", pixLabel);
    connect(posAni, &QPropertyAnimation::finished, [&, listModel, pixLabel]() mutable {
        delete pixLabel;
        pixLabel = nullptr;

        if (!m_lastFakeAni) {
            if (m_enableDropInside)
                listModel->dropSwap(m_dropToPos);// 无动画时,在listview内释放鼠标
            else
                // 搜索模式下，不需要做删除被拖拽的item，插入移动到新位置的item的操作，否则会导致被拖拽item的位置改变
                if (listModel->category() != AppsListModel::AppCategory::Search){
                    listModel->dropSwap(indexAt(m_dragStartPos).row());// 无动画时,listview之外释放鼠标
                }

            listModel->clearDraggingIndex();
        } else {
            connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDraggingIndex);// 动画执行结束后清理拖拽数据
        }

        setDropAndLastPos(QPoint(0, 0));
        m_enableDropInside = false;
        m_lastFakeAni = nullptr;
    });

    m_dropToPos = index.row();
    listModel->setDraggingIndex(index);

    int old_page = m_containerBox->property("curPage").toInt();

    setState(DraggingState);
    drag->exec(Qt::MoveAction);

    // 拖拽操作完成后暂停app移动动画
    m_dropThresholdTimer->stop();

    // 未触发分页则直接返回,触发分页则执行分页后操作
    emit dragEnd();

    int cur_page = m_containerBox->property("curPage").toInt();

    // 当拖动应用出现触发分页的情况，关闭上一个动画， 直接处理当前当前页的动画
    if (cur_page != old_page) {
        posAni->stop();

        delete pixLabel;
        pixLabel = nullptr;
        return;
    }

    QRect rectIcon(QPoint(), pixLabel->size());

    pixLabel->move((QCursor::pos() - rectIcon.center()));
    pixLabel->show();

    posAni->setEasingCurve(QEasingCurve::Linear);
    posAni->setDuration(DLauncher::APP_DRAG_MININUM_TIME);
    posAni->setStartValue((QCursor::pos() - rectIcon.center()));

    // 添加因为添加了水平方向的偏移，导致拖动item释放后终点位置不对导致的动画异常问题
    int padding = m_calcUtil->getScreenSize().width() * DLauncher::SIDES_SPACE_SCALE * 5 / 6;

    // 使用mapToGlobal出现终点坐标转换异常问题，故而使用坐标偏移
    if (m_calcUtil->displayMode() == GROUP_BY_CATEGORY)
        posAni->setEndValue(mapToGlobal(m_dropPoint) + QPoint(60, 0));
    else
        posAni->setEndValue(m_containerBox->mapToGlobal(QPoint()) + m_dropPoint + QPoint(padding, 0));

    posAni->start(QPropertyAnimation::DeleteWhenStopped);
}

/**
 * @brief AppGridView::fitToContent
 * change view size to fit viewport content
 */
void AppGridView::fitToContent()
{
    const QSize size { contentsRect().width(), contentsSize().height() };
    if (size == rect().size())
        return;

    setFixedSize(size);
}

/**
 * @brief AppGridView::prepareDropSwap 创建移动item的动画
 */
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

    for (int i = (start + moveToNext); i != (end - !moveToNext); ++i)
        createFakeAnimation(i, moveToNext);

    createFakeAnimation(end - !moveToNext, moveToNext, true);

    // item最后回归的位置
    setDropAndLastPos(appIconRect(dropIndex).topLeft());

    m_dragStartPos = indexRect(dropIndex).center();
}

/**
 * @brief AppGridView::createFakeAnimation 创建列表中item移动的动画效果
 * @param pos 需要移动的item当前所在的行数
 * @param moveNext item是否移动的标识
 * @param isLastAni 是最后的那个item移动的动画标识
 */
void AppGridView::createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni)
{
    // listview n行1列,肉眼所及的都是app自动换行后的效果
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

    int topMargin = m_calcUtil->appMarginTop();
    int leftMargin = m_calcUtil->appMarginLeft();
    ani->setStartValue(indexRect(index).topLeft() - QPoint(0, -topMargin) + QPoint(leftMargin, 0));
    ani->setEndValue(indexRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft() - QPoint(0, -topMargin) + QPoint(leftMargin, 0));

    // InOutQuad 描述起点矩形到终点矩形的速度曲线
    ani->setEasingCurve(QEasingCurve::Linear);
    ani->setDuration(DLauncher::APP_DRAG_MININUM_TIME);

    connect(ani, &QPropertyAnimation::finished, floatLabel, &QLabel::deleteLater);
    if (isLastAni) {
        m_lastFakeAni = ani;
        connect(ani, &QPropertyAnimation::finished, this, &AppGridView::dropSwap);
        connect(ani, &QPropertyAnimation::valueChanged, m_dropThresholdTimer, &QTimer::stop);
    }

    ani->start(QPropertyAnimation::DeleteWhenStopped);
}

/**
 * @brief AppGridView::dropSwap
 * 删除拖动前item所在位置的item，插入拖拽的item到新位置
 */
void AppGridView::dropSwap()
{
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

void AppGridView::setAppItemSpacing()
{
    setSpacing(m_calcUtil->appItemSpacing());
    setViewportMargins(m_calcUtil->appMarginLeft(), m_calcUtil->appMarginTop(), m_calcUtil->appMarginLeft(), 0);
}
