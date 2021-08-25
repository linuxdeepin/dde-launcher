#include "applistdelegate.h"

#define private public
#include "appgridview.h"
#include "fullscreenframe.h"
#undef private

#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>

#include <gtest/gtest.h>

class Tst_Appgridview : public testing::Test
{};

TEST_F(Tst_Appgridview, appGridView_test)
{
    FullScreenFrame frame;
    frame.initUI();

    AppGridView *view =frame.m_multiPagesView->m_appGridViewList.at(frame.m_multiPagesView->currentPage());

    ASSERT_TRUE(view);

    view->indexYOffset(view->indexAt(0));
    QWidget *w = new QWidget(view);
    view->setContainerBox(w);

    view->updateItemHiDPIFixHook(view->indexAt(0));

    view->setDelegate(view->getDelegate());

    QDropEvent event(QPointF(0, 0), Qt::CopyAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(view->viewport(), &event);

    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(view->viewport(), &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(view->viewport(), &event2);

    QMimeData data;
    data.setData("RequestDock","test");
    QDragEnterEvent event3(QPoint(0, 1), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(view->viewport(), &event3);

    QDragMoveEvent event4(QPoint(0, 2), Qt::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(view->viewport(), &event4);

    view->dragOut(-1);
    view->dragIn(view->indexAt(0));

    QDragLeaveEvent event5;
    QApplication::sendEvent(view->viewport(), &event5);

    QMouseEvent event6(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(view->viewport(), &event6);

    QMouseEvent event7(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(view->viewport(), &event7);

    view->fitToContent();
}
