#include <gtest/gtest.h>

#include <QApplication>
#include <QTest>

#define private public
#include "fullscreenframe.h"
#undef private

/** Multipagesview的某些接口调用依赖于FullScreenFrame类中的数据
 * 因此使用FullScreenFrame来间接测试Multipagesview的接口
 * @brief The Tst_Multipagesview class
 */
class Tst_Multipagesview : public testing::Test
{
public:
    void SetUp() override
    {
        fullscreen = new FullScreenFrame;
    }

    void TearDown() override
    {
        delete fullscreen;
        fullscreen = nullptr;
    }

public:
    FullScreenFrame *fullscreen;
};

TEST_F(Tst_Multipagesview, multiPagesView_test)
{
    fullscreen->initUI();
    fullscreen->m_multiPagesView->updatePageCount(AppsListModel::Category);
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(fullscreen->m_multiPagesView, &event);

    fullscreen->m_multiPagesView->updatePageCount(AppsListModel::Internet);
    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(fullscreen->m_multiPagesView, &event1);

    QMouseEvent event2(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(fullscreen->m_multiPagesView, &event2);

    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(fullscreen->m_multiPagesView, &event3);

    fullscreen->m_multiPagesView->setGradientVisible(false);

}

