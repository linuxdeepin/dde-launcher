#include <gtest/gtest.h>

#include <QApplication>
#include <QWheelEvent>

#include "blurboxwidget.h"
#include "appslistmodel.h"


class Tst_Blurboxwidget : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new BlurBoxWidget(AppsListModel::Others, const_cast<char *>("Others"));
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    BlurBoxWidget* widget = nullptr;
};

TEST_F(Tst_Blurboxwidget, blurBoxWidget_test)
{
    widget->setFixedSize(QSize(10, 10));

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event);

    QMouseEvent event1(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget, &event1);
}

