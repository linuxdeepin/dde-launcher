#define private public
#include "avatar.h"
#undef private

#include <QApplication>
#include <QWheelEvent>
#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Avatar : public testing::Test
{};

TEST_F(Tst_Avatar, avatar_test)
{
    Avatar avatar;
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    avatar.mouseReleaseEvent(&event);

    QSignalSpy spy(&avatar, SIGNAL(clicked()));
    QCOMPARE(spy.count(), 1);

    QPaintEvent event1(QRect(0, 0, 32, 32));
    avatar.paintEvent(&event1);

    avatar.setFilePath(QString());
}
