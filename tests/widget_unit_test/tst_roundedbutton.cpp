#include <gtest/gtest.h>

#define private public
#include "roundedbutton.h"
#undef private


class Tst_Roundedbutton : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new RoundedButton();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
    }

public:
    RoundedButton* widget = nullptr;
};

TEST_F(Tst_Roundedbutton, roundedButton_test)
{
    widget->setText("test");
}

