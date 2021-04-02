#include <gtest/gtest.h>

#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>

#define private public
#include "appgridview.h"
#undef private
#include "applistdelegate.h"


class Tst_Appgridview : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new AppGridView;
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    AppGridView* widget = nullptr;
};

TEST_F(Tst_Appgridview, appGridView_test)
{
    AppsListModel *pModel = new AppsListModel(AppsListModel::Internet);
    widget->setModel(pModel);

    widget->indexYOffset(widget->indexAt(0));
    QWidget *w = new QWidget();
    widget->setContainerBox(w);

    widget->updateItemHiDPIFixHook(widget->indexAt(0));

    widget->setDelegate(widget->getDelegate());

    QDropEvent event(QPointF(0, 0), Qt::CopyAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event);

    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event2);


    QMimeData *data = new QMimeData();
    data->setData("RequestDock","test");
    QDragEnterEvent event3(QPoint(0, 1), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event3);

    QDragMoveEvent event4(QPoint(0, 2), Qt::MoveAction, data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event4);

    widget->dragOut(-1);
    widget->dragIn(widget->indexAt(0));
    widget->flashDrag();

    QDragLeaveEvent event5;
    QApplication::sendEvent(widget->viewport(), &event5);

    QMouseEvent event6(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event6);

    QMouseEvent event7(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event7);

    widget->fitToContent();

}

