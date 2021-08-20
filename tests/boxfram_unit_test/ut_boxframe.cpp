// 欺骗编译器的操作, 旨在单元测试中方便使用该类中的私有成员函数或者对象
#define private public
#include "boxframe.h"
#include "backgroundmanager.h"
#undef private

#include <QPixmap>
#include <QApplication>
#include <QMoveEvent>
#include <QPoint>
#include <QTest>
#include <QSignalSpy>

#include <gtest/gtest.h>

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
    /* 模拟一下四种模式下修改屏幕的背景和模糊处理过的背景
    #define CUSTOM_MODE     0
    #define MERGE_MODE      1
    #define EXTEND_MODE     2
    #define SINGLE_MODE     3
    */
    for (int i = 0; i < 4; i++) {
        m_frame->m_bgManager->m_displayMode = i;
        m_frame->setBackground(m_frame->m_defaultBg);
        QVERIFY(m_frame->m_lastUrl.compare(m_frame->m_defaultBg));

        // url和上一次相同时
        m_frame->setBackground(m_frame->m_defaultBg);

        QSignalSpy spy(m_frame, SIGNAL(backgroundImageChanged(const QPixmap & img)));
        m_frame->setBlurBackground(m_frame->m_defaultBg);
        QVERIFY(m_frame->m_lastBlurUrl.compare(m_frame->m_defaultBg));

        // url和上一次相同时
        m_frame->setBlurBackground(m_frame->m_defaultBg);

        // 清空缓存
        m_frame->removeCache();

        // 清空上一次的背景
        m_frame->m_lastUrl.clear();
        m_frame->m_lastBlurUrl.clear();

        QCOMPARE(spy.count(), 1);
    }
}

TEST_F(Tst_Boxframe, moveEvent_test)
{
    QMoveEvent event(QPoint(0, 0), QPoint(0, 1));
    QApplication::sendEvent(m_frame, &event);
    QTest::qWait(100);
}

TEST_F(Tst_Boxframe, paintEvent_test)
{
    QPaintEvent event1(QRect(QPoint(0, 0), QPoint(0, 1)));
    QApplication::sendEvent(m_frame, &event1);
    QTest::qWait(100);
}
