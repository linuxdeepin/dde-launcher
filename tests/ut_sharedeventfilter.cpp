#include "sharedeventfilter.h"
#include "fullscreenframe.h"

#include <QEvent>
#include <QApplication>

#include "gtest/gtest.h"

class Tst_SharedEventFilter: public testing::Test
{
};

TEST_F(Tst_SharedEventFilter, eventFilter_test)
{
//    FullScreenFrame frame;
//    SharedEventFilter filter(&frame);

//    QKeyEvent f1KeyEvent(QEvent::KeyPress, Qt::Key_F1,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &f1KeyEvent);

//    QKeyEvent enterKeyEvent(QEvent::KeyPress, Qt::Key_Enter,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &enterKeyEvent);

//    QKeyEvent returnKeyEvent(QEvent::KeyPress, Qt::Key_Return,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &returnKeyEvent);

//    QKeyEvent escKeyEvent(QEvent::KeyPress, Qt::Key_Escape,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &escKeyEvent);

//    QKeyEvent spaceKeyEvent(QEvent::KeyPress, Qt::Key_Space,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &spaceKeyEvent);

//    QKeyEvent tabKeyEvent(QEvent::KeyPress, Qt::Key_Tab,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &tabKeyEvent);

//    QKeyEvent backTabKeyEvent(QEvent::KeyPress, Qt::Key_Backtab,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &backTabKeyEvent);

//    QKeyEvent upKeyEvent(QEvent::KeyPress, Qt::Key_Up,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &upKeyEvent);

//    QKeyEvent downKeyEvent(QEvent::KeyPress, Qt::Key_Down,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &downKeyEvent);

//    QKeyEvent leftKeyEvent(QEvent::KeyPress, Qt::Key_Left,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &leftKeyEvent);

//    QKeyEvent rightKeyEvent(QEvent::KeyPress, Qt::Key_Right,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &rightKeyEvent);

//    QKeyEvent backSpaceKeyEvent(QEvent::KeyPress, Qt::Key_Backspace,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &backSpaceKeyEvent);

//    QKeyEvent pKeyEvent(QEvent::KeyPress, Qt::Key_P,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &pKeyEvent);

//    QKeyEvent nKeyEvent(QEvent::KeyPress, Qt::Key_N,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &nKeyEvent);

//    QKeyEvent fKeyEvent(QEvent::KeyPress, Qt::Key_F,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &fKeyEvent);

//    QKeyEvent bKeyEvent(QEvent::KeyPress, Qt::Key_B,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &bKeyEvent);

//    QKeyEvent zKeyEvent(QEvent::KeyPress, Qt::Key_Z,Qt::ControlModifier);
//    QApplication::sendEvent(&filter, &zKeyEvent);
}
