#include <gtest/gtest.h>
#include <QtTest/QtTest>

#define private public
#include "maskqwidget.h"
#undef private

class Tst_Maskqwidget : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new MaskQWidget();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    MaskQWidget* widget = nullptr;
};

TEST_F(Tst_Maskqwidget, hSeparator_test)
{
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event);
}

