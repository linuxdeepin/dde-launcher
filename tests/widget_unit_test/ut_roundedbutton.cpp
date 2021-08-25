#define private public
#include "roundedbutton.h"
#undef private

#include <gtest/gtest.h>

class Tst_Roundedbutton : public testing::Test
{};

TEST_F(Tst_Roundedbutton, roundedButton_test)
{
    RoundedButton button;
    button.setText("test");
}
