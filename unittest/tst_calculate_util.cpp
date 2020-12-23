#include <gtest/gtest.h>
#include <QPixmap>

#include "../src/global_util/calculate_util.h"


class Tst_calculate : public testing::Test
{
public:
    void SetUp() override
    {
        m_calculate = CalculateUtil::instance();
        m_calculate->setFullScreen(false);
        m_calculate->appIconSize();
        m_calculate->getAppBoxSize();
        m_calculate->getScreenScaleX();
        m_calculate->getScreenSize();
    }

    void TearDown() override
    {

    }

public:
    CalculateUtil* m_calculate;
};

TEST_F(Tst_calculate, calculate)
{

}
