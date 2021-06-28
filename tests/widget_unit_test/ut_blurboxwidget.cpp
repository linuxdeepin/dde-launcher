#include "blurboxwidget.h"
#include "appslistmodel.h"

#include <QApplication>
#include <QWheelEvent>

#include <gtest/gtest.h>

class Tst_Blurboxwidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new BlurBoxWidget(AppsListModel::Others, const_cast<char *>("Others"));
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    BlurBoxWidget *m_widget;
};

TEST_F(Tst_Blurboxwidget, blurBoxWidget_test)
{
    m_widget->setFixedSize(QSize(10, 10));

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event);

    QMouseEvent event1(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event1);
}
