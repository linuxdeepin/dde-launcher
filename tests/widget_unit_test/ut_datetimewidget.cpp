#define private public
#include "datetimewidget.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Datetimewidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new DatetimeWidget();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    DatetimeWidget *m_widget;
};

TEST_F(Tst_Datetimewidget, datetimeWidget_test)
{
    m_widget->getDateTextWidth();
    m_widget->updateTime();

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event);
}
