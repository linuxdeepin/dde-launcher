#include <gtest/gtest.h>

#define private public
#include "miniframenavigation.h"
#undef private

class Tst_Miniframenavigation : public testing::Test
{
public:
    void SetUp() override
    {
        m_userButton = new UserButton();
        m_miniFrameNavigation = new MiniFrameNavigation(nullptr);
    }

    void TearDown() override
    {
        if (m_userButton) {
            delete m_userButton;
            m_userButton = nullptr;
        }

        if (m_miniFrameNavigation) {
            delete m_miniFrameNavigation;
            m_miniFrameNavigation = nullptr;
        }
    }

public:
    UserButton* m_userButton;
    MiniFrameNavigation* m_miniFrameNavigation;
};

TEST_F(Tst_Miniframenavigation, userButton_test)
{
    m_userButton->initUser();
    m_userButton->setUserPath("");
    m_userButton->setUserIconURL("");
}
