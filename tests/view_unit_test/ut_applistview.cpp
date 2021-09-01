//#define private public
//#include "appslistmodel.h"
//#include "applistview.h"
//#include "applistdelegate.h"
//#include "windowedframe.h"
//#undef private

//#include <QTest>
//#include <QApplication>
//#include <QDropEvent>
//#include <QDragEnterEvent>
//#include <QMimeData>
//#include <DGuiApplicationHelper>

//#include <gtest/gtest.h>

///**使用小窗口类WindowedFrame间接测试AppListView的
// * 接口, 因为AppListView的视图代码是在小窗口类初始
// * 化列表中创建的, 规避该类appListView_test中偶现的
// * 崩溃问题
// * @brief The Tst_Applistview class
// */
//class Tst_Applistview : public testing::Test
//{};

//TEST_F(Tst_Applistview, appDelegate_test)
//{
//    WindowedFrame frame;
//    AppListView *appListView = frame.m_appsView;

//    //　light type
//    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(1));
//    appListView->setModel(frame.m_appsModel);
//    appListView->setItemDelegate(new AppListDelegate(&frame));
//    AppListDelegate * delegate = static_cast<AppListDelegate *>(appListView->itemDelegate());
//    if (delegate)
//        delegate->setActived(true);

//    //　dark type
//    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(2));
//    delegate = static_cast<AppListDelegate *>(appListView->itemDelegate());
//    appListView->setItemDelegate(new AppListDelegate(frame.m_appsModel));
//    if (delegate)
//        delegate->setActived(true);

//    frame.show();
//    QTest::qWait(500);
//    frame.hide();
//    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(1));
//}

//TEST_F(Tst_Applistview, event_test)
//{
//    AppListView widget;
//    AppListDelegate delegate(&widget);
//    widget.setItemDelegate(&delegate);

//    AppsListModel model(AppsListModel::All);
//    QModelIndex index;
//    model.insertRow(0, index);
//    widget.setModel(&model);

//    QTest::qWait(1000);

//    QWheelEvent wheelEvent(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
//    widget.wheelEvent(&wheelEvent);

//    QMouseEvent mouseMoveEvent(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    widget.mouseMoveEvent(&mouseMoveEvent);

//    QMouseEvent mousePress(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    widget.mousePressEvent(&mousePress);

//    widget.m_updateEnableSelectionByMouseTimer->start();
//    widget.mousePressEvent(&mousePress);

//    widget.hasAutoScroll();
//    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    widget.mouseReleaseEvent(&releaseEvent);

//    QMimeData mimeData;
//    mimeData.setData("test", "test");

//    QDragEnterEvent dragEnterEvent(QPoint(0, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
//    widget.dragEnterEvent(&dragEnterEvent);

//    QDragMoveEvent dragMoveEvent(QPoint(0, 2), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
//    widget.dragMoveEvent(&dragMoveEvent);

//    QDragLeaveEvent dragLeaveEvent;
//    widget.dragLeaveEvent(&dragLeaveEvent);

//    QDropEvent dropEvent(QPointF(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
//    widget.dropEvent(&dropEvent);

//    QEvent enterEvent(QEvent::Enter);
//    widget.enterEvent(&enterEvent);

//    QEvent leaveEvent(QEvent::Leave);
//    widget.leaveEvent(&leaveEvent);

//    widget.handleScrollValueChanged();
//    widget.handleScrollFinished();
//    widget.prepareDropSwap();
//    widget.dropSwap();
//    widget.menuHide();
//}

//TEST_F(Tst_Applistview, appListView_test)
//{
//    WindowedFrame frame;

//    AppListView *widget = frame.m_appsView;
//    AppListDelegate delegate(&frame);
//    widget->setItemDelegate(&delegate);

//    AppsListModel model(AppsListModel::All);
//    QModelIndex index;
//    model.insertRow(0, index);
//    widget->setModel(&model);

//    widget->handleScrollValueChanged();
//    widget->handleScrollFinished();
//    widget->prepareDropSwap();
//    widget->dropSwap();
//    widget->menuHide();
//    frame.hide();
//}

//TEST_F(Tst_Applistview, appListDelegate_test)
//{
//    AppListDelegate appListDelegate;
//    appListDelegate.setActived(false);

//    ASSERT_FALSE(appListDelegate.m_actived);

//    QPixmap pix;
//    appListDelegate.dropShadow(pix, 5, QColor(Qt::red), QPoint(10, 10));

//    pix.load("/usr/share/backgrounds/default_background.jpg");
//    appListDelegate.dropShadow(pix, 5, QColor(Qt::red), QPoint(10, 10));
//}

//TEST_F(Tst_Applistview, viewApi_test)
//{
//    AppListView view;
//    QModelIndex index = QModelIndex();
//    view.startDrag(index);
//}
