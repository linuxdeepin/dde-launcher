#include "appslistmodel.h"

#define private public
#include "fullscreenframe.h"
#include "multipagesview.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

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

TEST_F(Tst_Fullscreenframe, wheelEvent_test)
{
    m_fullScreenFrame->m_displayMode = GROUP_BY_CATEGORY;

    QWheelEvent wheelEve(QPointF(0, 0), 10, Qt::MiddleButton, Qt::ControlModifier, Qt::Vertical);
    QApplication::sendEvent(m_fullScreenFrame, &wheelEve);

    m_fullScreenFrame->m_displayMode = SEARCH;
}

TEST_F(Tst_Fullscreenframe, keyEvent_test)
{
    QKeyEvent minusKeyEvent(QEvent::KeyPress, Qt::Key_Minus, Qt::NoModifier, QString(""));
    QApplication::sendEvent(m_fullScreenFrame, &minusKeyEvent);

    QKeyEvent ctrlMinusKeyEvent(QEvent::KeyPress, Qt::Key_Minus, Qt::ControlModifier, QString(""));
    QApplication::sendEvent(m_fullScreenFrame, &ctrlMinusKeyEvent);

    QKeyEvent equalKeyEvent(QEvent::KeyPress, Qt::Key_Equal, Qt::NoModifier, QString(""));
    QApplication::sendEvent(m_fullScreenFrame, &equalKeyEvent);

    QKeyEvent controlVKeyEvent(QEvent::KeyPress, Qt::Key_Equal, Qt::ControlModifier, QString(""));
    QApplication::sendEvent(m_fullScreenFrame, &controlVKeyEvent);
}

TEST_F(Tst_Fullscreenframe, scrollBlurBoxWidget_test)
{
    BlurBoxWidget blueBox(AppsListModel::AppCategory::Internet, nullptr);

    ScrollWidgetAgent scrollWidgetAgent(m_fullScreenFrame);
    scrollWidgetAgent.setPosType(Pos_M);
    scrollWidgetAgent.setBlurBoxWidget(&blueBox);

    m_fullScreenFrame->scrollBlurBoxWidget(nullptr);
    m_fullScreenFrame->scrollBlurBoxWidget(&scrollWidgetAgent);
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
    m_fullScreenFrame->dockPosition();
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

TEST_F(Tst_Fullscreenframe, categoryChange_test)
{
    m_fullScreenFrame->m_mouse_press = true;

    m_fullScreenFrame->m_displayMode = SEARCH;
    m_fullScreenFrame->showTips(QString(""));

    m_fullScreenFrame->m_displayMode = GROUP_BY_CATEGORY;
    m_fullScreenFrame->showTips(QString(""));
}

TEST_F(Tst_Fullscreenframe, launcherApp_test)
{
    m_fullScreenFrame->m_searchWidget->m_searchEdit->clear();
    m_fullScreenFrame->m_searchWidget->categoryBtn()->setFocus();
    m_fullScreenFrame->launchCurrentApp();

    m_fullScreenFrame->m_searchWidget->categoryBtn()->clearFocus();
    m_fullScreenFrame->m_displayMode = SEARCH;
    m_fullScreenFrame->launchCurrentApp();

    m_fullScreenFrame->m_displayMode = GROUP_BY_CATEGORY;
    m_fullScreenFrame->launchCurrentApp();

    m_fullScreenFrame->windowDeactiveEvent();

    QPoint point(100, 100);
    m_fullScreenFrame->regionMonitorPoint(point);

    m_fullScreenFrame->moveCurrentSelectApp(Qt::TabFocus);

    m_fullScreenFrame->appendToSearchEdit(-1);
    m_fullScreenFrame->appendToSearchEdit('1');
}
