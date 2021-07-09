#define private public
#include "maskqwidget.h"
#undef private

#include <QtTest/QtTest>

#include <gtest/gtest.h>

class Tst_Maskqwidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new MaskQWidget();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    MaskQWidget *m_widget;
};

TEST_F(Tst_Maskqwidget, hSeparator_test)
{
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event);
}
