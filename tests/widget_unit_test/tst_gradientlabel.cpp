#include <gtest/gtest.h>
#include <QtTest/QtTest>

#define private public
#include "gradientlabel.h"
#undef private

class Tst_Gradientlabel : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new GradientLabel();
    }

    void TearDown() override
    {
        widget = nullptr;
        delete widget;
    }

public:
    GradientLabel* widget = nullptr;
};

TEST_F(Tst_Gradientlabel, gradientLabel_test)
{
    QVERIFY(GradientLabel::TopToBottom == widget->direction());
    QPixmap pix(10, 10);

    widget->setDirection(GradientLabel::TopToBottom);
    QVERIFY(GradientLabel::TopToBottom == widget->direction());
    widget->setPixmap(pix);
    widget->setDirection(GradientLabel::BottomToTop);
    QVERIFY(GradientLabel::BottomToTop == widget->direction());
    widget->setPixmap(pix);
    widget->setDirection(GradientLabel::LeftToRight);
    QVERIFY(GradientLabel::LeftToRight == widget->direction());
    widget->setPixmap(pix);
    widget->setDirection(GradientLabel::RightToLeft);
    QVERIFY(GradientLabel::RightToLeft == widget->direction());
    widget->setPixmap(pix);

    QPaintEvent event4(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event4);
}

