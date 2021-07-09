#include "windowedframe.h"

#define private public
#include "miniframeswitchbtn.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Miniframeswitchbtn : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new MiniFrameSwitchBtn();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    MiniFrameSwitchBtn *m_widget;
};

TEST_F(Tst_Miniframeswitchbtn, miniFrameSwitchBtn_test)
{
    m_widget->updateStatus(WindowedFrame::All);
    m_widget->updateStatus(WindowedFrame::Category);
    m_widget->click();
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(m_widget, &event1);

    QEvent event2(QEvent::Leave);
    QApplication::sendEvent(m_widget, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event3);
}
