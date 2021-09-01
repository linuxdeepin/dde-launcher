//#define private public
//#include "fullscreenframe.h"
//#undef private

//#include <QApplication>
//#include <QTest>

//#include <gtest/gtest.h>

///** Multipagesview的某些接口调用依赖于FullScreenFrame类中的数据
// * 因此使用FullScreenFrame来间接测试Multipagesview的接口
// * @brief The Tst_Multipagesview class
// */
//class Tst_Multipagesview : public testing::Test
//{};

//TEST_F(Tst_Multipagesview, multiPagesView_test)
//{
//   该类使用当前方法调用很容易出现段错误，本地无法复现，包括tty,但gerrit会出现，暂时屏蔽，放到最后处理。
//    CalculateUtil::instance()->setDisplayMode(0);

//    FullScreenFrame frame;

//    MultiPagesView view(AppsListModel::All, &frame);

//    view.setViewContentMargin();

//    view.updateGradient();

//    QPixmap pix;
//    view.updateGradient(pix, QPoint(0, 0), QPoint(10, 10));

//    view.updatePageCount(AppsListModel::Category);
//    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    view.mousePressEvent(&mousePressEvent);

//    view.updatePageCount(AppsListModel::Internet);
//    view.mousePressEvent(&mousePressEvent);

//    QMouseEvent mouseMoveEvent(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    view.mouseMoveEvent(&mouseMoveEvent);

//    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    view.mouseReleaseEvent(&mouseReleaseEvent);

//    view.setGradientVisible(false);
//}
