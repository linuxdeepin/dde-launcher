#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#define private public
#include "miniframeswitchbtn.h"
#undef private
#include "windowedframe.h"

class Tst_Miniframeswitchbtn : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new MiniFrameSwitchBtn();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    MiniFrameSwitchBtn* widget = nullptr;
};

TEST_F(Tst_Miniframeswitchbtn, miniFrameSwitchBtn_test)
{
    widget->updateStatus(WindowedFrame::All);
    widget->updateStatus(WindowedFrame::Category);
    widget->click();
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(widget, &event1);

    QEvent event2(QEvent::Leave);
    QApplication::sendEvent(widget, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event3);
}

