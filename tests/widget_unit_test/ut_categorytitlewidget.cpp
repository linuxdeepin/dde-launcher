#define private public
#include "categorytitlewidget.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Categorytitlewidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new CategoryTitleWidget("test");
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    CategoryTitleWidget *m_widget;
};

TEST_F(Tst_Categorytitlewidget, categoryTitleWidget_test)
{
    m_widget->setTextVisible(true, true);
    m_widget->setTextVisible(true, false);
    m_widget->setTextVisible(false, false);
    m_widget->updatePosition(QPoint(0, 0), 10, 1);
    m_widget->updatePosition(QPoint(0, 0), 10, 2);
    m_widget->updatePosition(QPoint(0, 0), 10, 4);
    m_widget->updatePosition(QPoint(0, 0), 10, 5);

    QVERIFY(m_widget->textLabel()->text() == "test");
}
