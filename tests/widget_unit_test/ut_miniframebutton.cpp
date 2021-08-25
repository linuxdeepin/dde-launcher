#define private public
#include "miniframebutton.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <DGuiApplicationHelper>
#include <QRect>

#include <gtest/gtest.h>

DGUI_USE_NAMESPACE

class Tst_Miniframebutton : public testing::Test
{};

TEST_F(Tst_Miniframebutton, miniFrameButton_test)
{
    MiniFrameButton button("OK");

    QEvent event(QEvent::Enter);
    QApplication::sendEvent(&button, &event);

    QEvent event1(QEvent::Leave);
    QApplication::sendEvent(&button, &event1);

    QEvent event2(QEvent::ApplicationFontChange);
    QApplication::sendEvent(&button, &event2);

    button.setChecked(true);
    QPaintEvent event3(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &event3);
}
