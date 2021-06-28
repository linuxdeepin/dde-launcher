#include "appslistmodel.h"
#define private public
#include "categorybutton.h"
#undef private

#include <QApplication>
#include <QWheelEvent>

#include <gtest/gtest.h>

class Tst_Categorybutton : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new CategoryButton(AppsListModel::Others);
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    CategoryButton *m_widget;
};

TEST_F(Tst_Categorybutton, bategoryButton_test)
{
    QEvent event(QEvent::Enter);
    QApplication::sendEvent(m_widget, &event);

    QEvent event1(QEvent::Leave);
    QApplication::sendEvent(m_widget, &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &event3);

    m_widget->updateState(CategoryButton::Checked);
    QApplication::sendEvent(m_widget, &event3);

    QPaintEvent event4(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event4);

    m_widget->updateState(CategoryButton::Hover);
    QApplication::sendEvent(m_widget, &event4);

    m_widget->updateState(CategoryButton::Checked);
    QApplication::sendEvent(m_widget, &event4);

    m_widget->updateState(CategoryButton::Press);
    QApplication::sendEvent(m_widget, &event4);
}
