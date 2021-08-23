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
{
public:
    void SetUp() override
    {
        m_windowFrame = new WindowedFrame;
    }

    void TearDown() override
    {
        delete m_windowFrame;
        m_windowFrame = nullptr;
    }

public:
    WindowedFrame *m_windowFrame;
};

TEST_F(Tst_Applistview, appDelegate_test)
{
    AppListView *appListView = m_windowFrame->m_appsView;

    //　light type
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(1));
    appListView->setModel(m_windowFrame->m_appsModel);
    appListView->setItemDelegate(new AppListDelegate(m_windowFrame));

    //　dark type
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType(1));
    appListView->setItemDelegate(new AppListDelegate(m_windowFrame->m_appsModel));

    m_windowFrame->show();
    QTest::qWait(500);
    m_windowFrame->hide();
}

TEST_F(Tst_Applistview, appListView_test)
{
    AppListView *widget = m_windowFrame->m_appsView;

    AppsListModel *pModel = new AppsListModel(AppsListModel::All);
    QModelIndex index;
    pModel->insertRow(0, index);
    widget->setModel(pModel);

    m_windowFrame->show();
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
    m_windowFrame->hide();
}
