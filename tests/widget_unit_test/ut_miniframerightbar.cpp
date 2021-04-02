#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#define private public
#include "miniframerightbar.h"
#undef private

class Tst_Miniframerightbar : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new MiniFrameRightBar();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    MiniFrameRightBar* widget = nullptr;
};

TEST_F(Tst_Miniframerightbar, miniFrameRightBar_test)
{
    widget->setCurrentCheck(true);
    widget->moveUp();
    widget->moveDown();
    widget->execCurrent();

    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event);

    widget->openDirectory(":/test_res/test.jpg");
}

