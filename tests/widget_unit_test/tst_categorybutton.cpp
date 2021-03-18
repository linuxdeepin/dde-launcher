#include <gtest/gtest.h>

#include <QApplication>
#include <QWheelEvent>

#define private public
#include "categorybutton.h"
#undef private
#include "appslistmodel.h"


class Tst_Categorybutton : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new CategoryButton(AppsListModel::Others, "Other");
    }

    void TearDown() override
    {
        widget = nullptr;
        delete widget;
    }

public:
    CategoryButton* widget = nullptr;
};

TEST_F(Tst_Categorybutton, bategoryButton_test)
{
    QEvent event(QEvent::Enter);
    QApplication::sendEvent(widget, &event);

    QEvent event1(QEvent::Leave);
    QApplication::sendEvent(widget, &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event3);

    widget->updateState(CategoryButton::Checked);
    QApplication::sendEvent(widget, &event3);

    QPaintEvent event4(QRect(10, 10, 10, 10));
    QApplication::sendEvent(widget, &event4);
    widget->updateState(CategoryButton::Hover);
    QApplication::sendEvent(widget, &event4);
    widget->updateState(CategoryButton::Checked);
    QApplication::sendEvent(widget, &event4);
    widget->updateState(CategoryButton::Press);
    QApplication::sendEvent(widget, &event4);

}

