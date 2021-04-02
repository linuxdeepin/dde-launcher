#include <gtest/gtest.h>
#include <QtTest/QtTest>

#define private public
#include "datetimewidget.h"
#undef private



class Tst_Datetimewidget : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new DatetimeWidget();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    DatetimeWidget* widget = nullptr;
};

TEST_F(Tst_Datetimewidget, datetimeWidget_test)
{
    widget->getDateTextWidth();
    widget->updateTime();

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event);
}

