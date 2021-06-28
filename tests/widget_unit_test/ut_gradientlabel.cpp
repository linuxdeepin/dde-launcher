#define private public
#include "gradientlabel.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Gradientlabel : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new GradientLabel();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    GradientLabel *m_widget;
};

TEST_F(Tst_Gradientlabel, gradientLabel_test)
{
    QVERIFY(GradientLabel::TopToBottom == m_widget->direction());
    QPixmap pix(10, 10);

    m_widget->setDirection(GradientLabel::TopToBottom);
    QVERIFY(GradientLabel::TopToBottom == m_widget->direction());
    m_widget->setPixmap(pix);
    m_widget->setDirection(GradientLabel::BottomToTop);
    QVERIFY(GradientLabel::BottomToTop == m_widget->direction());
    m_widget->setPixmap(pix);
    m_widget->setDirection(GradientLabel::LeftToRight);
    QVERIFY(GradientLabel::LeftToRight == m_widget->direction());
    m_widget->setPixmap(pix);
    m_widget->setDirection(GradientLabel::RightToLeft);
    QVERIFY(GradientLabel::RightToLeft == m_widget->direction());
    m_widget->setPixmap(pix);

    QPaintEvent event4(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event4);
}
