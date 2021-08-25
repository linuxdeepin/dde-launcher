#define private public
#include "modetogglebutton.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTest>

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

class Tst_Modetogglebutton : public testing::Test
{};

TEST_F(Tst_Modetogglebutton, modeToggleButton_test)
{
    ModeToggleButton button;

    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();

    button.setFocus();
    button.m_hover = true;
    QPaintEvent paintEvent1(QRect(10, 10, 10, 10));
    QApplication::sendEvent(&button, &paintEvent1);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(&button, &event1);

    QEvent event2(QEvent::Leave);
    QApplication::sendEvent(&button, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(&button, &event3);

    for (int i = 0; i <= 2; i++) {
        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(i));
        button.resetStyle();
        QTest::qWait(10);
    }

    DGuiApplicationHelper::instance()->setThemeType(defaultType);
}
