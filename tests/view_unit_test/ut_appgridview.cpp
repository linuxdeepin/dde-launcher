#include "applistdelegate.h"

#define private public
#include "appgridview.h"
#include "fullscreenframe.h"
#undef private

#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Appgridview : public testing::Test
{};

TEST_F(Tst_Appgridview, appGridView_test)
{
    AppGridView view;
    view.indexYOffset(QModelIndex());
    QWidget w;
    view.setContainerBox(&w);

    view.updateItemHiDPIFixHook(QModelIndex());

    view.fitToContent();

    view.creatSrcPix(QModelIndex(), QString("dde-calendar"));
    view.creatSrcPix(QModelIndex(), QString("deepin-editor"));

    view.appIconRect(QModelIndex());

    QMouseEvent leftPressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    view.mousePressEvent(&leftPressEvent);

    view.m_mousePress = false;
    QMouseEvent rightPressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    view.mousePressEvent(&rightPressEvent);

    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    view.mouseMoveEvent(&moveEvent);

    QMouseEvent leftRelease(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    view.mouseReleaseEvent(&leftRelease);

    QMouseEvent rightRelease(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::LeftButton, Qt::NoModifier);
    view.mouseReleaseEvent(&rightRelease);

    QMimeData data;
    data.setText("test");

    QDragMoveEvent dragMoveEvent(QPoint(0, 10), Qt::DropAction::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    view.dragMoveEvent(&dragMoveEvent);

    QDropEvent dropEvent(QPointF(0, 20), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier, QEvent::Drop);
    view.dropEvent(&dropEvent);

    QDragLeaveEvent dragLeaveEvent;
    view.dragLeaveEvent(&dragLeaveEvent);

    view.m_calcUtil->setDisplayMode(GROUP_BY_CATEGORY);
    view.dragLeaveEvent(&dragLeaveEvent);

    view.dragOut(-1);
    view.dragIn(QModelIndex());
}

TEST_F(Tst_Appgridview, appItemDelegate_test)
{
    AppItemDelegate appItemDelegate;

    QFont font;
    const QFontMetrics fontMetrics(font);

    QRect rect(QPoint(10, 10), QSize(20, 30));
    appItemDelegate.itemBoundingRect(rect);

    rect.setSize(QSize(30, 20));
    appItemDelegate.itemBoundingRect(rect);

    rect.setSize(QSize(20, 20));
    appItemDelegate.itemBoundingRect(rect);
}

TEST_F(Tst_Appgridview, listModel_test)
{
    AppsListModel model(AppsListModel::Category, nullptr);

    model.setDragDropIndex(QModelIndex());

    model.setDraggingIndex(QModelIndex());

    model.dropInsert("deepin-editor", 0);

    model.dropSwap(0);

    model.clearDraggingIndex();

    model.layoutChanged(AppsListModel::Category);

    model.indexDragging(QModelIndex());

    ItemInfo info;
    model.itemDataChanged(info);
}

TEST_F(Tst_Appgridview, itemDelegate_test)
{
    AppGridView view;
    AppItemDelegate delegate(&view);

// painter无效
//    QRect paintRect(QPoint(10, 10), QSize(100, 100));
//    QPainter painter(&view);
//    QStyleOptionViewItem item;
//    delegate.paint(&painter, item, QModelIndex());

    QRect boundRect(QPoint(10, 10), QSize(20, 20));
    delegate.itemTextRect(boundRect, boundRect, true);
}

