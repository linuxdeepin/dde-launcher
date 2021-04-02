#include <gtest/gtest.h>
#include <QPixmap>
#include <QtTest/QtTest>

#include "../src/global_util/calculate_util.h"


class Tst_calculate : public testing::Test
{
public:
    void SetUp() override
    {
        m_calculate = CalculateUtil::instance();
        m_calculate->setFullScreen(false);
        m_calculate->appIconSize();
        m_calculate->getAppBoxSize();
        m_calculate->getScreenScaleX();
        m_calculate->getScreenSize();
    }

    void TearDown() override
    {
        if (m_calculate) {
            delete m_calculate;
            m_calculate = nullptr;
        }
    }

public:
    CalculateUtil* m_calculate;
};

TEST_F(Tst_calculate, calendarSelectIcon_test)
{
    QStringList iconList = m_calculate->calendarSelectIcon();
    QVERIFY(iconList.size() == 4);
    QVERIFY(iconList[0] == ":/icons/skin/icons/calendar_bg.svg");
    QVERIFY(iconList[1] == ":/icons/skin/icons/calendar_month/month3.svg");
    QVERIFY(iconList[2] == ":/icons/skin/icons/calendar_day/day4.svg");
    QVERIFY(iconList[3] == ":/icons/skin/icons/calendar_week/week4.svg");
}
