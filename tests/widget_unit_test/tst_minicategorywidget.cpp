#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#define private public
#include "minicategorywidget.h"
#undef private

class Tst_Minicategorywidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_miniCategoryWidget = new MiniCategoryWidget();
    }

    void TearDown() override
    {

    }

public:
    MiniCategoryWidget* m_miniCategoryWidget;
};

TEST_F(Tst_Minicategorywidget, miniCategoryWidget_test)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_miniCategoryWidget, &event);

//    QKeyEvent event1(QEvent::KeyPress, Qt::Key_Down,Qt::NoModifier);
//    QApplication::sendEvent(m_miniCategoryWidget, &event1);
//    QKeyEvent event2(QEvent::KeyPress, Qt::Key_Up,Qt::NoModifier);
//    QApplication::sendEvent(m_miniCategoryWidget, &event2);
//    QKeyEvent event3(QEvent::KeyPress, Qt::Key_Right,Qt::NoModifier);
//    QApplication::sendEvent(m_miniCategoryWidget, &event3);

    QEvent event4(QEvent::Enter);
    QApplication::sendEvent(m_miniCategoryWidget, &event4);

    QEvent event5(QEvent::FocusIn);
    QApplication::sendEvent(m_miniCategoryWidget, &event5);
    QEvent event6(QEvent::FocusOut);
    QApplication::sendEvent(m_miniCategoryWidget, &event6);

    m_miniCategoryWidget->onCategoryListChanged();
}

