#include "gtest/gtest.h"

#include<QEvent>
#include <QApplication>

#include "sharedeventfilter.h"

class Tst_SharedEventFilter: public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    SharedEventFilter* m_eventFilter;
};

void Tst_SharedEventFilter::SetUp()
{
    m_eventFilter = new SharedEventFilter;
}

void Tst_SharedEventFilter::TearDown()
{
    if (m_eventFilter) {
        delete m_eventFilter;
        m_eventFilter = nullptr;
    }
}

TEST_F(Tst_SharedEventFilter, eventFilter_test)
{
    QKeyEvent event1(QEvent::KeyPress, Qt::Key_F1,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event1);

    QKeyEvent event2(QEvent::KeyPress, Qt::Key_Enter,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event2);

    QKeyEvent event3(QEvent::KeyPress, Qt::Key_Return,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event3);

    QKeyEvent event4(QEvent::KeyPress, Qt::Key_Escape,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event4);

    QKeyEvent event5(QEvent::KeyPress, Qt::Key_Space,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event5);

    QKeyEvent event6(QEvent::KeyPress, Qt::Key_Tab,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event6);

    QKeyEvent event7(QEvent::KeyPress, Qt::Key_Backtab,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event7);

    QKeyEvent event8(QEvent::KeyPress, Qt::Key_Up,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event8);

    QKeyEvent event9(QEvent::KeyPress, Qt::Key_Down,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event9);

    QKeyEvent event10(QEvent::KeyPress, Qt::Key_Left,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event10);

    QKeyEvent event11(QEvent::KeyPress, Qt::Key_Right,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event11);

    QKeyEvent event12(QEvent::KeyPress, Qt::Key_Backspace,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event12);

    QKeyEvent event13(QEvent::KeyPress, Qt::Key_P,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event13);

    QKeyEvent event14(QEvent::KeyPress, Qt::Key_N,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event14);

    QKeyEvent event15(QEvent::KeyPress, Qt::Key_F,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event15);

    QKeyEvent event16(QEvent::KeyPress, Qt::Key_B,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event16);

    QKeyEvent event17(QEvent::KeyPress, Qt::Key_Z,Qt::ControlModifier);
    QApplication::sendEvent(m_eventFilter, &event17);
}
