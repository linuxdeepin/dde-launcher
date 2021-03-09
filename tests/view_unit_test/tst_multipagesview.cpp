#include <gtest/gtest.h>

#include <QApplication>
#include <QtTest/QtTest>

#define private public
#include "multipagesview.h"
#undef private
#include "applistdelegate.h"


class Tst_Multipagesview : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new MultiPagesView;
    }

    void TearDown() override
    {
        widget = nullptr;
        delete widget;
    }

public:
    MultiPagesView* widget = nullptr;
};

TEST_F(Tst_Multipagesview, multiPagesView_test)
{
    widget->updatePageCount(AppsListModel::Category);
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->pageView(0)->viewport(), &event);

    widget->updatePageCount(AppsListModel::Internet);
    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->pageView(0)->viewport(), &event1);

    QMouseEvent event2(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->pageView(0)->viewport(), &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->pageView(0)->viewport(), &event3);

    widget->setGradientVisible(false);

}

