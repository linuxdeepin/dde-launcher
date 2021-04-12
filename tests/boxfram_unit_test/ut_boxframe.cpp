#include <gtest/gtest.h>
#include <QPixmap>
#include <QApplication>
#include <QMoveEvent>
#include <QPoint>

#include "boxframe.h"
#include "backgroundmanager.h"


class Tst_Boxframe : public testing::Test
{
public:
    void SetUp() override
    {
        m_frame = new BoxFrame;
        m_bgManager = new BackgroundManager(m_frame);
        m_frame->setBackground("");
        m_frame->setBlurBackground("");
    }

    void TearDown() override
    {
        if (m_frame) {
            delete  m_frame;
            m_frame = nullptr;
        }
    }

public:
    BoxFrame *m_frame;
    BackgroundManager *m_bgManager;
};


TEST_F(Tst_Boxframe, frame)
{
    QMoveEvent event(QPoint(0, 0), QPoint(0, 1));
    QApplication::sendEvent(m_frame, &event);

    QPaintEvent event1(QRect(QPoint(0, 0), QPoint(0, 1)));
    QApplication::sendEvent(m_frame, &event1);
}

