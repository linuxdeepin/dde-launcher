#include <gtest/gtest.h>
#include <QtTest/QtTest>

#define private public
#include "categorytitlewidget.h"
#undef private



class Tst_Categorytitlewidget : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new CategoryTitleWidget("test");
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    CategoryTitleWidget* widget = nullptr;
};

TEST_F(Tst_Categorytitlewidget, categoryTitleWidget_test)
{

    widget->setTextVisible(true, true);
    widget->setTextVisible(true, false);
    widget->setTextVisible(false, false);
    widget->updatePosition(QPoint(0, 0), 10, 1);
    widget->updatePosition(QPoint(0, 0), 10, 2);
    widget->updatePosition(QPoint(0, 0), 10, 4);
    widget->updatePosition(QPoint(0, 0), 10, 5);

    QVERIFY(widget->textLabel()->text() == "test");
}

