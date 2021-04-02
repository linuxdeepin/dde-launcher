#include <gtest/gtest.h>

#include <QApplication>
#include <QWheelEvent>
#include <QSignalSpy>
#include <QTest>

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
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    Avatar* widget = nullptr;
};

TEST_F(Tst_Avatar, avatar_test)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget, &event);

    QTest::qWait(10);

    QSignalSpy spy(widget, SIGNAL(clicked()));
    QCOMPARE(spy.count(), 1);

    QPaintEvent event1(QRect(0, 0, 32, 32));
    QApplication::sendEvent(widget, &event1);
}

