#define private public
#include "miniframebutton.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Miniframebutton : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new MiniFrameButton("OK");
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    MiniFrameButton *m_widget;
};

TEST_F(Tst_Miniframebutton, miniFrameButton_test)
{
    QEvent event(QEvent::Enter);
    QApplication::sendEvent(m_widget, &event);

    QEvent event1(QEvent::Leave);
    QApplication::sendEvent(m_widget, &event1);

    QEvent event2(QEvent::ApplicationFontChange);
    QApplication::sendEvent(m_widget, &event2);

    m_widget->setChecked(true);
    QPaintEvent event3(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event3);
}
