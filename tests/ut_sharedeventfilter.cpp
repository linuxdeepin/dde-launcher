#include "sharedeventfilter.h"
#include "fullscreenframe.h"

#include <QEvent>
#include <QApplication>

#include "gtest/gtest.h"

class Tst_SharedEventFilter: public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    FullScreenFrame *m_frame;
    SharedEventFilter *m_eventFilter;
};

void Tst_SharedEventFilter::SetUp()
{
    m_frame = new FullScreenFrame;
    m_eventFilter = new SharedEventFilter(m_frame);
}

void Tst_SharedEventFilter::TearDown()
{
    delete m_eventFilter;
    delete m_frame;
}

TEST_F(Tst_SharedEventFilter, commonKeyEvent_test)
{
    QKeyEvent enterKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &enterKeyEvent);

    QKeyEvent returnKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &returnKeyEvent);

    QKeyEvent escKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &escKeyEvent);

    QKeyEvent spaceKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &spaceKeyEvent);

    QKeyEvent tabKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &tabKeyEvent);

    QKeyEvent upKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &upKeyEvent);

    QKeyEvent downKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &downKeyEvent);

    QKeyEvent leftKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &leftKeyEvent);

    QKeyEvent rightKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &rightKeyEvent);

    QKeyEvent backSpaceKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &backSpaceKeyEvent);
}

TEST_F(Tst_SharedEventFilter, letterKeyEvent_test)
{
    QKeyEvent pKeyEvent(QEvent::KeyPress, Qt::Key_P, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &pKeyEvent);

    QKeyEvent nKeyEvent(QEvent::KeyPress, Qt::Key_N, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &nKeyEvent);

    QKeyEvent fKeyEvent(QEvent::KeyPress, Qt::Key_F, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &fKeyEvent);

    QKeyEvent bKeyEvent(QEvent::KeyPress, Qt::Key_B, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &bKeyEvent);

    QKeyEvent zKeyEvent(QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &zKeyEvent);

    QKeyEvent f31KeyEvent(QEvent::KeyPress, Qt::Key_F31, Qt::ControlModifier);
    m_eventFilter->eventFilter(m_frame, &f31KeyEvent);

    QKeyEvent f32KeyEvent(QEvent::KeyPress, Qt::Key_F32, Qt::NoModifier);
    m_eventFilter->eventFilter(m_frame, &f32KeyEvent);
}
