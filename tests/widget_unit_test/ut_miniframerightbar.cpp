#define private public
#include "miniframerightbar.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Miniframerightbar : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new MiniFrameRightBar();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    MiniFrameRightBar *m_widget;
};

TEST_F(Tst_Miniframerightbar, miniFrameRightBar_test)
{
    m_widget->setCurrentCheck(true);
    m_widget->moveUp();
    m_widget->moveDown();
    m_widget->execCurrent();

    QPaintEvent event(QRect(10, 10, 10, 10));
    QApplication::sendEvent(m_widget, &event);

    m_widget->openDirectory(":/test_res/test.jpg");
}
