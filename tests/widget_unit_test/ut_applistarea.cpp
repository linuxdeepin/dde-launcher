#include "applistarea.h"

#include <QApplication>
#include <QWheelEvent>

#include <gtest/gtest.h>

class Tst_Applistarea : public testing::Test
{};

TEST_F(Tst_Applistarea, appListArea_test)
{
    AppListArea area;
    QWheelEvent wheelEvent(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
    area.wheelEvent(&wheelEvent);

    QEvent enterEvent(QEvent::Enter);
    area.enterEvent(&enterEvent);

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    area.mousePressEvent(&pressEvent);

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    area.mouseReleaseEvent(&releaseEvent);

    QWheelEvent plusWheelEvent(QPointF(0, 0), 10, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(&area, &plusWheelEvent);

    QWheelEvent minusWheelEvent(QPointF(0, 0), -10, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(&area, &minusWheelEvent);
}
