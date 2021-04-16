#include <gtest/gtest.h>

#include <QPixmap>
#include <QApplication>
#include <QMoveEvent>
#include <QPoint>
#include <QTest>
#include <QSignalSpy>

// 欺骗编译器的操作, 旨在单元测试中方便使用该类中的私有成员函数或者对象
#define private public

#include "boxframe.h"
#include "backgroundmanager.h"

class Tst_Boxframe : public testing::Test
{
public:
    void SetUp() override
    {
        m_frame = new BoxFrame;
        m_bgManager = new BackgroundManager(m_frame);
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

TEST_F(Tst_Boxframe, checkBackground_test)
{
    // 确保函数正常执行
    m_frame->setBackground(m_frame->m_defaultBg);
    QVERIFY(m_frame->m_lastUrl.compare(m_frame->m_defaultBg));

    QSignalSpy spy(m_frame, SIGNAL(backgroundImageChanged(const QPixmap & img)));
    m_frame->setBlurBackground(m_frame->m_defaultBg);
    QVERIFY(m_frame->m_lastBlurUrl.compare(m_frame->m_defaultBg));

    QCOMPARE(spy.count(), 1);
}

TEST_F(Tst_Boxframe, frame)
{
    QMoveEvent event(QPoint(0, 0), QPoint(0, 1));
    QApplication::sendEvent(m_frame, &event);

    QPaintEvent event1(QRect(QPoint(0, 0), QPoint(0, 1)));
    QApplication::sendEvent(m_frame, &event1);
}

