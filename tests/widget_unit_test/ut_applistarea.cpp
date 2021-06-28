#include "applistarea.h"

#include <QApplication>
#include <QWheelEvent>

#include <gtest/gtest.h>

class Tst_Applistarea : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new AppListArea;
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    AppListArea *m_widget;
};

TEST_F(Tst_Applistarea, appListArea_test)
{
    QWheelEvent event(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(m_widget, &event);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(m_widget, &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event3);
}
