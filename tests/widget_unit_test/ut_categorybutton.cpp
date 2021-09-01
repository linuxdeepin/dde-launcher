#include "appslistmodel.h"
#define private public
#include "categorybutton.h"
#undef private

#include <QApplication>
#include <QWheelEvent>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Categorybutton : public testing::Test
{};

TEST_F(Tst_Categorybutton, mouseEvent_test)
{
    CategoryButton button(AppsListModel::Others);

//    QEnterEvent enterEvent(QPoint(0, 0), QPoint(10, 0), QPoint(10, 10));
//    button.enterEvent(&enterEvent);
//    button.updateState(CategoryButton::Checked);

//    QEvent leaveEvent(QEvent::Leave);
//    button.leaveEvent(&leaveEvent);
//    QTest::qWait(10);

//    button.updateState(CategoryButton::Normal);
//    button.leaveEvent(&leaveEvent);
//    QTest::qWait(10);

//    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    button.mousePressEvent(&mousePressEvent);
//    QTest::qWait(10);

//    button.updateState(CategoryButton::Checked);
//    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    button.mouseReleaseEvent(&mouseReleaseEvent);
//    QTest::qWait(10);

//    button.updateState(CategoryButton::Normal);
//    button.mouseReleaseEvent(&mouseReleaseEvent);
//    QTest::qWait(10);

//    button.updateState(CategoryButton::Checked);
//    button.mouseReleaseEvent(&mouseReleaseEvent);
//    QTest::qWait(10);
}

TEST_F(Tst_Categorybutton, paintEvent_test)
{
    CategoryButton button(AppsListModel::Others);

    QPaintEvent paintEvent(QRect(10, 10, 10, 10));
    button.paintEvent(&paintEvent);

    button.updateState(CategoryButton::Hover);
    button.paintEvent(&paintEvent);

    button.updateState(CategoryButton::Checked);
    button.paintEvent(&paintEvent);

    button.updateState(CategoryButton::Press);
    button.paintEvent(&paintEvent);
}
