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
{
public:
    void SetUp() override
    {
        m_fullscreen = new FullScreenFrame;
    }

    void TearDown() override
    {
        delete m_fullscreen;
    }

public:
    FullScreenFrame *m_fullscreen;
};

TEST_F(Tst_Multipagesview, multiPagesView_test)
{
    m_fullscreen->m_calcUtil->setDisplayMode(0);
    MultiPagesView *multipageView = m_fullscreen->m_multiPagesView;

    QPixmap pix;
    multipageView->updateGradient(pix, QPoint(0, 0), QPoint(10, 10));
    multipageView->setViewContentMargin();
    multipageView->updateGradient();
    multipageView->setGradientVisible(false);
}

TEST_F(Tst_Multipagesview, event_test)
{
    MultiPagesView *multipageView = m_fullscreen->m_multiPagesView;
    multipageView->updatePageCount(AppsListModel::Category);

    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(multipageView, &mousePressEvent);

    multipageView->updatePageCount(AppsListModel::Internet);
    QApplication::sendEvent(multipageView, &mousePressEvent);

    QMouseEvent mouseMoveEvent(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(multipageView, &mouseMoveEvent);

    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(multipageView, &mouseReleaseEvent);
}
