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
{};

TEST_F(Tst_Boxframe, checkBackground_test)
{
    BoxFrame frame;
    /* 模拟一下四种模式下修改屏幕的背景和模糊处理过的背景
    #define CUSTOM_MODE     0
    #define MERGE_MODE      1
    #define EXTEND_MODE     2
    #define SINGLE_MODE     3
    */
    for (int i = 0; i < 4; i++) {
        frame.m_bgManager->m_displayMode = i;
        frame.setBackground(frame.m_defaultBg);
        QVERIFY(frame.m_lastUrl.compare(frame.m_defaultBg));

        // url和上一次相同时
        frame.setBackground(frame.m_defaultBg);

        QSignalSpy spy(&frame, SIGNAL(backgroundImageChanged(const QPixmap & img)));
        frame.setBlurBackground(frame.m_defaultBg);
        QVERIFY(frame.m_lastBlurUrl.compare(frame.m_defaultBg));

        // url和上一次相同时
        frame.setBlurBackground(frame.m_defaultBg);

        // 清空缓存
        frame.removeCache();

        // 清空上一次的背景
        frame.m_lastUrl.clear();
        frame.m_lastBlurUrl.clear();

        QCOMPARE(spy.count(), 1);
    }
}

TEST_F(Tst_Boxframe, moveEvent_test)
{
    BoxFrame frame;

    QMoveEvent event(QPoint(0, 0), QPoint(0, 1));
    QApplication::sendEvent(&frame, &event);
    QTest::qWait(100);
}

TEST_F(Tst_Boxframe, paintEvent_test)
{
    BoxFrame frame;

    QPaintEvent event1(QRect(QPoint(0, 0), QPoint(0, 1)));
    QApplication::sendEvent(&frame, &event1);
    QTest::qWait(100);
}
