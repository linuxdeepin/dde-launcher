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
//    ModeToggleButton button;

//    DGuiApplicationHelper::ColorType defaultType = DGuiApplicationHelper::instance()->themeType();

//    button.setFocus();
//    button.m_hover = true;
//    QPaintEvent paintEvent1(QRect(10, 10, 10, 10));
//    button.paintEvent(&paintEvent1);

//    QEvent enterEvent(QEvent::Enter);
//    button.enterEvent(&enterEvent);

//    QEvent leaveEvent(QEvent::Leave);
//    button.leaveEvent(&leaveEvent);

//    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    button.mouseReleaseEvent(&releaseEvent);

//    for (int i = 0; i <= 2; i++) {
//        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(i));
//        button.resetStyle();
//        QTest::qWait(10);
//    }

//    DGuiApplicationHelper::instance()->setThemeType(defaultType);
}
