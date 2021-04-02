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
        std::shared_ptr<BoxFrame> frame(new BoxFrame);
        m_frame = frame;
        m_frame->setBackground("");
        m_frame->setBlurBackground("");

        std::shared_ptr<BackgroundManager> bgManager(new BackgroundManager);
        m_bgManager = bgManager;
    }

    void TearDown() override
    {
    }

public:
    std::shared_ptr<BoxFrame> m_frame;
    std::shared_ptr<BackgroundManager> m_bgManager;
};

TEST_F(Tst_Boxframe, frame)
{
    QMoveEvent event(QPoint(0, 0), QPoint(0, 1));
    QApplication::sendEvent(m_frame.get(), &event);

    QPaintEvent event1(QRect(QPoint(0, 0), QPoint(0, 1)));
    QApplication::sendEvent(m_frame.get(), &event1);
}

