#define private public
#include "pagecontrol.h"
#undef private

#include <QApplication>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Pagecontrol : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new PageControl;
        m_widget->setPageCount(1);
        m_widget->updateIconSize(0.5, 0.5);
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    PageControl *m_widget;
};

TEST_F(Tst_Pagecontrol, pageControl_test)
{
    m_widget->pageBtnClicked(true);
}
