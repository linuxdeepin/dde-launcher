#include <gtest/gtest.h>

#include <QApplication>
#include <QtTest/QtTest>

#define private public
#include "pagecontrol.h"
#undef private
#include "applistdelegate.h"


class Tst_Pagecontrol : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new pageControl;
        widget->setPageCount(1);
        widget->UpdateIconSize(0.5, 0.5);
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    pageControl* widget = nullptr;
};

TEST_F(Tst_Pagecontrol, pageControl_test)
{
    widget->pageBtnClicked(true);
}

