#include "appslistmodel.h"
#include "applistdelegate.h"

#define private public
#include "applistview.h"
#include "windowedframe.h"
#undef private

#include <QTest>
#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

/**使用小窗口类WindowedFrame间接测试AppListView的
 * 接口, 因为AppListView的视图代码是在小窗口类初始
 * 化列表中创建的, 规避该类appListView_test中偶现的
 * 崩溃问题
 * @brief The Tst_Applistview class
 */
class Tst_Applistview : public testing::Test
{};

TEST_F(Tst_Applistview, appDelegate_test)
{
    WindowedFrame frame;
    AppListView *appListView = frame.m_appsView;

    //　light type
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(1));
    appListView->setModel(frame.m_appsModel);
    appListView->setItemDelegate(new AppListDelegate(&frame));
    AppListDelegate * delegate = static_cast<AppListDelegate *>(appListView->itemDelegate());
    if (delegate)
        delegate->setActived(true);

    //　dark type
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(2));
    delegate = static_cast<AppListDelegate *>(appListView->itemDelegate());
    appListView->setItemDelegate(new AppListDelegate(frame.m_appsModel));
    if (delegate)
        delegate->setActived(true);

    frame.show();
    QTest::qWait(500);
    frame.hide();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(1));
}

TEST_F(Tst_Applistview, appListView_test)
{
    WindowedFrame frame;

    AppListView *widget = frame.m_appsView;
    widget->setItemDelegate(new AppListDelegate(&frame));

    AppsListModel *pModel = new AppsListModel(AppsListModel::All);
    QModelIndex index;
    pModel->insertRow(0, index);
    widget->setModel(pModel);

    frame.show();
    QTest::qWait(1000);

    QWheelEvent event(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(widget->viewport(), &event);

    QMouseEvent event1(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event2);

    widget->hasAutoScroll();
    QMouseEvent event3(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(widget->viewport(), &event3);

    QMimeData mimeData;
    mimeData.setData("test", "test");

    QDragEnterEvent event4(QPoint(0, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event4);

    QDragMoveEvent event5(QPoint(0, 2), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget, &event5);

    QDragLeaveEvent event6;
    QApplication::sendEvent(widget, &event6);

    QDropEvent event7(QPointF(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(widget->viewport(), &event7);

    QEvent event8(QEvent::Enter);
    QApplication::sendEvent(widget, &event8);

    QEvent event9(QEvent::Leave);
    QApplication::sendEvent(widget, &event9);

    widget->handleScrollValueChanged();
    widget->handleScrollFinished();
    widget->prepareDropSwap();
    widget->dropSwap();
    widget->menuHide();
    frame.hide();
}
