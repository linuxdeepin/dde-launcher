#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#define private public
#include "miniframebutton.h"
#undef private

class Tst_Miniframebutton : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new MiniFrameButton("OK");
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    MiniFrameButton* widget = nullptr;
};

TEST_F(Tst_Miniframebutton, miniFrameButton_test)
{

    QEvent event(QEvent::Enter);
    QApplication::sendEvent(widget, &event);

    QEvent event1(QEvent::Leave);
    QApplication::sendEvent(widget, &event1);

    QEvent event2(QEvent::ApplicationFontChange);
    QApplication::sendEvent(widget, &event2);

    widget->setChecked(true);
    QPaintEvent event3(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event3);
}

