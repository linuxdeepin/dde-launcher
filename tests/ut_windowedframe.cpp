#define private public
#include "windowedframe.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class Tst_Windowedframe : public testing::Test
{
public:
    void SetUp() override
    {
        m_windowedframe = new WindowedFrame(nullptr);
    }

    void TearDown() override
    {
        if (m_windowedframe) {
            delete m_windowedframe;
            m_windowedframe = nullptr;
        }
    }

public:
    WindowedFrame *m_windowedframe;
};

TEST_F(Tst_Windowedframe, privateMethod_test)
{
    m_windowedframe->moveCurrentSelectApp(Qt::Key_Undo);
    // 循环切换
    for (int count = 0; count < 6; count++) {
        m_windowedframe->moveCurrentSelectApp(Qt::Key_Tab);
    }
    // 循环回退
    for (int count = 0; count < 6; count++) {
        m_windowedframe->moveCurrentSelectApp(Qt::Key_Backtab);
    }
    m_windowedframe->moveCurrentSelectApp(Qt::Key_Up);
    m_windowedframe->moveCurrentSelectApp(Qt::Key_Down);
    m_windowedframe->moveCurrentSelectApp(Qt::Key_Left);
    m_windowedframe->moveCurrentSelectApp(Qt::Key_Right);

    const QModelIndex index;
    m_windowedframe->switchToCategory(index);

    m_windowedframe->getCornerPath(WindowedFrame::AnchoredCornor::TopLeft);
    m_windowedframe->getCornerPath(WindowedFrame::AnchoredCornor::TopRight);
    m_windowedframe->getCornerPath(WindowedFrame::AnchoredCornor::BottomLeft);
    m_windowedframe->getCornerPath(WindowedFrame::AnchoredCornor::BottomRight);

    m_windowedframe->resetWidgetStyle();
    m_windowedframe->hideLauncher();
}

TEST_F(Tst_Windowedframe, privateSlots_test)
{
    m_windowedframe->adjustPosition();
    m_windowedframe->onToggleFullScreen();

    const double value = 0.5;
    m_windowedframe->onOpacityChanged(value);
    // display model switch
    m_windowedframe->onSwitchBtnClicked();
    m_windowedframe->onSwitchBtnClicked();

    m_windowedframe->onWMCompositeChanged();

    m_windowedframe->searchText("");
    m_windowedframe->searchText("test");

    m_windowedframe->showTips("test");
    m_windowedframe->prepareHideLauncher();
    m_windowedframe->recoveryAll();
}

TEST_F(Tst_Windowedframe, eventFilter_test)
{
    QEvent event(QEvent::Type::None);
    m_windowedframe->m_eventFilter->eventFilter(nullptr, &event);

    QEvent event1(QEvent::Type::KeyPress);
    QKeyEvent *keyPress = static_cast<QKeyEvent *>(&event1);

    QKeyEvent f1KeyEvent(QEvent::Type::KeyPress, Qt::Key_F1, static_cast<QKeyEvent *>(keyPress)->modifiers());
    m_windowedframe->m_eventFilter->eventFilter(nullptr, &f1KeyEvent);

    QKeyEvent returnKeyEvent(QEvent::Type::KeyPress, Qt::Key_Return, static_cast<QKeyEvent *>(keyPress)->modifiers());
    m_windowedframe->m_eventFilter->eventFilter(nullptr, &returnKeyEvent);

    QKeyEvent escapeKeyEvent(QEvent::Type::KeyPress, Qt::Key_Escape, static_cast<QKeyEvent *>(keyPress)->modifiers());
    m_windowedframe->m_eventFilter->eventFilter(nullptr, &escapeKeyEvent);

    QKeyEvent spaceKeyEvent(QEvent::Type::KeyPress, Qt::Key_Space, static_cast<QKeyEvent *>(keyPress)->modifiers());
    m_windowedframe->m_eventFilter->eventFilter(nullptr, &spaceKeyEvent);

    QKeyEvent tabKeyEvent(QEvent::Type::KeyPress, Qt::Key_Tab, static_cast<QKeyEvent *>(keyPress)->modifiers());
    m_windowedframe->m_eventFilter->eventFilter(nullptr, &tabKeyEvent);

    QKeyEvent backSpaceKeyEvent(QEvent::Type::KeyPress, Qt::Key_Backspace, static_cast<QKeyEvent *>(keyPress)->modifiers());
    m_windowedframe->m_eventFilter->eventFilter(nullptr, &backSpaceKeyEvent);
}

TEST_F(Tst_Windowedframe, updatePostion_test)
{
    int defaultPos = CalculateUtil::instance()->m_dockInter->position();

    /*
     * 上: 0, 右: 1, 下: 2, 左: 3
    */
    for (int i = 0; i < 4; i++) {
        /*
         * 时尚模式: 0
         * 高效模式: 1
        */
        for (int j = 0; j < 2; j++) {
            m_windowedframe->m_dockInter->setPosition(i);
            m_windowedframe->m_dockInter->setDisplayMode(j);
            m_windowedframe->updatePosition();
            QTest::qWait(500);
        }
    }

    QTest::qWait(500);
    //　恢复默认位置．
    CalculateUtil::instance()->m_dockInter->setPosition(defaultPos);
}
