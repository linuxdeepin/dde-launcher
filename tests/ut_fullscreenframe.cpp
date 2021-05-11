#include <gtest/gtest.h>

#define private public
#include "fullscreenframe.h"
#include "multipagesview.h"
#undef private

#include "../src/model/appslistmodel.h"

class Tst_Fullscreenframe : public testing::Test
{
public:
    void SetUp() override
    {
        m_fullScreenFrame = new FullScreenFrame(nullptr);
    }

    void TearDown() override
    {
        if (m_fullScreenFrame) {
            delete m_fullScreenFrame;
            m_fullScreenFrame = nullptr;
        }
    }

public:
    FullScreenFrame *m_fullScreenFrame;
};

TEST_F(Tst_Fullscreenframe, fullScreenFrame)
{
    m_fullScreenFrame->initUI();
    QWheelEvent event(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(m_fullScreenFrame, &event);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_fullScreenFrame, &event2);

    QMouseEvent event3(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_fullScreenFrame, &event3);

    QMouseEvent event4(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_fullScreenFrame, &event4);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollCurrent_test)
{
    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Chat, AppsListModel::AppCategory::Chat);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollPrev_test)
{
//    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Chat, AppsListModel::AppCategory::Internet);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollPrev1_test)
{
//    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Video, AppsListModel::AppCategory::Internet);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollNext_test)
{
//    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Internet, AppsListModel::AppCategory::Chat);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollNext1_test)
{
//    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::All, AppsListModel::AppCategory::Chat);
}

TEST_F(Tst_Fullscreenframe, scrollBlurBoxWidget_test)
{
    BlurBoxWidget *blueBox = new BlurBoxWidget(AppsListModel::AppCategory::Internet, nullptr);
    ScrollWidgetAgent* scrollWidgetAgent = new ScrollWidgetAgent;
    scrollWidgetAgent->setPosType(Pos_M);
    scrollWidgetAgent->setBlurBoxWidget(blueBox);

    m_fullScreenFrame->scrollBlurBoxWidget(scrollWidgetAgent);
}

TEST_F(Tst_Fullscreenframe, tips_test)
{
    for (int type = 0; type < 2; type++) {
         m_fullScreenFrame->updateDisplayMode(type);
         m_fullScreenFrame->showTips("");
         m_fullScreenFrame->hideTips();
    }
}

TEST_F(Tst_Fullscreenframe, backtab_nextTabWidget_test)
{
    m_fullScreenFrame->updateDisplayMode(GROUP_BY_CATEGORY);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Backtab);

    m_fullScreenFrame->updateDisplayMode(SEARCH);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Backtab);
}

TEST_F(Tst_Fullscreenframe, tab_nextTabWidget_test)
{
    m_fullScreenFrame->updateDisplayMode(GROUP_BY_CATEGORY);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Tab);

    m_fullScreenFrame->updateDisplayMode(SEARCH);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Tab);
}

TEST_F(Tst_Fullscreenframe, privateMethod_test)
{
    m_fullScreenFrame->primaryScreenChanged();
    m_fullScreenFrame->updateDockPosition();
    m_fullScreenFrame->updateGeometry();

    m_fullScreenFrame->updateDisplayMode(GROUP_BY_CATEGORY);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Backtab);

    m_fullScreenFrame->categoryListChanged();

    m_fullScreenFrame->moveCurrentSelectApp(Qt::Key_Tab);
    m_fullScreenFrame->moveCurrentSelectApp(Qt::Key_Undo);
    m_fullScreenFrame->moveCurrentSelectApp(Qt::Key_Space);
    m_fullScreenFrame->moveCurrentSelectApp(Qt::Key_Backtab);
    m_fullScreenFrame->moveCurrentSelectApp(Qt::Key_Right);
    m_fullScreenFrame->moveCurrentSelectApp(Qt::Key_Down);

    m_fullScreenFrame->appendToSearchEdit(static_cast<const char>(-1));
    m_fullScreenFrame->appendToSearchEdit(static_cast<const char>(0));

    m_fullScreenFrame->hideLauncher();
}

TEST_F(Tst_Fullscreenframe, parentWidget_test)
{
    AppGridView *gridView = nullptr;
    gridView = m_fullScreenFrame->m_multiPagesView->m_appGridViewList[m_fullScreenFrame->m_multiPagesView->m_pageIndex];

    ASSERT_TRUE(gridView);

    FullScreenFrame *fullscreenFrame = nullptr;
    fullscreenFrame = (qobject_cast<FullScreenFrame*>)(
                gridView->parentWidget()->parentWidget()->parentWidget()->parentWidget()
                ->parentWidget()->parentWidget()->parentWidget());

    ASSERT_TRUE(fullscreenFrame);

    gridView->flashDrag();
}
