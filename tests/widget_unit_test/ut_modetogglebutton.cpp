#define private public
#include "modetogglebutton.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Modetogglebutton : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new ModeToggleButton();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    ModeToggleButton *m_widget;
};

TEST_F(Tst_Modetogglebutton, modeToggleButton_test)
{
    m_widget->setFocusPolicy(Qt::ClickFocus);
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(m_widget, &event1);

    QEvent event2(QEvent::Leave);
    QApplication::sendEvent(m_widget, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event3);
}
