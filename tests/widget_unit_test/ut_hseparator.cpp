#include <gtest/gtest.h>
#include <QtTest/QtTest>

#define private public
#include "hseparator.h"
#undef private

class Tst_Hseparator : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new HSeparator();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    HSeparator* widget = nullptr;
};

TEST_F(Tst_Hseparator, hSeparator_test)
{
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event);
}

