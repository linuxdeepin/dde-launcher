#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#define private public
#include "modetogglebutton.h"
#undef private


class Tst_Modetogglebutton : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new ModeToggleButton();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    ModeToggleButton* widget = nullptr;
};

TEST_F(Tst_Modetogglebutton, modeToggleButton_test)
{
    widget->setFocusPolicy(Qt::ClickFocus);
    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event);

    QEvent event1(QEvent::Enter);
    QApplication::sendEvent(widget, &event1);

    QEvent event2(QEvent::Leave);
    QApplication::sendEvent(widget, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event3);
}

