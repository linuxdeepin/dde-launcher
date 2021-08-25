#define private public
#include "fullscreenframe.h"
#undef private

#include <QApplication>
#include <QTest>

#include <gtest/gtest.h>

/** Multipagesview的某些接口调用依赖于FullScreenFrame类中的数据
 * 因此使用FullScreenFrame来间接测试Multipagesview的接口
 * @brief The Tst_Multipagesview class
 */
class Tst_Multipagesview : public testing::Test
{};

TEST_F(Tst_Multipagesview, multiPagesView_test)
{
    FullScreenFrame frame;
    MultiPagesView *multipageView = frame.m_multiPagesView;
    ASSERT_TRUE(multipageView);

    multipageView->updatePageCount(AppsListModel::Category);
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(multipageView, &event);

    multipageView->updatePageCount(AppsListModel::Internet);
    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(multipageView, &event1);

    QMouseEvent event2(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(multipageView, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(multipageView, &event3);

    multipageView->setGradientVisible(false);
}
