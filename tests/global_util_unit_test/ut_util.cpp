#include "util.h"

#include <QSize>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Util : public testing::Test
{
};

TEST_F(Tst_Util, loadSvg_test)
{
    QSize size = QSize(50, 50);
    loadSvg("", 20);

    // 文件名为空时，
    loadSvg("", size);

    // 文件名不为空时
    loadSvg("/usr/share/backgrounds/default_background.jpg", size);

    renderSVG("", size);
}

TEST_F(Tst_Util, settingsPtr_test)
{
    QVERIFY(ModuleSettingsPtr("", "", nullptr) == nullptr);
}

TEST_F(Tst_Util, calendar_test)
{
    createCalendarIcon("./calendar.svg");

    QPixmap pix;
    ItemInfo itemInfo;
    itemInfo.m_iconKey = QString("dde-calendar");
    getThemeIcon(pix, itemInfo, 24, false);
}

TEST_F(Tst_Util, findIcon_test)
{
    QVERIFY(!getIcon("1.svg").isNull());
}

TEST_F(Tst_Util, cache_test)
{
    ItemInfo itemInfo;
    itemInfo.m_iconKey = QString("dde-calendar");
    itemInfo.m_name = "dde-calendar";

    QVERIFY(!cacheKey(itemInfo, CacheType::ImageType).isEmpty());
}
