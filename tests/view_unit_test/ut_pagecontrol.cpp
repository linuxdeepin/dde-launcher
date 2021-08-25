#define private public
#include "pagecontrol.h"
#undef private

#include <QApplication>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Pagecontrol : public testing::Test
{};

TEST_F(Tst_Pagecontrol, pageControl_test)
{
    PageControl control;
    control.setPageCount(1);
    control.updateIconSize(0.5, 0.5);
    control.pageBtnClicked(true);
}
