#include <gtest/gtest.h>
#include <QPixmap>

#include "boxframe.h"
#include "backgroundmanager.h"


class Tst_Boxframe : public testing::Test
{
public:
    void SetUp() override
    {
        m_frame = new BoxFrame(nullptr);
        m_frame->setBackground("");
        m_frame->setBlurBackground("");
    }

    void TearDown() override
    {
        m_frame = nullptr;
    }

public:
    BoxFrame* m_frame;
};

TEST_F(Tst_Boxframe, frame)
{

}

TEST_F(Tst_Boxframe, set_background_test)
{
    m_frame->setBackground(":/test_res/test.jpg");
}

TEST_F(Tst_Boxframe, invalid_dbackground_test)
{
    m_frame->setBackground("invalid");
}

TEST_F(Tst_Boxframe, set_blurbackground_test)
{
    m_frame->setBlurBackground(":/test_res/test.jpg");
}

TEST_F(Tst_Boxframe, invalid_blurbackground_test)
{
    m_frame->setBlurBackground("invalid");
}
