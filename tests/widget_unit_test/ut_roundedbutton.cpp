#define private public
#include "roundedbutton.h"
#undef private

#include <gtest/gtest.h>

class Tst_Roundedbutton : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new RoundedButton();
    }

    void TearDown() override
    {
        delete m_widget;
        m_widget = nullptr;
    }

public:
    RoundedButton *m_widget;
};

TEST_F(Tst_Roundedbutton, roundedButton_test)
{
    m_widget->setText("test");
}
