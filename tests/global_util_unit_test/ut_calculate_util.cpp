#define private public
#include "calculate_util.h"
#undef private

#include <QTest>

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

DGUI_USE_NAMESPACE

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

    QDate::currentDate().setDate(2000, 13, 0);
    QStringList iconList = CalculateUtil::instance()->calendarSelectIcon();
    QVERIFY2((iconList.size() == 4), "iconList size isn't equal to 4...");
    QVERIFY2(iconList.at(0).compare(":/icons/skin/icons/calendar_bg.svg"), "calendar background icon is abnormal...");

    int month_num = QDate::currentDate().month();
    switch(month_num)
    {
    case 1 ... 12:
        QVERIFY2(iconList.at(1).compare(QString(":/icons/skin/icons/calendar_month/month%1.svg").arg(month_num)), "month num is abnormal...");
        break;
    default:
        QVERIFY2(iconList.at(1).compare(QString(":/icons/skin/icons/calendar_month/month4.svg")), "default month num is abnormal...");
        break;
    }

    int day_num = QDate::currentDate().day();
    switch(day_num)
    {
    case 1 ... 31:
        QVERIFY2(iconList.at(2).compare(QString(":/icons/skin/icons/calendar_day/day%1.svg").arg(day_num)), "day num is abnormal...");
        break;
    default:
        QVERIFY2(iconList.at(2).compare(QString(":/icons/skin/icons/calendar_day/day23.svg")), "default day num is abnormal...");
        break;
    }

    int week_num = QDate::currentDate().dayOfWeek();
    switch(week_num)
    {
    case 1 ... 7:
        QVERIFY2(iconList.at(3).compare(QString(":/icons/skin/icons/calendar_week/week%1.svg").arg(week_num)), "week num is abnormal...");
        break;
    default:
        QVERIFY2(iconList.at(3).compare(QString(":/icons/skin/icons/calendar_week/week4.svg")), "defualt week num is abnormal...");
        break;
    }
}

TEST_F(Tst_calculate, size_test)
{
    int columnCount = CalculateUtil::instance()->appColumnCount();
    QVERIFY(columnCount == 7);

    CalculateUtil::instance()->setCurrentCategory(5);
    QVERIFY(CalculateUtil::instance()->currentCategory() != 4);

    CalculateUtil::instance()->setSearchWidgetSizeHint(QSize(40, 40));
    QVERIFY(!CalculateUtil::instance()->getSearchWidgetSizeHint().isNull());

    CalculateUtil::instance()->setNavigationWidgetSizeHint(QSize(40, 40));
    QVERIFY(!CalculateUtil::instance()->getNavigationWidgetSizeHint().isNull());

#if 0 // 没有gsettings环境，没有dbus环境。
    int defaultPos = CalculateUtil::instance()->m_dockInter->position();
    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();

    /*
     * 全屏自由模式: 0
     * 全屏分类模式: 1
    */
    for (int i = 0; i < 2; i++) {
        /*
         * 顶部：0, 右边:1, 底部: 2, 左边：3
        */
        for (int j = 0; j < 4; j++) {
            CalculateUtil::instance()->m_dockInter->setPosition(j);

        /*
         * 全屏自由模式：0
         * 全屏分类模式: 1         * 全屏搜索模式：２
        */
            for (int k = 0; k < 3; k++) {
//                CalculateUtil::instance()->setDisplayMode(k);
                CalculateUtil::instance()->calculateIconSize(i);
                QTest::qWait(500);
            }
        }
    }

    QTest::qWait(500);

    //　恢复默认位置．
    CalculateUtil::instance()->m_dockInter->setPosition(defaultPos);
    DGuiApplicationHelper::instance()->setThemeType(defaultType);
#endif
}

TEST_F(Tst_calculate, appIconSize_test)
{
    CalculateUtil::instance()->setFullScreen(true);
    CalculateUtil::instance()->appIconSize();
    CalculateUtil::instance()->appIconSize(true, 1, 32);

    CalculateUtil::instance()->setFullScreen(false);
    CalculateUtil::instance()->appIconSize();
    CalculateUtil::instance()->appIconSize(true, 1, 32);

    QVERIFY(CalculateUtil::instance()->increaseIconSize());
    QVERIFY(CalculateUtil::instance()->decreaseIconSize());
}