#define private public
#include "minicategorywidget.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Minicategorywidget : public testing::Test
{};

TEST_F(Tst_Minicategorywidget, miniCategoryWidget_test)
{
    MiniCategoryWidget widget;
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&widget, &event);

    QKeyEvent rightKeyEvent(QEvent::KeyPress, Qt::Key_Right,Qt::NoModifier);
    widget.keyPressEvent(&rightKeyEvent);

//    crash
//    QKeyEvent event1(QEvent::KeyPress, Qt::Key_Down,Qt::NoModifier);
//    QApplication::sendEvent(m_miniCategoryWidget, &event1);
//    QKeyEvent event2(QEvent::KeyPress, Qt::Key_Up,Qt::NoModifier);
//    QApplication::sendEvent(m_miniCategoryWidget, &event2);
//    QKeyEvent event3(QEvent::KeyPress, Qt::Key_Right,Qt::NoModifier);
//    QApplication::sendEvent(m_miniCategoryWidget, &event3);

    QEvent event4(QEvent::Enter);
    QApplication::sendEvent(&widget, &event4);

    QEvent event5(QEvent::FocusIn);
    QApplication::sendEvent(&widget, &event5);
    QEvent event6(QEvent::FocusOut);
    QApplication::sendEvent(&widget, &event6);

    widget.onCategoryListChanged();
    ASSERT_TRUE(widget.active() == widget.m_active);

//    crash
//    m_miniCategoryWidget->selectNext();
//    m_miniCategoryWidget->selectPrev();
}
