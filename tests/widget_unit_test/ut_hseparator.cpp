#define private public
#include "hseparator.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Hseparator : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new HSeparator();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    HSeparator *m_widget;
};

TEST_F(Tst_Hseparator, hSeparator_test)
{
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event);
}
