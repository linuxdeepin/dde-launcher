#include "applistdelegate.h"

#define private public
#include "appgridview.h"
#undef private

#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>

#include <gtest/gtest.h>

class Tst_Appgridview : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new AppGridView;
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    AppGridView *m_widget;
};

TEST_F(Tst_Appgridview, appGridView_test)
{
    AppsListModel *pModel = new AppsListModel(AppsListModel::Internet);
    m_widget->setModel(pModel);

    m_widget->indexYOffset(m_widget->indexAt(0));
    QWidget *w = new QWidget(m_widget);
    m_widget->setContainerBox(w);

    m_widget->updateItemHiDPIFixHook(m_widget->indexAt(0));

    m_widget->setDelegate(m_widget->getDelegate());

    QDropEvent event(QPointF(0, 0), Qt::CopyAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget->viewport(), &event);

    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget->viewport(), &event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget->viewport(), &event2);

    QMimeData data;
    data.setData("RequestDock","test");
    QDragEnterEvent event3(QPoint(0, 1), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget->viewport(), &event3);

    QDragMoveEvent event4(QPoint(0, 2), Qt::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget->viewport(), &event4);

    m_widget->dragOut(-1);
    m_widget->dragIn(m_widget->indexAt(0));

    QDragLeaveEvent event5;
    QApplication::sendEvent(m_widget->viewport(), &event5);

    QMouseEvent event6(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget->viewport(), &event6);

    QMouseEvent event7(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget->viewport(), &event7);

    m_widget->fitToContent();
}
