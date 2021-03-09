#include <gtest/gtest.h>

#include <QApplication>
#include <QWheelEvent>

#include "avatar.h"


class Tst_Avatar : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new Avatar;
    }

    void TearDown() override
    {
        widget = nullptr;
        delete widget;
    }

public:
    Avatar* widget = nullptr;
};

TEST_F(Tst_Avatar, avatar_test)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event);

    QPaintEvent event1(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event1);
}

