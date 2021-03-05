#include <gtest/gtest.h>
#include <QPixmap>

#include "../src/boxframe/boxframe.h"
#include "../src/boxframe/backgroundmanager.h"


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
