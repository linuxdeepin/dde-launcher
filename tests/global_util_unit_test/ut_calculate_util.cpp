#include <gtest/gtest.h>

#include "calculate_util.h"

#include <QTest>


class Tst_calculate : public testing::Test
{
};

TEST_F(Tst_calculate, calendarSelectIcon_test)
{
    CalculateUtil::instance()->setFullScreen(false);
    CalculateUtil::instance()->appIconSize();
    CalculateUtil::instance()->getAppBoxSize();
    CalculateUtil::instance()->getScreenScaleX();
    CalculateUtil::instance()->getScreenSize();

    QStringList iconList = CalculateUtil::instance()->calendarSelectIcon();
    QVERIFY2(iconList.size() == 4, "iconList size isn't equal to 4...");
    QVERIFY(iconList[0] == ":/icons/skin/icons/calendar_bg.svg");
    QVERIFY(iconList[1] == ":/icons/skin/icons/calendar_month/month3.svg");
    QVERIFY(iconList[2] == ":/icons/skin/icons/calendar_day/day4.svg");
    QVERIFY(iconList[3] == ":/icons/skin/icons/calendar_week/week4.svg");
}
