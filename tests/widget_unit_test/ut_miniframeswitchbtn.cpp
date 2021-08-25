#include "windowedframe.h"

#define private public
#include "miniframeswitchbtn.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

class Tst_Miniframeswitchbtn : public testing::Test
{};

TEST_F(Tst_Miniframeswitchbtn, miniFrameSwitchBtn_test)
{
    MiniFrameSwitchBtn button;

    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();

    button.updateStatus(WindowedFrame::All);
    button.updateStatus(WindowedFrame::Category);
    button.click();
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &event);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(&button, &event1);

    QEvent event2(QEvent::Leave);
    QApplication::sendEvent(&button, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&button, &event3);

    button.setFocus();
    QPaintEvent paintEvent(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &paintEvent);

    for (int i = 0; i <= 2; i++) {
        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(i));
        button.updateIcon();
    }

    DGuiApplicationHelper::instance()->setThemeType(defaultType);
}
