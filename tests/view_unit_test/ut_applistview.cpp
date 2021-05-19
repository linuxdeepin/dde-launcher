#include <gtest/gtest.h>

#include <QtTest/QtTest>
#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#define private public
#include "applistview.h"
#undef private
#include "appslistmodel.h"

class Tst_Applistview : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new AppListView;
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    AppListView* widget = nullptr;
};

TEST_F(Tst_Applistview, appListView_test)
{
    AppsListModel *pModel = new AppsListModel(AppsListModel::All);
    QModelIndex index;
    pModel->insertRow(0, index);
    widget->setModel(pModel);

    QWheelEvent event(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(widget->viewport(), &event);

    QMouseEvent event1(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event2);

    widget->hasAutoScroll();
    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event3);

    QMimeData mimeData;
    mimeData.setData("test", "test");

    QDragEnterEvent event4(QPoint(0, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event4);

    QDragMoveEvent event5(QPoint(0, 2), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget, &event5);

    QDragLeaveEvent event6;
    QApplication::sendEvent(widget, &event6);

    QDropEvent event7(QPointF(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event7);

    QEvent event8(QEvent::Enter);
    QApplication::sendEvent(widget, &event8);

    QEvent event9(QEvent::Leave);
    QApplication::sendEvent(widget, &event9);

    widget->handleScrollValueChanged();
    widget->handleScrollFinished();
    widget->prepareDropSwap();
    widget->dropSwap();
    widget->menuHide();
}

