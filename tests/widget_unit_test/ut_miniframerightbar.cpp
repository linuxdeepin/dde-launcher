#define private public
#include "miniframerightbar.h"
#undef private

#include <DGuiApplicationHelper>

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTest>

#include <gtest/gtest.h>

DGUI_USE_NAMESPACE

class Tst_Miniframerightbar : public testing::Test
{};

TEST_F(Tst_Miniframerightbar, miniFrameRightBar_test)
{
    MiniFrameRightBar bar;

    bar.setCurrentCheck(true);
    bar.moveUp();
    bar.moveDown();
    bar.execCurrent();
    bar.openStandardDirectory(QStandardPaths::DesktopLocation);

    bar.openDirectory(":/test_res/test.jpg");
}

TEST_F(Tst_Miniframerightbar, event_test)
{
    MiniFrameRightBar bar;

    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();
    for (int i = 1; i <= 2; i++) {
        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(i));
        QPaintEvent paintEvent(QRect(10, 10, 10, 10));
        QApplication::sendEvent(&bar, &paintEvent);
        QTest::qWait(10);
    }

    DGuiApplicationHelper::instance()->setThemeType(defaultType);

    QShowEvent showEvent;
    QApplication::sendEvent(&bar, &showEvent);
}
